#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "xpload/configurator.h"

namespace xpload {

struct Payload
{
  std::filesystem::path file;
  std::string domain;
  std::string tag;
  std::string tag_type = "online";
  std::string tag_status = "unlocked";
  uint64_t ts_min = 0;
  uint64_t ts_max = INT64_MAX;
};

void push(Payload payload, const Configurator& cfg = Configurator());

}
