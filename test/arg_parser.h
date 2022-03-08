#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>


class ArgParser
{
 public:

  ArgParser(int &argc, char *argv[]);
  std::string get_value(const std::string &option) const;
  bool verify();
  void usage();

 private:

  std::vector<std::string> args;
};


ArgParser::ArgParser(int &argc, char *argv[])
{
  std::string prg_name(argv[0]);
  prg_name.erase(0, prg_name.find_last_of('/')+1);

  args.push_back(prg_name);

  for (int i=1; i < argc; ++i)
    args.push_back( std::string(argv[i]) );
}


std::string ArgParser::get_value(const std::string &option) const
{
  auto itr = std::find(args.begin(), args.end(), option);

  std::string value{""};

  if (itr != args.end() && std::next(itr) != args.end() && (*++itr)[0] != '-') {
    value = *itr;
  }

  // Default values
  if (value.empty() && option == "-c") return "test";
  if (value.empty() && option == "-t") return "";
  if (value.empty() && option == "-d") return "";
  if (value.empty() && option == "-s") return std::to_string(UINT64_MAX);

  return value;
}


bool ArgParser::verify()
{
  if (get_value("-t").empty()) {
    std::cerr << "Error: -t option must be set\n";
    return false;
  }

  return true;
}


void ArgParser::usage()
{
  std::cout << "Usage: " << args[0] << " -c <config_name> -t <tag> -d <domain> -s <timestamp>\n";
}
