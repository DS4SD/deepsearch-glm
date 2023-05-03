//-*-C++-*-

#ifndef ANDROMEDA_BASE_CRF_UTILS_H
#define ANDROMEDA_BASE_CRF_UTILS_H

namespace andromeda_crf
{
  static void tabulate(std::vector<andromeda_crf::utils::crf_token>& tokens)
  {
    std::stringstream ss;

    ss << std::setw(8) << "i" << " | "
       << std::setw(8) << "begin" << " | "
       << std::setw(8) << "end" << " | "
       << std::setw(16) << "true-label" << " | "
       << std::setw(16) << "pred-label" << " | "
       << std::setw(8) << "conf" << " | "
       << std::setw(32) << "text" << " | "
       << "\n";

    ss << std::setw(8) << std::string(8, '-') << " | "
       << std::setw(8) << std::string(8, '-') << " | "
       << std::setw(8) << std::string(8, '-') << " | "
       << std::setw(16) << std::string(16, '-') << " | "
       << std::setw(16) << std::string(16, '-') << " | "
       << std::setw(8) << std::string(8, '-') << " | "
       << std::setw(32) << std::string(32, '-') << " | "
       << "\n";

    std::size_t i=0;
    for(auto& elem:tokens)
      {
        ss << std::setw(8) << i++ << " | "
           << std::setw(8) << elem.beg << " | "
           << std::setw(8) << elem.end << " | "
           << std::setw(16) << elem.true_label << " | "
           << std::setw(16) << elem.pred_label << " | "
           << std::setw(8) << elem.pred_conf << " | "
           << std::setw(32) << elem.text << " | "
           << "\n";
      }

    std::string text="";
    for(auto elem:tokens)
      {
        text += elem.text;
        text += " ";
      }

    LOG_S(INFO) << "text: " << text << "\n\n" << ss.str() << "\n";
  }

}

#endif
