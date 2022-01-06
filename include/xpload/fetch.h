#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "xpload/configurator.h"

namespace xpload {

std::vector<std::string> fetch(std::string tag, std::string domain = "", uint64_t timestamp = UINT64_MAX, const Configurator& cfg = Configurator(), bool use_cache = false);

}
