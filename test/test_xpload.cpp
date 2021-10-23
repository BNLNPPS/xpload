#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "xpload/xpload.h"
#include "arg_parser.h"


int main(int argc, char *argv[])
{
  using namespace std;

  ArgParser arg_parser(argc, argv);

  if (!arg_parser.verify())
  {
    arg_parser.usage();
    return EXIT_FAILURE;
  }

  string tag = arg_parser.get_value("--gtName");
  uint64_t timestamp = stol(arg_parser.get_value("--minorIOV"));

  vector<string> paths = xpload::fetch(tag, timestamp);

  for (const string& path : paths) 
    cout << path << '\n';

  return EXIT_SUCCESS;
} 
