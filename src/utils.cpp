#include "utils.hpp"

#ifdef _WIN32
  #include <conio.h>  // _kbhit
  #include <windows.h>
#endif

#ifdef linux
  #include <fcntl.h>
  #include <sys/mman.h>
  #include <sys/termios.h>
  #include <sys/time.h>
  #include <sys/types.h>
  #include <unistd.h>
#endif

#include <cstdint>
#include <cstdlib>

#include "fmt/format.h"

// Input buffering windows
#ifdef _WIN32
HANDLE hStdin = INVALID_HANDLE_VALUE;  // NOLINT
DWORD fdwMode;                         // NOLINT
DWORD fdwOldMode;                      // NOLINT

auto check_key() -> uint16_t {
  return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}

auto disable_input_buffering() -> void {
  hStdin = GetStdHandle(STD_INPUT_HANDLE);
  GetConsoleMode(hStdin, &fdwOldMode);  // save old mode

  // no input echo and return when one or more characters are available
  fdwMode = fdwOldMode ^ ENABLE_ECHO_INPUT ^ ENABLE_LINE_INPUT;  // NOLINT

  SetConsoleMode(hStdin, fdwMode);  // set new mode
  FlushConsoleInputBuffer(hStdin);  // clear buffer
}

auto restore_input_buffering() -> void { SetConsoleMode(hStdin, fdwOldMode); }
#endif

// Input buffering linux
#ifdef linux
auto check_key() -> uint16_t {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);

  struct timeval timeout;
  timeout.tv_sec  = 0;
  timeout.tv_usec = 0;
  return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

struct termios original_tio;

auto disable_input_buffering() -> void {
  tcgetattr(STDIN_FILENO, &original_tio);
  struct termios new_tio = original_tio;
  new_tio.c_lflag &= ~ICANON & ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

auto restore_input_buffering() -> void {
  tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}
#endif

auto handle_interrupt([[maybe_unused]] int signal) -> void {
  restore_input_buffering();
  fmt::print("\n");
  std::exit(-2);  // NOLINT
}

namespace vm {
[[nodiscard]] auto sign_extend(tl::u16 x, tl::u16 bit_count) noexcept
  -> tl::u16 {
  // extends a bit
  // e.g. 5bit -> 16bit
  //
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  if ((x >> (bit_count - 1)) & 1) { x |= (0xFFFF << bit_count); }
  return x;
}

[[nodiscard]] auto destination(const std::bitset<16> &instr) noexcept
  -> tl::u16 {
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  return static_cast<tl::u16>((instr >> 9).to_ulong() & 0x7);
}
}  // namespace vm
