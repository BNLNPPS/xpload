#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "xpload/configurator.h"

namespace xpload {

struct Result
{
  std::vector<std::string> paths;
  long response_code;
  double byte_count;
};

Result fetch(std::string tag, std::string domain = "", uint64_t timestamp = UINT64_MAX, const Configurator& cfg = Configurator());

}
