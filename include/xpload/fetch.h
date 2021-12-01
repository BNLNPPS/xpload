#pragma once

#include <cstdint>
#include <string>
#include <vector>


namespace xpload {

std::vector<std::string> fetch(std::string tag, uint64_t timestamp);
std::vector<std::string> fetch(std::string tag, uint64_t timestamp, const Configurator& cfg);

}
