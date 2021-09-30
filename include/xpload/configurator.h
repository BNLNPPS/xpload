#pragma once

#include <string>

namespace xpload {

/**
 * Provides access to all configuration types and data.
 */
class Configurator
{
 public:

  Configurator(std::string name = "xpload");

  /// A unique name associated with this Configurator
  std::string name;

};

}
