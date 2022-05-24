#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <numeric>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "xpload/xpload.h"


/**
 * Splits a [0, b] interval into segments of integer length by n distinct random
 * points.
 */
std::vector<int> split_interval(int b, int n)
{
  bool prereq = (b > 0 && n > 0 && n <= b + 1);
  if (!prereq)
  {
    std::cerr << "Error: Assertion failed: (b > 0 && n > 0 && n <= b + 1) [" << __PRETTY_FUNCTION__ << "]\n";
    return {};
  }

  // Generate a vector of indices [0, ..., b]
  std::vector<int> indices(b+1);
  std::iota(indices.begin(), indices.end(), 0);

  // Pick n distinct random indices
  std::vector<int> isegments;
  for (int i=0; i<n; ++i)
  {
    int rand_index = std::rand() % indices.size(); // % is biased but ok for the purpose
    isegments.push_back(indices[rand_index]);
    indices.erase(indices.begin() + rand_index);
  }

  std::sort(isegments.begin(), isegments.end());

  std::vector<int> segments;
  std::adjacent_difference(isegments.begin(), isegments.end(), std::back_inserter(segments));

  // Ensure that the entire [0, b] interval is covered by segments by adding
  // the last point b unless it is already selected
  if (b != isegments.back())
    segments.push_back(b - isegments.back());

  return segments;
}


std::pair<int, int> parse_limits(std::string limits_str)
{
  const std::regex limits_regex("([0-9]+)-([0-9]+)");
  std::smatch limits_matches;
  // Create default limits [0,0]
  std::pair<int, int> limits;

  if (std::regex_match(limits_str, limits_matches, limits_regex)) {
    limits.first  = std::stoi(limits_matches[1]);
    limits.second = std::stoi(limits_matches[2]);
  } else {
    std::cerr << "Error: Expected input formatted as N-M, e.g. 3-8 [" << __PRETTY_FUNCTION__ << "]\n";
  }

  return limits;
}


struct Tokens {
  uint64_t timestamp;
  std::string tag, domain, payload;
};

auto random_tokens(std::pair<int, int> tag_limits, std::pair<int, int> dom_limits, std::pair<int, int> tst_limits)
{
  if (tag_limits.first > tag_limits.second ||
      dom_limits.first > dom_limits.second ||
      tst_limits.first > tst_limits.second)
  {
    std::cerr << "Error: Assertion failed: a <= b in [a, b] [" << __PRETTY_FUNCTION__ << "]\n";
    return Tokens{};
  }

  uint64_t timestamp  = tst_limits.first + std::rand() % (tst_limits.second - tst_limits.first + 1);
  int tag_index       = tag_limits.first + std::rand() % (tag_limits.second - tag_limits.first + 1);
  int dom_index       = dom_limits.first + std::rand() % (dom_limits.second - dom_limits.first + 1);

  std::ostringstream tag; tag << "Tag_" << tag_index;
  std::ostringstream domain; domain << "Domain_" << dom_index;
  std::ostringstream payload; payload << "Payload_" << timestamp << "_Commit_" << tag_index << "_Domain_" << dom_index;

  return Tokens{timestamp, tag.str(), domain.str(), payload.str()};
}


/**
 * Usage:
 *
 * $ test_xpload_rand <b> <n> [rand_seed]
 *
 * <b> is a positive integer defining a closed interval [0, b]
 * <n> is a number of calls to be made within the interval
 * [rand_seed] is a seed for the random number generator
 */
int main(int argc, char *argv[])
{
  using namespace std;

  // Process command line options
  vector<string> args;
  for (int i=1; i < argc; ++i)
    args.push_back( string(argv[i]) );

  int b = (args.size() > 0) ? stoi(args[0]) : 100;
  int n = (args.size() > 1) ? stoi(args[1]) : ceil(b/10.);
  int rand_seed = (args.size() > 2) ? stoi(args[2]) : 12345;
  pair<int, int> tag_limits = (args.size() > 3) ? parse_limits(args[3]) : pair<int, int>{1, 100};
  pair<int, int> dom_limits = (args.size() > 4) ? parse_limits(args[4]) : pair<int, int>{1, 10};
  pair<int, int> tst_limits = (args.size() > 5) ? parse_limits(args[5]) : pair<int, int>{1, 1000};

  std::srand(rand_seed);

  vector<int> segments = split_interval(b, n);

  int sum = accumulate(segments.begin(), segments.end(), 0);
  if (sum != b) {
    cerr << "Error: Assertion failed: (sum == b) [" << __PRETTY_FUNCTION__ << "]\n";
    return EXIT_FAILURE;
  }

  string cfg = getenv("XPLOAD_CONFIG") ? string(getenv("XPLOAD_CONFIG")) : "test";
  xpload::Configurator config(cfg);

  // Print the header
  if (config.db.verbosity > 0)
    cout << "time, duration, wait, tag, domain, timestamp, byte_count, response_code, cache_size, total_retries, path, error_code\n";

  Tokens tk;

  for (int segment : segments)
  {
    this_thread::sleep_for(chrono::seconds(segment));

    tk = random_tokens(tag_limits, dom_limits, tst_limits);

    auto t0 = chrono::system_clock::now();
    auto t1 = chrono::high_resolution_clock::now();
    xpload::Result result = xpload::fetch(tk.tag, tk.domain, tk.timestamp, config);
    auto t2 = chrono::high_resolution_clock::now();

    chrono::duration<double, std::milli> td = t2 - t1;

    int error_code = 0;

    if (result.paths.size() < 1) {
      cerr << "Error: Expected at least one payload but got " << result.paths.size() << " [" << __PRETTY_FUNCTION__ << "]\n";
      error_code = 1;
    } else if ( result.paths[0].filename() != tk.payload) {
      cerr << "Error: Expected " << tk.payload << " but got " << result.paths[0] << " [" << __PRETTY_FUNCTION__ << "]\n";
      error_code = 2;
    }

    if (config.db.verbosity > 0) {
      cout << chrono::system_clock::to_time_t(t0) << ", " << td.count() << ", " << segment << ", "
           << result.reqpars.tag << ", "
           << result.reqpars.domain << ", "
           << result.reqpars.timestamp << ", "
           << result.byte_count << ", "
           << result.response_code << ", "
           << result.cache_size << ", "
           << result.total_retries << ", \""
           << (!error_code ? result.payload : "") << "\", "
           << error_code << "\n";
    }
  }

  return EXIT_SUCCESS;
}
