#pragma once

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

#include "common.h"
#include "hash_func.h"
#include "words_reader.h"
#include "words_writer.h"

class Partition {
 public:
  Partition(std::string filename, int level = 0)
      : _level(level), _filename(filename), _ww(filename), _hash_func(level) {
    if (level == 0)
      _wr.reset(new TopWordsReader(filename));
    else
      _wr.reset(new DeeperWordsReader(filename));
  }

  /* get first unique <word, idx> */
  word_info_t get_first_unique() {
    word_t res_word;
    idx_t res_idx = IDX_NULL;

    // we haven't open yet
    _wr->open();

    // check in sub-partitions
    if (!_parts.empty()) {
      for (auto& parts : _parts) {
        auto info = parts->get_first_unique();
        auto word = info.word;
        auto idx = info.idx;
        if (idx < res_idx) {
          res_idx = idx;
          res_word = word;
        }
      }
      return {res_word, res_idx};
    }

    // no sub-partitions, check in this partition
    LOG(LOG_LEVEL_INFO, "getting first unique in %s", _filename.c_str());

    // populate `_unique_m` and `_dup_s`
    word_info_t info;
    while (info = _wr->get_next_word_info()) {
      add_unique(info);
    }

    // find first unique in `_uniqe_m`
    auto first_iter = std::min_element(
        _unique_m.begin(), _unique_m.end(),
        [](const auto& l, const auto& r) { return l.second < r.second; });
    if (first_iter != _unique_m.end()) {
      res_word = first_iter->first;
      res_idx = first_iter->second;
    }

    // free memory
    // TODO: maybe reusable?
    _unique_m = WordInfoMap();
    _dup_s = WordSet();

    return {res_word, res_idx};
  }

  /* add unique <word, idx> into this partition */
  void add_unique(word_info_t info, bool flushable = false) {
    auto word = info.word;
    auto idx = info.idx;

    if (_dup_s.count(word)) return;
    if (_unique_m.count(word)) {
      _unique_m.erase(word);
      _dup_s.insert(word);
      return;
    }
    _unique_m[word] = idx;

    if (flushable && _unique_m.size() + _dup_s.size() > FLUSH_LIMIT) flush();
  }

  size_t diskfile_size() {
    struct stat stat_buf;
    int rc = stat(_filename.c_str(), &stat_buf);
    if (rc != 0) {
      LOG(LOG_LEVEL_ERROR, "bad file: %s", _filename.c_str());
      std::abort();
    }
    return stat_buf.st_size;
  }

  /* associated diskfile is large, need deeper splitting */
  bool diskfile_is_large() { return diskfile_size() > SPLIT_LIMIT; }

  /* the input file is too large, split into sub-partitions */
  void split(int n = 16) {
    LOG(LOG_LEVEL_INFO, "splitting %s...", _filename.c_str());

    // create `n` parts
    for (int i = 0; i < n; i++) {
      _parts.emplace_back(std::make_unique<Partition>(
          _filename + "." + std::to_string(i), _level + 1));
      _parts[i]->_ww.open();
    }

    // split input words into `n` parts
    _wr->open();
    word_info_t info;
    idx_t cnt = 0;
    while (info = _wr->get_next_word_info()) {
      int part = _hash_func(info.word) % n;
      _parts[part]->add_unique(info, true);
    }
    _wr->close();

    // flush all parts
    for (auto& part : _parts) {
      part->flush();
      part->_ww.close();
    }

    // do more splits if needed
    for (auto& part : _parts) {
      if (part->diskfile_is_large()) part->split();
    }
  }

  // flush this part into disk when we're out of memory
  void flush() {
    _ww.dump(_unique_m);
    _unique_m.clear();
    _dup_s.clear();
  }

 private:
  int _level;                                      // partition level
  std::vector<std::unique_ptr<Partition>> _parts;  // sub-partitions
  std::string _filename;                           // input/output file
  std::unique_ptr<WordsReader> _wr;  // read <words, idx> from disk
  WordsWriter _ww;                   // write <words, idx> to disk
  WordInfoMap _unique_m;             // map for unique words
  WordSet _dup_s;                    // set for duplicate words
  MyHash _hash_func;                 // hash func for this partition
};
