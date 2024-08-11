#pragma once

#include "easylog.hpp"

#include <fstream>
#include <thread>

struct Timestamp {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  long long millisecond;
};

struct MsgBuffer {
  std::string msg;
  easylog::Level level;
  Timestamp date;
};

class Log {
public:
  Log() = delete;
  Log(easylog::Level, const char *);
  ~Log();

  void run();
  void log(easylog::Level, const char *);
  void printBuffer();

private:
  Timestamp getTime();
  void swapBuffer();
  std::string getLevelStr(easylog::Level);
  void printBufferToFile(const MsgBuffer &);

private:
  bool _ready = false;
  bool _initialized = false;
  size_t _cacheBufferIdx = 0;

  std::ofstream _file;
  std::thread _worker;

  easylog::Level _level;
  MsgBuffer *_processingBuffer = nullptr;
  MsgBuffer *_cacheBuffer = nullptr;
};