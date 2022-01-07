#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <numeric>
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
    std::cerr << "Assertion failed: (b > 0 && n > 0 && n <= b + 1)\n";
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
  segments.push_back(b - isegments.back());

  return segments;
}


auto random_tokens(std::pair<int, int> tag_range, std::pair<int, int> dom_range, std::pair<int, int> tst_range)
{
  struct Tokens { uint64_t tst; std::string tag, dom, pld; };

  uint64_t tst  = tst_range.first + std::rand() % tst_range.second;
  int tag_index = tag_range.first + std::rand() % tag_range.second;
  int dom_index = dom_range.first + std::rand() % dom_range.second;

  std::ostringstream tag; tag << "Tag_" << tag_index;
  std::ostringstream dom; dom << "Domain_" << dom_index;
  std::ostringstream pld; pld << "Payload_" << tst << "_Commit_" << tag_range.second - tag_index + 1 << "_Domain_" << dom_index;

  return Tokens{tst, tag.str(), dom.str(), pld.str()};
}


/**
 * Usage:
 *
 * $ test_xpload_rand <b> <n> [seed]
 *
 * <b> is a positive integer defining a closed interval [0, b]
 * <n> is a number of calls to be made within the interval
 * [seed] is a seed for the random number generator
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
  int seed = (args.size() > 2) ? stoi(args[2]) : 12345;

  vector<int> segments = split_interval(b, n);

  int sum = accumulate(segments.begin(), segments.end(), 0);
  if (sum != b) {
    cerr << "Assertion failed: (sum == b)\n";
    return EXIT_FAILURE;
  }

  std::srand(seed);

  string cfg = getenv("XPLOAD_CONFIG_NAME") ? string(getenv("XPLOAD_CONFIG_NAME")) : "dev";
  xpload::Configurator config(cfg);

  for (int segment : segments)
  {
    this_thread::sleep_for(chrono::seconds(segment));

    auto [timestamp, tag, domain, payload] = random_tokens({1, 10}, {1, 10}, {1, 10});

    auto t1 = chrono::high_resolution_clock::now();
    xpload::Result result = xpload::fetch(tag, domain, timestamp, config);
    auto t2 = chrono::high_resolution_clock::now();

    chrono::duration<double, std::milli> td = t2 - t1;

    if (result.paths.size() != 1 || result.paths[0] != config.db.path + "/" + payload)
    {
      cerr << "Expected " << payload << " but got something else\n";
      return EXIT_FAILURE;
    } else {
      cout << "OK in " << td.count() << " ms after " << segment << " s " << result.byte_count << " B \"" << result.paths[0] << "\"\n";
    }
  }

  return EXIT_SUCCESS;
}
