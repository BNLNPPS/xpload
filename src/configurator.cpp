#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include <nlohmann/json.hpp>

#include "xpload/configurator.h"
#include "config.h"

namespace xpload {


Configurator::Configurator(std::string config_name) :
  name{std::getenv("XPLOAD_CONFIG") ? std::getenv("XPLOAD_CONFIG") : config_name}
{
  std::string filepath = Locate(name + ".json");

  if (filepath.empty()) {
    std::string paths{std::getenv("XPLOAD_CONFIG_DIR") ? std::getenv("XPLOAD_CONFIG_DIR") : XPLOAD_CONFIG_SEARCH_PATHS};
    std::cerr << "Warning: Could not find config file \"" << name << ".json\" in " << paths << "\n";
  }

  std::string error = ReadConfig(filepath);

  if (!error.empty())
    std::cerr << "Warning: Could not read config file \"" << name << ".json\". Using default\n";
}


std::string Configurator::Locate(std::string filename) const
{
  std::vector<std::string> search_paths;

  const std::string user_dir{std::getenv("XPLOAD_CONFIG_DIR") ? std::getenv("XPLOAD_CONFIG_DIR") : ""};

  if (user_dir.empty())
  {
    std::string paths(XPLOAD_CONFIG_SEARCH_PATHS);

    size_t last = 0;
    size_t next = 0;
    while ((next = paths.find(':', last)) != std::string::npos)
    {
      search_paths.push_back(paths.substr(last, next-last));
      last = next + 1;
    }

    search_paths.push_back(paths.substr(last));
  }
  else
  {
    search_paths.push_back(user_dir);
  }

  struct stat buffer;
  for (std::string path : search_paths)
  {
    std::string fullname(path + "/" + filename);
    if (stat(fullname.c_str(), &buffer) == 0)
      return fullname;
  }

  return "";
}


/**
 * Expects a valid filepath.
 */
std::string Configurator::ReadConfig(std::string filepath)
{
  nlohmann::json json;

  try {
    std::ifstream ifs(filepath);
    ifs >> json;

    db = {
      json["host"], json["port"],
      json["apiroot"], json["apiver"],
      json["path"],
      json["use_cache"],
      json["verbosity"],
      json["retry_times"],
      json["retry_max_delay"]
    };
  }
  catch (nlohmann::json::exception& e) {
    std::cerr << "Error: Failed reading config parameters from " << filepath << " " << e.what() << " [" << __PRETTY_FUNCTION__ << "]\n";
  }

  if (db.retry_times < 0)   db.retry_times = 0;
  if (db.retry_times > 100) db.retry_times = 100;

  if (db.retry_max_delay < 1)   db.retry_max_delay = 1;
  if (db.retry_max_delay > 100) db.retry_max_delay = 100;

  return {};
}

}
