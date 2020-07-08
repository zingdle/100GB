#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>

#include "common.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <input>" << std::endl;
    exit(-1);
  }

  std::string input = argv[1];
  std::unordered_map<word_t, int> word_counts;
  std::unordered_map<word_t, idx_t> word_idxs;

  std::ifstream infile(input);

  LOG(LOG_LEVEL_INFO, "reading into mem begins");
  word_t word;
  std::vector<word_t> words;
  while (infile >> word) {
    words.push_back(word);
  }
  LOG(LOG_LEVEL_INFO, "reading into mem end");

  LOG(LOG_LEVEL_INFO, "real task begins");
  std::set<std::string> _dup_s;
  std::unordered_map<std::string, uint64_t> _unique_m;
  uint64_t idx = 0;
  for (auto& word : words) {
    if (_dup_s.count(word)) continue;
    if (_unique_m.count(word)) {
      _unique_m.erase(word);
      _dup_s.insert(word);
      continue;
    }
    _unique_m[word] = ++idx;
  }

  word = "";
  idx = IDX_NULL;
  auto first_iter = std::min_element(
      _unique_m.begin(), _unique_m.end(),
      [](const auto& l, const auto& r) { return l.second < r.second; });
  if (first_iter != _unique_m.end()) {
    word = first_iter->first;
    idx = first_iter->second;
  }
  LOG(LOG_LEVEL_INFO, "real task ends");

  if (idx == IDX_NULL) {
    std::cout << "not found" << std::endl;
    return 0;
  }

  std::cout << "found:" << std::endl;
  std::cout << "word: " << word << std::endl;
  std::cout << "idx: " << idx << std::endl;

  return 0;
}
