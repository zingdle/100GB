#pragma once

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <future>
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
  Partition(std::string filename, int parts_n = 32, int threads_n = 1,
            int level = 0)
      : _parts_n(parts_n),
        _threads_n(threads_n),
        _level(level),
        _filename(filename),
        _ww(filename),
        _hash_func(level) {
    if (level == 0)
      _wr.reset(new TopWordsReader(filename));
    else
      _wr.reset(new DeeperWordsReader(filename));
  }

  void free_mem() {
    // TODO: maybe reusable?
    _unique_m = WordInfoMap();
    _dup_s = WordSet();
  }

  /* get first unique <word, idx> */
  word_info_t get_first_unique() {
    word_t res_word;
    idx_t res_idx = IDX_NULL;

    // we haven't open yet
    _wr->open();

    // check in sub-partitions
    if (!_parts.empty()) {
      // use `_thread_n` worker to `get_first_unique` in each partition
      int start = 0, end = 0;
      std::vector<Partition*> parts;
      for (auto& p : _parts) parts.push_back(p.get());
      std::vector<std::future<word_info_t>> word_info_futures;
      for (int i = 0; i < _threads_n; i++) {
        start = end;
        end = std::min(_parts_n, start + (_parts_n / _threads_n));
        // distribute _parts[start, end) to one thread
        auto word_info_future = std::async([&parts, start, end]() {
          word_info_t res{"", IDX_NULL};
          for (int i = start; i < end; i++) {
            auto info = parts[i]->get_first_unique();
            if (info.idx < res.idx) res = info;
          }
          return res;
        });
        word_info_futures.push_back(std::move(word_info_future));
      }

      word_info_t res{"", IDX_NULL};
      for (auto& info_future : word_info_futures) {
        auto info = info_future.get();
        if (info.idx < res.idx) res = info;
      }
      return res;
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

    free_mem();

    _wr->close();

    return {res_word, res_idx};
  }

  /* add unique <word, idx> into this partition */
  void add_unique(const word_info_t& info, bool flushable = false) {
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

  /* add vector of unique <word, idx> into this partition */
  void add_unique_vec(const WordInfoVec& info_vec, bool flushable = false) {
    for (auto& info : info_vec) {
      add_unique(info);
    }
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
  void split() {
    LOG(LOG_LEVEL_INFO, "splitting %s...", _filename.c_str());

    // create `n` parts
    for (int i = 0; i < _parts_n; i++) {
      _parts.emplace_back(
          std::make_unique<Partition>(_filename + "." + std::to_string(i),
                                      _parts_n, _threads_n, _level + 1));
      _parts[i]->_ww.open();
    }

    // split input words into `n` parts
    _wr->open();

    std::vector<WordInfoVec> vecs(_parts_n);
    for (auto& v : vecs) v.reserve(VEC_SIZE);
    std::vector<Partition*> parts;
    for (auto& p : _parts) parts.push_back(p.get());
    while (true) {
      auto& info_vec = _wr->get_next_word_info_vec();
      if (info_vec.empty()) break;

      // distribute `info_vecs` by hash value
      for (auto& vec : vecs) vec.clear();
      for (auto& info : info_vec) {
        vecs[_hash_func(info.word) % _parts_n].push_back(info);
      }

      // each worker for parts of partitions
      int start = 0, end = 0;
      std::vector<std::future<void>> fs;
      for (int i = 0; i < _threads_n; i++) {
        start = end;
        end = std::min(_parts_n, start + (_parts_n / _threads_n));
        auto f = std::async([&parts, start, end, &vecs]() {
          for (int i = start; i < end; i++) {
            parts[i]->add_unique_vec(vecs[i], true);
          }
        });
        fs.push_back(std::move(f));
      }
      for (auto& f : fs) f.wait();
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

  /* flush this part into disk when we're out of memory */
  void flush() {
    _ww.dump(_unique_m);
    _unique_m.clear();
    _dup_s.clear();
  }

 private:
  int _parts_n;                                    // # of partitions
  int _threads_n;                                  // parallel threads
  int _level;                                      // partition level
  std::vector<std::unique_ptr<Partition>> _parts;  // sub-partitions
  std::string _filename;                           // input/output file
  std::unique_ptr<WordsReader> _wr;  // read <words, idx> from disk
  WordsWriter _ww;                   // write <words, idx> to disk
  WordInfoMap _unique_m;             // map for unique words
  WordSet _dup_s;                    // set for duplicate words
  MyHash _hash_func;                 // hash func for this partition
};
