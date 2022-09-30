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

  std::string mode;

 private:

  std::vector<std::string> args;
};


ArgParser::ArgParser(int &argc, char *argv[])
{
  std::string prg_name(argv[0]);
  prg_name.erase(0, prg_name.find_last_of('/')+1);

  args.push_back(prg_name);

  mode = argc > 1 ? std::string(argv[1]) : "";

  for (int i=2; i < argc; ++i)
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
  if (value.empty() && option == "-t") return "";
  if (value.empty() && option == "-d") return "";
  if (value.empty() && option == "-p") return "";
  if (value.empty() && option == "-b") return std::to_string(0);
  if (value.empty() && option == "-e") return std::to_string(INT64_MAX);
  if (value.empty() && option == "-s") return std::to_string(INT64_MAX);

  return value;
}


bool ArgParser::verify()
{
  if (mode != "push" && mode != "fetch") {
    std::cerr << "Error: the first argument (mode) must be either \"push\" or \"fetch\"\n";
    return false;
  }

  if (get_value("-t").empty()) {
    std::cerr << "Error: -t option must be set\n";
    return false;
  }

  if (get_value("-d").empty() && mode == "push") {
    std::cerr << "Error: -d option must be set\n";
    return false;
  }

  if (get_value("-p").empty() && mode == "push") {
    std::cerr << "Error: -d option must be set\n";
    return false;
  }

  if (stoul(get_value("-b")) > stoul(get_value("-e")) ) {
    std::cerr << "Error: begin timestamp > end timestamp\n";
    return false;
  }

  return true;
}


void ArgParser::usage()
{
  std::cout << "Usage: " << args[0] << " push -t <tag> -d <domain> -p <payload> [-b <timestamp>] [-e <timestamp>]\n"
            << "       " << args[0] << " fetch -t <tag> [-d <domain>] [-s <timestamp>]\n";
}
