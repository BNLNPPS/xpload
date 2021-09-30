#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>
#include <string>

#include "xpload/xpload.h"


int main()
{
  using namespace std;

  uint64_t timestamp = 9999999999;

  optional<string> path = xpload::fetch(timestamp);

  if (path)
    cout << path.value() << '\n';

  return EXIT_SUCCESS;
} 
