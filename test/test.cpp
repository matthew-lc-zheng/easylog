#include "easylog.hpp"

int main() {
  el_launch("./test.log");
  el_info("yes!");
  el_terminate();
  return 0;
}