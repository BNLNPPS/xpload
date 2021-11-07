#pragma once

#include <string>

namespace xpload {

struct DbConfig
{
  std::string host{"localhost"};
  std::string apiroot{"/api/cdb_rest"};
  std::string port{"8000"};
  std::string path{"/path/to/payload/data"};

  std::string url() const { return "http://" + host + ':' + port + apiroot; }
};


/**
 * Provides access to all configuration types and data.
 */
class Configurator
{
 public:

  Configurator(std::string config_name = "test");

  /// A unique name associated with this Configurator
  std::string name;

  DbConfig db;

 private:

  std::string Locate(std::string filename) const;
  std::string ReadConfig(std::string filepath);
};

}
