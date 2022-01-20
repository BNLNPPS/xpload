#include <algorithm>
#include <chrono>
#include <cmath>
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

struct Tokens {
  uint64_t timestamp;
  std::string tag, domain, payload;
};

auto random_tokens(std::pair<int, int> tag_range, std::pair<int, int> dom_range, std::pair<int, int> tst_range)
{
  uint64_t timestamp  = tst_range.first + std::rand() % tst_range.second;
  int tag_index = tag_range.first + std::rand() % tag_range.second;
  int dom_index = dom_range.first + std::rand() % dom_range.second;

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

  vector<int> segments = split_interval(b, n);

  int sum = accumulate(segments.begin(), segments.end(), 0);
  if (sum != b) {
    cerr << "Assertion failed: (sum == b)\n";
    return EXIT_FAILURE;
  }

  std::srand(rand_seed);

  string cfg = getenv("XPLOAD_CONFIG_NAME") ? string(getenv("XPLOAD_CONFIG_NAME")) : "test";
  xpload::Configurator config(cfg);

  for (int segment : segments)
  {
    this_thread::sleep_for(chrono::seconds(segment));

    Tokens tk = random_tokens({1, 100}, {1, 10}, {1, 1000});

    auto t0 = chrono::system_clock::now();
    auto t1 = chrono::high_resolution_clock::now();
    xpload::Result result = xpload::fetch(tk.tag, tk.domain, tk.timestamp, config);
    auto t2 = chrono::high_resolution_clock::now();

    chrono::duration<double, std::milli> td = t2 - t1;

    if (result.paths.size() != 1)
    {
      cerr << "Expected single payload but got " << result.paths.size() << "\n";
      return EXIT_FAILURE;
    } else if ( result.paths[0] != config.db.path + "/" + tk.payload) {
      cerr << "Expected " << tk.payload << " but got " << result.paths[0] << "\n";
      return EXIT_FAILURE;
    } else {
      if (config.db.verbosity > 1)
        cout << "OK in " << td.count() << " ms after " << segment << " s " << result.byte_count << " B \"" << result.paths[0] << "\"\n";
      else if (config.db.verbosity > 0)
        cout << chrono::system_clock::to_time_t(t0) << ", " << td.count() << ", " << segment << ", "
             << result.byte_count << ", "
             << result.response_code << ", \""
             << result.paths[0] << "\"\n";
    }
  }

  return EXIT_SUCCESS;
}
