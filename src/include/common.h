#pragma once

#include <sys/time.h>

#include <string>
#include <unordered_map>

using idx_t = uint64_t;
using word_t = std::string;
using WordInfoMap = std::unordered_map<word_t, idx_t>;
using WordSet = std::set<word_t>;

#define IDX_NULL ((idx_t)(~0))

struct word_info_t {
  word_t word;
  idx_t idx;
  operator bool() const { return idx != IDX_NULL; }
};

#define FLUSH_LIMIT (1 * 1024 * 1024)    // # of element
#define SPLIT_LIMIT (160 * 1024 * 1024)  // file size

/////////////////////////// logger ///////////////////////////

#define LOG_LEVEL_FATAL 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4

#ifndef LOG_PRINT_LEVEL
#define LOG_PRINT_LEVEL LOG_LEVEL_INFO
#endif

#define PRETTY_PRINT

#define LOG(lvl, fmt, args...)                                \
  do {                                                        \
    if (lvl <= LOG_PRINT_LEVEL) {                             \
      char buf[128];                                          \
      LOG_HEADER(buf, lvl, __FILE__, __LINE__, __FUNCTION__); \
      fprintf(stderr, "%s" fmt "\n", buf, ##args);            \
    }                                                         \
  } while (0);

inline void LOG_HEADER(char *buf, const int lvl, const char *file,
                       const int line, const char *func) {
  struct timeval tv;
  const char *lvlstr;

  gettimeofday(&tv, NULL);

#ifdef PRETTY_PRINT
  char tmbuf[64], tsbuf[64];
  struct tm *nowtm;

  nowtm = localtime(&tv.tv_sec);
  strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
  snprintf(tsbuf, sizeof tsbuf, "%s", tmbuf);
#else
  double curtime = (double)tv.tv_sec + tv.tv_usec / 1000000.0;
#endif

  switch (lvl) {
    case LOG_LEVEL_FATAL:
      lvlstr = "F";
      break;
    case LOG_LEVEL_ERROR:
      lvlstr = "E";
      break;
    case LOG_LEVEL_WARN:
      lvlstr = "W";
      break;
    case LOG_LEVEL_INFO:
      lvlstr = "I";
      break;
    case LOG_LEVEL_DEBUG:
      lvlstr = "D";
      break;
    default:
      lvlstr = "?";
      break;
  };

#ifdef PRETTY_PRINT
  sprintf(buf, "%s| %s (%s:%d:%s) ", lvlstr, tsbuf, file, line, func);
#else
  sprintf(buf, "%s| %f (%s:%d:%s) ", lvlstr, curtime, file, line, func);
#endif
}
