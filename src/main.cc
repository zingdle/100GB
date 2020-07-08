#include <cassert>
#include <iostream>

#include "partition.h"

int main(int argc, char* argv[]) {
  if (argc != 2 && argc != 3) {
    std::cerr << "usage: " << argv[0] << " <input> [partitions=16]"
              << std::endl;
    exit(-1);
  }

  // parameters
  std::string input = argv[1];
  int parts = 16;
  if (argc == 3) {
    parts = std::stoi(argv[2]);
    if (parts < 0) {
      std::cerr << "partitions=" << parts << " < 0" << std::endl;
      exit(-1);
    }
  }

  // split original file then get first unique word in partitions
  Partition p(input);
  p.split(parts);
  auto info = p.get_first_unique();

  if (!info) {
    std::cout << "not found" << std::endl;
    return 0;
  }

  std::cout << "found:" << std::endl;
  std::cout << "word: " << info.word << std::endl;
  std::cout << "idx: " << info.idx << std::endl;

  return 0;
}
