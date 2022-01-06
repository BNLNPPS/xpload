#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "xpload/xpload.h"
#include "arg_parser.h"


/**
 * Usage:
 *
 * $ test_xpload -c <config_name> -t <tag> -s <timestamp>
 */
int main(int argc, char *argv[])
{
  using namespace std;

  ArgParser arg_parser(argc, argv);

  if (!arg_parser.verify())
  {
    arg_parser.usage();
    return EXIT_FAILURE;
  }

  string cfg(arg_parser.get_value("-c"));
  string tag(arg_parser.get_value("-t"));
  string domain("");
  uint64_t timestamp = stoul(arg_parser.get_value("-s"));

  xpload::Configurator config(cfg);

  vector<string> paths = xpload::fetch(tag, domain, timestamp, config);

  if (paths.empty())
  {
    cout << "No paths found\n";
    return EXIT_FAILURE;
  }

  for (const string& path : paths) 
    cout << path << '\n';

  return EXIT_SUCCESS;
}
