#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <algorithm>
#include <fstream>
#include <vector>

#include "doctest.h"
#include "partition.h"
#include "utils.h"

TEST_CASE("test add_unique and flush") {
  const char* tmp_file = "/tmp/sample.txt";
  const int n = 100000;
  std::vector<word_t> words;
  std::vector<idx_t> idxs;
  Partition p(tmp_file);

  // fresh test
  for (int i = 0; i < n; i++) words.push_back(random_string(5));
  for (int i = 0; i < n; i++) idxs.push_back(rand() % n);
  auto s = unique_words(words);

  // add_unique and flush
  for (int i = 0; i < n; i++) p.add_unique({words[i], idxs[i]});
  p.flush();

  WordsReader* wr = new DeeperWordsReader(tmp_file);
  wr->open();

  word_info_t info;
  int cnt = 0;
  while (info = wr->get_next_word_info()) {
    CHECK(s.count(info.word));
    cnt++;
  }
  CHECK(cnt == s.size());

  wr->close();
  delete wr;
}

TEST_CASE("test split and get_first_unique") {
  const char* tmp_file = "/tmp/sample.txt";
  const int n = 100000;
  const int parts = 16;
  std::vector<word_t> words;
  std::vector<idx_t> idxs;
  std::unordered_map<word_t, idx_t> m;

  // populate random words and indexs
  for (int i = 0; i < n; i++) words.push_back(random_string(5));
  for (int i = 0; i < n; i++) m[words[i]] = i + 1;

  // write words to tmp file
  std::ofstream out(tmp_file);
  for (int i = 0; i < n; i++) out << words[i] << " ";
  out.close();

  // split
  Partition p(tmp_file);
  p.split();
  std::vector<word_t> split_words;
  for (int p = 0; p < parts; p++) {
    WordsReader* wr =
        new DeeperWordsReader(tmp_file + std::string(".") + std::to_string(p));
    wr->open();
    word_info_t info;
    while (info = wr->get_next_word_info()) {
      MyHash hash_func(0);
      CHECK(hash_func(info.word) % parts == p);
      split_words.push_back(info.word);
    }
    wr->close();
    delete wr;
  }
  std::set<word_t> words_set(words.begin(), words.end());
  for (auto& word : split_words) {
    CHECK(words_set.count(word));
  }

  // get_first_unique
  auto info = p.get_first_unique();
  auto word = info.word;
  auto idx = info.idx;
  auto s = unique_words(words);
  CHECK(s.count(word));
  CHECK(m[word] == idx);
  for (auto sword : s) {
    CHECK(m[sword] >= idx);
  }
}

TEST_CASE("test multithread") {
  const char* tmp_file = "/tmp/sample.txt";
  const int n = 100000;
  const int parts = 16;
  std::vector<word_t> words;
  std::vector<idx_t> idxs;
  std::unordered_map<word_t, idx_t> m;

  // populate random words and indexs
  for (int i = 0; i < n; i++) words.push_back(random_string(5));
  for (int i = 0; i < n; i++) m[words[i]] = i + 1;

  // write words to tmp file
  std::ofstream out(tmp_file);
  for (int i = 0; i < n; i++) out << words[i] << " ";
  out.close();

  // split
  Partition p(tmp_file, 16, 4);
  p.split();
  std::vector<word_t> split_words;
  for (int p = 0; p < parts; p++) {
    WordsReader* wr =
        new DeeperWordsReader(tmp_file + std::string(".") + std::to_string(p));
    wr->open();
    word_info_t info;
    while (info = wr->get_next_word_info()) {
      MyHash hash_func(0);
      CHECK(hash_func(info.word) % parts == p);
      split_words.push_back(info.word);
    }
    wr->close();
    delete wr;
  }
  std::set<word_t> words_set(words.begin(), words.end());
  for (auto& word : split_words) {
    CHECK(words_set.count(word));
  }

  // get_first_unique
  auto info = p.get_first_unique();
  auto word = info.word;
  auto idx = info.idx;
  auto s = unique_words(words);
  CHECK(s.count(word));
  CHECK(m[word] == idx);
  for (auto sword : s) {
    CHECK(m[sword] >= idx);
  }
}
