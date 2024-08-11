#include "easylog.hpp"

int main() {
  el_launch("./test.log");
  for (size_t i = 0; i < 100; i++) {
    el_info("yes! " << i);
  }
  el_terminate();
  return 0;
}