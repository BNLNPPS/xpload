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

  if (filepath.empty())
    std::cerr << "Warning: Could not find config file \"" << name << ".json\" in paths defined by $XPLOAD_DIR"
              << " and built-in list \"" << std::string(XPLOAD_CONFIG_SEARCH_PATHS) << "\". Using default\n";

  std::string error = ReadConfig(filepath);

  if (!error.empty())
    std::cerr << "Warning: Could not read config file \"" << name << ".json\". Using default\n";
}


std::string Configurator::Locate(std::string filename) const
{
  std::string paths(XPLOAD_CONFIG_SEARCH_PATHS);
  std::vector<std::string> search_paths;

  size_t last = 0;
  size_t next = 0;
  while ((next = paths.find(':', last)) != std::string::npos)
  {
    search_paths.push_back(paths.substr(last, next-last));
    last = next + 1;
  }

  search_paths.push_back(paths.substr(last));

  // Prepend all relative paths with user's XPLOAD_DIR
  const std::string prefix{std::getenv("XPLOAD_DIR") ? std::getenv("XPLOAD_DIR") : ""};
  std::vector<std::string> search_paths_x;

  for (std::string path : search_paths)
  {
    if (!prefix.empty() && path.size() && path[0] != '/')
      search_paths_x.push_back(prefix + '/' + path);
  }

  search_paths_x.insert(search_paths_x.end(), search_paths.begin(), search_paths.end());

  struct stat buffer;
  for (std::string path : search_paths_x)
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
      json["apiroot"],
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
