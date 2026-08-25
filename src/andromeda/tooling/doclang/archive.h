//-*-C++-*-

#ifndef ANDROMEDA_TOOLING_DOCLANG_ARCHIVE_H_
#define ANDROMEDA_TOOLING_DOCLANG_ARCHIVE_H_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <miniz/miniz.h>

namespace andromeda::doclang
{

  class archive
  {
  public:

    typedef std::vector<std::byte> bytes_type;

  public:

    archive() = default;

    void clear();

    bool load_from_memory(std::span<const std::byte> data);

    bool has(std::string_view path) const;

    std::optional<std::string_view> text(std::string_view path) const;
    std::optional<std::span<const std::byte> > bytes(std::string_view path) const;

    std::vector<std::string> paths() const;

    const std::string& get_last_error() const { return last_error; }

  private:

    void set_error(std::string msg);

  private:

    std::map<std::string, bytes_type> entries;
    std::string last_error;
  };

  void archive::clear()
  {
    entries.clear();
    last_error.clear();
  }

  void archive::set_error(std::string msg)
  {
    last_error = std::move(msg);
  }

  bool archive::load_from_memory(std::span<const std::byte> data)
  {
    clear();

    mz_zip_archive zip;
    std::memset(&zip, 0, sizeof(zip));

    const void* ptr = static_cast<const void*>(data.data());
    if(!mz_zip_reader_init_mem(&zip, ptr, data.size(), 0))
      {
        set_error("could not initialise zip reader from memory");
        return false;
      }

    bool success = true;
    const mz_uint num_files = mz_zip_reader_get_num_files(&zip);

    for(mz_uint i=0; i<num_files; i++)
      {
        mz_zip_archive_file_stat stat;
        std::memset(&stat, 0, sizeof(stat));

        if(!mz_zip_reader_file_stat(&zip, i, &stat))
          {
            set_error("could not read zip file metadata");
            success = false;
            break;
          }

        if(mz_zip_reader_is_file_a_directory(&zip, i))
          {
            continue;
          }

        size_t uncomp_size = 0;
        void* raw = mz_zip_reader_extract_to_heap(&zip, i, &uncomp_size, 0);

        if(raw==nullptr)
          {
            set_error("could not extract zip entry: " + std::string(stat.m_filename));
            success = false;
            break;
          }

        bytes_type bytes(uncomp_size);
        std::memcpy(bytes.data(), raw, uncomp_size);
        mz_free(raw);

        entries[stat.m_filename] = std::move(bytes);
      }

    mz_zip_reader_end(&zip);

    if(success and not has("document.xml"))
      {
        set_error("dclx archive does not contain document.xml");
        return false;
      }

    return success;
  }

  bool archive::has(std::string_view path) const
  {
    return entries.count(std::string(path))==1;
  }

  std::optional<std::string_view> archive::text(std::string_view path) const
  {
    auto itr = entries.find(std::string(path));
    if(itr==entries.end())
      {
        return std::nullopt;
      }

    const auto& data = itr->second;
    const char* ptr = reinterpret_cast<const char*>(data.data());
    return std::string_view(ptr, data.size());
  }

  std::optional<std::span<const std::byte> > archive::bytes(std::string_view path) const
  {
    auto itr = entries.find(std::string(path));
    if(itr==entries.end())
      {
        return std::nullopt;
      }

    return std::span<const std::byte>(itr->second.data(), itr->second.size());
  }

  std::vector<std::string> archive::paths() const
  {
    std::vector<std::string> result;
    result.reserve(entries.size());

    for(const auto& item:entries)
      {
        result.push_back(item.first);
      }

    return result;
  }

}

#endif
