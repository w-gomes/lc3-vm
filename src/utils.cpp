#include "utils.hpp"

#include <conio.h>  // _kbhit
#include <stdint.h>
#include <windows.h>

#include <cstdlib>

#include "fmt/format.h"

// windows stuffs
HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode;
DWORD fdwOldMode;

auto check_key() -> uint16_t {
  return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}

auto disable_input_buffering() -> void {
  hStdin = GetStdHandle(STD_INPUT_HANDLE);
  GetConsoleMode(hStdin, &fdwOldMode);  // save old mode

  // no input echo and return when one or more characters are available
  fdwMode = fdwOldMode ^ ENABLE_ECHO_INPUT ^ ENABLE_LINE_INPUT;

  SetConsoleMode(hStdin, fdwMode);  // set new mode
  FlushConsoleInputBuffer(hStdin);  // clear buffer
}

auto restore_input_buffering() -> void { SetConsoleMode(hStdin, fdwOldMode); }

auto handle_interrupt([[maybe_unused]] int signal) -> void {
  restore_input_buffering();
  fmt::print("\n");
  std::exit(-2);
}
