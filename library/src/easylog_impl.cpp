#include "easylog_impl.hpp"
#ifdef _WIN32
#include <errhandlingapi.h>
#endif

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <stdexcept>

namespace easylog {
Log *_log_ = nullptr;
std::condition_variable _cv_;
std::mutex _mtx_;
bool _printReady_ = false;
bool _exitFlag_ = false;
const unsigned _initial_buffer_length_ = 100;

void launch(const char *filePath, Level level) {
  if (!_log_) {
    _log_ = new Log(level, filePath);
    _log_->run();
  }
}
void terminate() {
  if (_log_) {
    auto temp = _log_;
    _log_ = nullptr;
    delete temp;
  }
}

void trace(const char *msg) {
  if (_log_) {
    _log_->log(Level::trace, msg);
  }
}
void debug(const char *msg) {
  if (_log_) {
    _log_->log(Level::debug, msg);
  }
}
void info(const char *msg) {
  if (_log_) {
    _log_->log(Level::info, msg);
  }
}
void warn(const char *msg) {
  if (_log_) {
    _log_->log(Level::warn, msg);
  }
}
void error(const char *msg) {
  if (_log_) {
    _log_->log(Level::error, msg);
  }
}
void fatal(const char *msg) {
  if (_log_) {
    _log_->log(Level::fatal, msg);
  }
}
} // namespace easylog

Log::Log(easylog::Level level, const char *filePath) : _level(level) {
  easylog::_exitFlag_ = false;
  _processingBuffer = new MsgBuffer[easylog::_initial_buffer_length_];
  _cacheBuffer = new MsgBuffer[easylog::_initial_buffer_length_];
  _file.open(filePath, std::fstream::app);
  if (_file.is_open()) {
    _initialized = true;
  }
}

Log::~Log() {
  if (_initialized) {
    _ready = false;
    {
      std::lock_guard<std::mutex> lock(easylog::_mtx_);
      easylog::_exitFlag_ = true;
      easylog::_cv_.notify_all();
    }
    _worker.join();
    for (size_t i = 0; i < _cacheBufferIdx; ++i) {
      printBufferToFile(_cacheBuffer[i]);
    }
    _file.close();
  }
}

void Log::run() {
  if (!_initialized) {
    return;
  }
  _worker = std::thread([this]() {
    while (!easylog::_exitFlag_) {
      std::unique_lock<std::mutex> lock(easylog::_mtx_);
      easylog::_cv_.wait(
          lock, [] { return easylog::_printReady_ || easylog::_exitFlag_; });
      if (easylog::_printReady_) {
        easylog::_printReady_ = false;
        this->printBuffer();
      }
    }
  });
  _ready = true;
}

void Log::log(easylog::Level level, const char *msg) {
  if (!_ready || level < _level) {
    return;
  }

  if (_cacheBufferIdx < easylog::_initial_buffer_length_) {
    _cacheBuffer[_cacheBufferIdx].msg = msg;
    _cacheBuffer[_cacheBufferIdx].level = level;
    _cacheBuffer[_cacheBufferIdx].date = getTime();
  } else {
    _cacheBufferIdx = 0;
    std::lock_guard<std::mutex> lock(easylog::_mtx_);
    swapBuffer();
    easylog::_printReady_ = true;
    easylog::_cv_.notify_all();
  }
  ++_cacheBufferIdx;
}
void Log::printBuffer() {
  for (size_t i = 0; i < easylog::_initial_buffer_length_; ++i) {
    printBufferToFile(_processingBuffer[i]);
  }
}

Timestamp Log::getTime() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  std::tm *now_tm = std::localtime(&now_time);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;
  Timestamp ts{now_tm->tm_year + 1900,
               now_tm->tm_mon + 1,
               now_tm->tm_mday,
               now_tm->tm_hour,
               now_tm->tm_min,
               now_tm->tm_sec,
               ms.count()};

  return ts;
}

void Log::swapBuffer() {
  auto temp = _cacheBuffer;
  _cacheBuffer = _processingBuffer;
  _processingBuffer = temp;
}

std::string Log::getLevelStr(easylog::Level level) {
  std::string levelStr;
  switch (level) {
  case easylog::Level::trace:
    levelStr = "trace";
    break;
  case easylog::Level::debug:
    levelStr = "debug";
    break;
  case easylog::Level::info:
    levelStr = "info";
    break;
  case easylog::Level::warn:
    levelStr = "warn";
    break;
  case easylog::Level::error:
    levelStr = "error";
    break;
  default:
    levelStr = "fatal";
  }
  return levelStr;
}

void Log::printBufferToFile(const MsgBuffer &buffer) {
  _file << buffer.date.year << "-" << buffer.date.month << "-"
        << buffer.date.day << " " << buffer.date.hour << ":"
        << buffer.date.minute << ":" << buffer.date.second << "."
        << buffer.date.millisecond << " [" << getLevelStr(buffer.level).c_str()
        << "] " << buffer.msg << std::endl;
}