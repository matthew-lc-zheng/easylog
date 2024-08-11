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
/* public interface */
#define el_launch(...) easylog::launch(__VA_ARGS__)
#define el_terminate easylog::terminate

#define el_trace(msg) el_private_stream_format(easylog::trace, msg)
#define el_debug(msg) el_private_stream_format(easylog::debug, msg)
#define el_info(msg) el_private_stream_format(easylog::info, msg)
#define el_warn(msg) el_private_stream_format(easylog::warn, msg)
#define el_error(msg) el_private_stream_format(easylog::error, msg)
#define el_fatal(msg) el_private_stream_format(easylog::fatal, msg)

/* private usage */
#define el_private_stream_format(logger, msg)                                  \
  {                                                                            \
    std::stringstream ss;                                                      \
    ss << msg;                                                                 \
    logger(ss.str().c_str());                                                  \
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