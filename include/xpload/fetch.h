#pragma once

#include <cstdint>
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
  std::vector<std::string> paths;
  long response_code;
  double byte_count;
  size_t cache_size;
};

Result fetch(std::string tag, std::string domain = "", uint64_t timestamp = UINT64_MAX, const Configurator& cfg = Configurator());

}
