#pragma once

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) ||                  \
    defined(__WIN64__) || defined(WIN32) || defined(_WIN32) ||                 \
    defined(__WIN32__) || defined(__NT__)
#ifdef EASYLOG
#define EASYLOG_EXPORT __declspec(dllexport)
#else
#define EASYLOG_EXPORT __declspec(dllimport)
#endif
#else
#define EASYLOG_EXPORT __attribute__((visibility("default")))
#endif

#include <sstream>
#include <string>

#define el_launch(path) easylog::launch(path)
#define el_terminate() easylog::terminate()
#define el_trace(msg)                                                          \
  {                                                                            \
    std::stringstream ss;                                                      \
    ss << msg;                                                                 \
    easylog::trace(ss.str().c_str());                                          \
  }
#define el_debug(msg)                                                          \
  {                                                                            \
    std::stringstream ss;                                                      \
    ss << msg;                                                                 \
    easylog::debug(ss.str().c_str());                                          \
  }
#define el_info(msg)                                                           \
  {                                                                            \
    std::stringstream ss;                                                      \
    ss << msg;                                                                 \
    easylog::info(ss.str().c_str());                                           \
  }
#define el_warn(msg)                                                           \
  {                                                                            \
    std::stringstream ss;                                                      \
    ss << msg;                                                                 \
    easylog::warn(ss.str().c_str());                                           \
  }
#define el_error(msg)                                                          \
  {                                                                            \
    std::stringstream ss;                                                      \
    ss << msg;                                                                 \
    easylog::error(ss.str().c_str());                                          \
  }
#define el_fatal(msg)                                                          \
  {                                                                            \
    std::stringstream ss;                                                      \
    ss << msg;                                                                 \
    easylog::fatal(ss.str().c_str());                                          \
  }

namespace easylog {
enum class Level { trace, debug, info, warn, error, fatal };

void EASYLOG_EXPORT launch(const char *, Level level = Level::info);
void EASYLOG_EXPORT terminate();

void EASYLOG_EXPORT trace(const char *);
void EASYLOG_EXPORT debug(const char *);
void EASYLOG_EXPORT info(const char *);
void EASYLOG_EXPORT warn(const char *);
void EASYLOG_EXPORT error(const char *);
void EASYLOG_EXPORT fatal(const char *);
} // namespace easylog