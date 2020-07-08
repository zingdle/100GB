#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <fstream>
#include <vector>

#include "doctest.h"
#include "utils.h"
#include "words_reader.h"

TEST_CASE("test TopWordsReader") {
  const char* tmp_file = "/tmp/sample.txt";
  const int n = 100000;
  std::vector<word_t> words;

  // populate random words
  for (int i = 0; i < n; i++) words.push_back(random_string(5));

  // write words to tmp file
  std::ofstream out(tmp_file);
  for (int i = 0; i < n; i++) out << words[i] << " ";
  out.close();

  // read words
  WordsReader* wr = new TopWordsReader(tmp_file);
  wr->open();

  for (int i = 0; i < n; i++) {
    auto info = wr->get_next_word_info();
    CHECK(info.word == words[i]);
    CHECK(info.idx == i + 1);
  }

  // no more words
  auto info = wr->get_next_word_info();
  CHECK(info.word == "");
  CHECK(info.idx == IDX_NULL);

  wr->close();
  delete wr;
}

TEST_CASE("test DeeperWordsReader") {
  const char* tmp_file = "/tmp/sample.txt";
  const int n = 100000;
  std::vector<word_t> words;
  std::vector<idx_t> idxs;

  // populate random words and indexs
  for (int i = 0; i < n; i++) words.push_back(random_string(5));
  for (int i = 0; i < n; i++) idxs.push_back(rand() % n);

  // write words to tmp file
  std::ofstream out(tmp_file);
  for (int i = 0; i < n; i++) out << words[i] << " " << idxs[i] << " ";
  out.close();

  // read words
  WordsReader* wr = new DeeperWordsReader(tmp_file);
  wr->open();

  for (int i = 0; i < n; i++) {
    auto info = wr->get_next_word_info();
    CHECK(info.word == words[i]);
    CHECK(info.idx == idxs[i]);
  }

  // no more words
  auto info = wr->get_next_word_info();
  CHECK(info.word == "");
  CHECK(info.idx == IDX_NULL);

  wr->close();
  delete wr;
}
