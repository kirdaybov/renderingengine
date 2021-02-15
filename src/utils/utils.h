#pragma once
#include <string>

struct Utils
{
  static std::string StripPathFromName(const std::string& path)
  {
    std::size_t lastDot = path.find_last_of(".");
    std::size_t lastSep = path.find_last_of("\\");

    return path.substr(lastSep + 1, lastDot - lastSep - 1);
  }

  static std::string GetFileExtension(const std::string& path)
  {
    std::size_t lastDot = path.find_last_of(".");

    return path.substr(lastDot + 1, path.length() - lastDot - 1);
  }
};
