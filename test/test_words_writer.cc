#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <fstream>
#include <vector>

#include "doctest.h"
#include "utils.h"
#include "words_reader.h"
#include "words_writer.h"

TEST_CASE("test WordsWriter") {
  const char* tmp_file = "/tmp/sample.txt";
  const int n = 100000;
  std::vector<word_t> words;
  std::vector<idx_t> idxs;

  // populate word info map
  WordInfoMap m;
  for (int i = 0; i < n; i++) words.push_back(random_string(5));
  for (int i = 0; i < n; i++) idxs.push_back(rand() % n);
  for (int i = 0; i < n; i++) m[words[i]] = idxs[i];

  // write map to tmp file
  std::ofstream out(tmp_file);
  for (auto& info : m) {
    auto word = info.first;
    auto idx = info.second;
    out << word << " " << idx << " ";
  }
  out.close();

  // write map
  WordsWriter ww(tmp_file);
  ww.open();
  ww.dump(m);
  ww.close();

  // read dumped map
  WordsReader* wr = new DeeperWordsReader(tmp_file);
  word_info_t info;
  int cnt = 0;
  wr->open();
  while (info = wr->get_next_word_info()) {
    CHECK(m.count(info.word));
    CHECK(m[info.word] == info.idx);
    cnt++;
  }
  CHECK(cnt == m.size());
  wr->close();
  delete wr;
}
