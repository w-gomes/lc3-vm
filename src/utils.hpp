#pragma once
#include <bitset>
#include <cstdint>

auto check_key() -> uint16_t;

auto disable_input_buffering() -> void;

auto restore_input_buffering() -> void;

auto handle_interrupt(int signal) -> void;

namespace vm {
#include "tl/numeric-aliases.hpp"

[[nodiscard]] auto sign_extend(tl::u16 x, tl::u16 bit_count) noexcept
  -> tl::u16;
[[nodiscard]] auto destination(const std::bitset<16> &instr) noexcept
  -> tl::u16;
}  // namespace vm
