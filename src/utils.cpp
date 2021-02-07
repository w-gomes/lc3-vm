#include "utils.hpp"

#include <conio.h>  // _kbhit
#include <windows.h>

#include <cstdint>
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

namespace vm {
auto sign_extend(tl::u16 x, int bit_count) noexcept -> tl::u16 {
  // extends a bit
  // e.g. 5bit -> 16bit
  //
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  if ((x >> (bit_count - 1)) & 1) { x |= (0xFFFF << bit_count); }
  return x;
}

auto destination(const std::bitset<16> &instr) noexcept -> tl::u16 {
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  return static_cast<tl::u16>((instr >> 9).to_ulong() & 0x7);
}
}  // namespace vm
