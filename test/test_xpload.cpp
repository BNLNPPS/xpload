#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "xpload/xpload.h"
#include "arg_parser.h"

using namespace std;


int test_push(const ArgParser& arg_parser)
{
  string tag{arg_parser.get_value("-t")};
  string domain{arg_parser.get_value("-d")};
  string file{arg_parser.get_value("-p")};
  uint64_t ts_min{stoul(arg_parser.get_value("-b"))};
  uint64_t ts_max{stoul(arg_parser.get_value("-e"))};

  xpload::push({file, domain, tag, "online", "unlocked", ts_min, ts_max});

  return EXIT_SUCCESS;
}


int test_fetch(const ArgParser& arg_parser)
{
  string tag(arg_parser.get_value("-t"));
  string domain(arg_parser.get_value("-d"));
  uint64_t timestamp = stoul(arg_parser.get_value("-s"));

  xpload::Configurator config;

  xpload::Result result = xpload::fetch(tag, domain, timestamp, config);

  if (result.paths.empty())
  {
    cout << "No paths found\n";
    return EXIT_FAILURE;
  }

  for (const string path : result.paths)
    cout << path << '\n';

  return EXIT_SUCCESS;
}


/**
 * Usage:
 *
 * $ test_xpload <push|fetch> -t <tag> -s <timestamp>
 */
int main(int argc, char *argv[])
{
  ArgParser arg_parser(argc, argv);

  if (!arg_parser.verify())
  {
    arg_parser.usage();
    return EXIT_FAILURE;
  }

  return arg_parser.mode == "push" ? test_push(arg_parser) : test_fetch(arg_parser);
}
