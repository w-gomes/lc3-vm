#include <csignal>

#include "fmt/format.h"
#include "utils.hpp"
#include "vm.hpp"

auto main(int argc, const char* argv[]) -> int {
  // image-file must be passed as argument.
  if (argc < 2) {
    fmt::print(stderr, "{}\n", "Error! Usage: vm.exe [image-file] ...");
    return -1;
  }

  auto vm = vm::Virtual_Machine();

  // load the image-file
  for (auto i = 1; i < argc; ++i) {
    if (!vm.read_file(argv[i])) {
      fmt::print(stderr, "{} {}\n", "Failed to load image:", argv[i]);
      return -1;
    }
  }

  signal(SIGINT, handle_interrupt);
  disable_input_buffering();

  vm.run();

  restore_input_buffering();

  return 0;
}
