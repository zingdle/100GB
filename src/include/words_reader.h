#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "common.h"

class WordsReader {
 public:
  WordsReader(std::string input) : _input(input) {}

  virtual ~WordsReader() { _infile.close(); }

  bool is_open() { return _infile.is_open(); }

  /* open partition file */
  void open() {
    if (_infile.is_open()) return;

    _infile.open(_input);

    if (!_infile.is_open()) {
      LOG(LOG_LEVEL_ERROR, "opening file failed: %s", _input.c_str());
      std::abort();
    }
  }

  /* close partition file */
  void close() {
    if (_infile.is_open()) _infile.close();
  }

  virtual word_info_t get_next_word_info() = 0;

 protected:
  std::string _input;     // input filename
  std::ifstream _infile;  // input file
};

class TopWordsReader : public WordsReader {
 public:
  TopWordsReader(std::string input) : WordsReader(input), cur_idx(0) {}

  word_info_t get_next_word_info() override {
    if (!is_open()) open();

    word_t cur_word;
    if (_infile >> cur_word) {
      cur_idx++;
      return {cur_word, cur_idx};
    }

    return {"", IDX_NULL};
  }

 private:
  idx_t cur_idx;
};

class DeeperWordsReader : public WordsReader {
 public:
  DeeperWordsReader(std::string input) : WordsReader(input) {}

  word_info_t get_next_word_info() override {
    if (!is_open()) open();

    word_t cur_word;
    idx_t cur_idx;
    if (_infile >> cur_word >> cur_idx) {
      return {cur_word, cur_idx};
    }
    return {"", IDX_NULL};
  }
};
