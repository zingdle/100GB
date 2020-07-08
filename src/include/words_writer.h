#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "common.h"

class WordsWriter {
 public:
  WordsWriter(std::string output) : _output(output) {}

  ~WordsWriter() { _outfile.close(); }

  bool is_open() { return _outfile.is_open(); }

  /* open file */
  void open() {
    if (_outfile.is_open()) {
      return;
    }

    _outfile.open(_output);

    if (!_outfile.is_open()) {
      LOG(LOG_LEVEL_ERROR, "opening file failed: %s", _output.c_str());
      std::abort();
    }
  }

  void close() {
    if (_outfile.is_open()) _outfile.close();
  }

  /* dump WordInfoMap into disk */
  void dump(WordInfoMap& wm) {
    // TODO: better compression method
    if (!is_open()) open();

    for (auto& word_info : wm) {
      auto word = word_info.first;
      auto idx = word_info.second;
      std::string record = word + " " + std::to_string(idx) + " ";
      _outfile.write(record.c_str(), record.size());
    }
    _outfile.flush();
  }

 private:
  std::string _output;     // output filename
  std::ofstream _outfile;  // output file
};
