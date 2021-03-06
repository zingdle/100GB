#include <cassert>
#include <future>
#include <iostream>

#include "partition.h"

int main(int argc, char* argv[]) {
  if (argc != 2 && argc != 3 && argc != 4) {
    std::cerr << "usage: " << argv[0] << " <input> [partitions=16] [threads=1]"
              << std::endl;
    exit(-1);
  }

  // parameters
  std::string input = argv[1];
  int parts = 16;
  int threads = 1;
  if (argc >= 3) {
    parts = std::stoi(argv[2]);
    if (parts < 0) {
      std::cerr << "partitions=" << parts << " < 0" << std::endl;
      exit(-1);
    }
  }
  if (argc == 4) {
    threads = std::stoi(argv[3]);
    if (threads <= 0) {
      std::cerr << "threads=" << threads << " <= 0" << std::endl;
      exit(-1);
    }
  }
  if (parts < threads) {
    std::cerr << "parts < threads" << std::endl;
    exit(-1);
  }

  // split original file then get first unique word in partitions
  Partition p(input, parts, threads);
  p.split();
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
