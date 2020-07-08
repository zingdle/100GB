#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>

#include "common.h"

// https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c

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

std::set<word_t> unique_words(std::vector<word_t> v) {
  std::unordered_map<word_t, int> m;
  std::set<word_t> s;

  for (auto& word : v) m[word]++;
  for (auto& word_cnt : m) {
    if (word_cnt.second == 1) s.insert(word_cnt.first);
  }

  return s;
}
