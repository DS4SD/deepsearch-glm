//-*-C++-*-

#ifndef ANDROMEDA_TOOLING_DOCLANG_READER_H_
#define ANDROMEDA_TOOLING_DOCLANG_READER_H_

#include <cstddef>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <andromeda/tooling/doclang/document.h>

namespace andromeda::doclang
{

  enum class format
  {
    dclg,
    dclx,
    unknown
  };

  class reader
  {
  public:

    static format detect_format(const std::filesystem::path& path);

    static bool read(const std::filesystem::path& path, document& out);

    static bool read_dclg_buffer(std::string_view xml, document& out);
    static bool read_dclx_buffer(std::span<const std::byte> bytes, document& out);

  private:

    static bool read_file(const std::filesystem::path& path,
                          std::vector<std::byte>& data,
                          std::string& error);

    static bool parse_xml(std::string_view xml, document& out);
  };

  format reader::detect_format(const std::filesystem::path& path)
  {
    std::string ext = path.extension().string();

    for(char& c:ext)
      {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
      }

    if(ext==".dclg")
      {
        return format::dclg;
      }

    if(ext==".dclx")
      {
        return format::dclx;
      }

    return format::unknown;
  }

  bool reader::read(const std::filesystem::path& path, document& out)
  {
    out.clear();
    out.set_source_path(path);

    std::vector<std::byte> data;
    std::string error;
    if(not read_file(path, data, error))
      {
        out.set_last_error(error);
        return false;
      }

    switch(detect_format(path))
      {
      case format::dclg:
        {
          const char* ptr = reinterpret_cast<const char*>(data.data());
          return read_dclg_buffer(std::string_view(ptr, data.size()), out);
        }

      case format::dclx:
        {
          return read_dclx_buffer(std::span<const std::byte>(data.data(), data.size()), out);
        }

      case format::unknown:
      default:
        {
          out.set_last_error("unsupported DocLang file extension: " + path.extension().string());
          return false;
        }
      }
  }

  bool reader::read_dclg_buffer(std::string_view xml, document& out)
  {
    out.clear();
    return parse_xml(xml, out);
  }

  bool reader::read_dclx_buffer(std::span<const std::byte> bytes, document& out)
  {
    out.clear();

    archive zip;
    if(not zip.load_from_memory(bytes))
      {
        out.set_last_error(zip.get_last_error());
        return false;
      }

    auto xml = zip.text("document.xml");
    if(not xml.has_value())
      {
        out.set_last_error("dclx archive does not contain document.xml");
        return false;
      }

    if(not parse_xml(xml.value(), out))
      {
        return false;
      }

    out.set_archive(std::move(zip));
    return true;
  }

  bool reader::read_file(const std::filesystem::path& path,
                         std::vector<std::byte>& data,
                         std::string& error)
  {
    std::ifstream ifs(path, std::ios::binary);
    if(not ifs)
      {
        error = "could not open file: " + path.string();
        return false;
      }

    ifs.seekg(0, std::ios::end);
    std::streamoff size = ifs.tellg();
    if(size<0)
      {
        error = "could not determine file size: " + path.string();
        return false;
      }

    ifs.seekg(0, std::ios::beg);
    data.resize(static_cast<std::size_t>(size));

    if(size>0)
      {
        ifs.read(reinterpret_cast<char*>(data.data()), size);
        if(not ifs)
          {
            error = "could not read file: " + path.string();
            return false;
          }
      }

    return true;
  }

  bool reader::parse_xml(std::string_view xml, document& out)
  {
    pugi::xml_parse_result result = out.xml().load_buffer(
      xml.data(), xml.size(), pugi::parse_default | pugi::parse_ws_pcdata);
    if(not result)
      {
        std::stringstream ss;
        ss << "could not parse DocLang XML: " << result.description()
           << " at offset " << result.offset;
        out.set_last_error(ss.str());
        return false;
      }

    if(not out.root())
      {
        out.set_last_error("DocLang XML does not contain root <doclang>");
        return false;
      }

    return true;
  }

}

#endif
