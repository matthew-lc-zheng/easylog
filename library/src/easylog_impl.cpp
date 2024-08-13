#include "easylog_impl.hpp"

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iomanip>

static Log *_log_ = nullptr;
static std::condition_variable _cv_;
static std::mutex _mtx_;
static bool _printReady_ = false;
static bool _exitFlag_ = false;
const unsigned _initial_buffer_length_ = 10;

namespace easylog {
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
  _exitFlag_ = false;
  _processingBuffer = std::make_unique<MsgBuffer[]>(_initial_buffer_length_);
  _cacheBuffer = std::make_unique<MsgBuffer[]>(_initial_buffer_length_);
  _file.open(filePath, std::fstream::app);
  if (_file.is_open()) {
    _initialized = true;
  }
}

Log::~Log() {
  if (_initialized) {
    {
      std::lock_guard<std::mutex> lock(_mtx_log);
      _ready = false;
    }
    {
      std::lock_guard<std::mutex> lock(_mtx_);
      _exitFlag_ = true;
      _cv_.notify_all();
    }
    _worker.join();
    {
      std::lock_guard<std::mutex> lock(_mtx_buffer);
      for (size_t i = 0; i < _cacheBufferIdx; ++i) {
        printBufferToFile(_cacheBuffer[i]);
      }
    }
    _file.close();
  }
}

void Log::run() {
  if (!_initialized) {
    return;
  }
  _worker = std::thread([this]() {
    while (!_exitFlag_) {
      std::unique_lock<std::mutex> lock(_mtx_);
      _cv_.wait(lock, [] { return _printReady_ || _exitFlag_; });
      if (_printReady_) {
        _printReady_ = false;
        this->printBuffer();
      }
    }
  });
  _ready = true;
}

void Log::log(easylog::Level level, const char *msg) {
  if (level < _level) {
    return;
  }
  std::lock_guard<std::mutex> lock(_mtx_log);
  if (!_ready) {
    return;
  }
  _cacheBuffer[_cacheBufferIdx].msg = msg;
  _cacheBuffer[_cacheBufferIdx].level = level;
  _cacheBuffer[_cacheBufferIdx].date = getTime();
  ++_cacheBufferIdx;
  if (_cacheBufferIdx == _initial_buffer_length_) {
    _cacheBufferIdx = 0;
    swapBuffer();
    std::lock_guard<std::mutex> lock(_mtx_);
    _printReady_ = true;
    _cv_.notify_all();
  }
}

void Log::printBuffer() {
  std::lock_guard<std::mutex> lock(_mtx_buffer);
  for (size_t i = 0; i < _initial_buffer_length_; ++i) {
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
  std::lock_guard<std::mutex> lock(_mtx_buffer);
  std::swap(_cacheBuffer, _processingBuffer);
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
  _file << dateStr(buffer.date) << " [" << getLevelStr(buffer.level).c_str()
        << "] " << buffer.msg << std::endl;
}

std::string Log::dateStr(const Timestamp &ts) {
  std::ostringstream oss;
  oss << ts.year << "-";
  oss.fill('0');
  oss.width(2);
  oss << ts.month << "-";
  oss.width(2);
  oss << ts.day << " ";
  oss.width(2);
  oss << ts.hour << ":";
  oss.width(2);
  oss << ts.minute << ":";
  oss.width(2);
  oss << ts.second << ".";
  oss.width(3);
  oss << ts.millisecond << " ";
  return oss.str();
}