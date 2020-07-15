#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "common.h"

class WordsReader {
 public:
  WordsReader(std::string input) : _input(input), _word_info_vec(VEC_SIZE) {}

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

  /* get <word, idx> from the file */
  virtual word_info_t get_next_word_info() = 0;

  /* get vector of <word, idx> from the file */
  virtual WordInfoVec& get_next_word_info_vec() = 0;

 protected:
  std::string _input;          // input filename
  std::ifstream _infile;       // input file
  WordInfoVec _word_info_vec;  // vector of word_info
};

/* `TopWordsReader` reads from original input file (level == 0) */
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

  WordInfoVec& get_next_word_info_vec() override {
    if (!is_open()) open();

    _word_info_vec.clear();
    word_t cur_word;
    while (_word_info_vec.size() < VEC_SIZE && _infile >> cur_word) {
      cur_idx++;
      _word_info_vec.push_back({cur_word, cur_idx});
    }
    return _word_info_vec;
  }

 private:
  idx_t cur_idx;
};

/* `DeeperWordsReader` reads from splitted input file (level > 0)*/
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

  WordInfoVec& get_next_word_info_vec() override {
    if (!is_open()) open();

    _word_info_vec.clear();
    word_t cur_word;
    idx_t cur_idx;
    while (_word_info_vec.size() < VEC_SIZE && _infile >> cur_word >> cur_idx) {
      _word_info_vec.push_back({cur_word, cur_idx});
    }
    return _word_info_vec;
  }
};
