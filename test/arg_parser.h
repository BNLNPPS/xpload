#pragma once

#include <algorithm>
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

  if (itr != args.end() && std::next(itr) != args.end() && (*++itr)[0] != '-') {
    return *itr;
  }
  return "";
}


bool ArgParser::verify()
{
  std::string tag = get_value("--gtName");

  if (tag.empty()) {
    std::cerr << "Error: --gtName option must be set\n";
    return false;
  }

  std::string timestamp = get_value("--minorIOV");

  if (timestamp.empty()) {
    std::cerr << "Error: --minorIOV option must be set\n";
    return false;
  }

  return true;
}


void ArgParser::usage()
{
  std::cout << "Usage: " << args[0] << " --gtName <tag> --minorIOV <timestamp>\n";
}
