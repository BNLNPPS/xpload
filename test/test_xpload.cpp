#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "xpload/xpload.h"


int main()
{
  using namespace std;

  string tag = "TestSmallGT2";
  uint64_t timestamp = 9999999999;

  vector<string> paths = xpload::fetch(tag, timestamp);

  for (const string& path : paths) 
    cout << path << '\n';

  return EXIT_SUCCESS;
} 
