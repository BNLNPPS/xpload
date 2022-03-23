#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "xpload/configurator.h"

namespace xpload {

struct RequestParams
{
  std::string tag;
  std::string domain;
  uint64_t timestamp;
  const Configurator& cfg;
};

struct Result
{
  const RequestParams reqpars;
  std::string payload;
  std::vector<std::filesystem::path> paths;
  long response_code;
  double byte_count;
  size_t cache_size;
  int total_retries;
};

Result fetch(std::string tag, std::string domain = "", uint64_t timestamp = UINT64_MAX, const Configurator& cfg = Configurator());

}
