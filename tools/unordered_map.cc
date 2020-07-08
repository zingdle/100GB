// run with `mprof`

#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

std::string random_string(size_t length) {
  auto randchar = []() -> char {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

int main() {
  std::unordered_map<std::string, uint64_t> m;

  for (int i = 0; i < 10; i++) {
    std::cerr << i << std::endl;
    for (int j = 1; j < 100000; j++) {
      m[random_string(7)] = 1;
    }
    sleep(10);
  }
  return 0;
}
