#pragma once
#include <array>
#include <bitset>
#include <fstream>

#include "opcodes.hpp"
#include "tl/numeric-aliases.hpp"

namespace vm {
// Location address space.
static constexpr auto LAS      = 65536;
static constexpr auto REG_SIZE = 10;

// the starting memory is 0x3000 12888
static constexpr auto PC_START = 0x3000;

/*
 * LC-3 Arch:
 *
 * Memory: 16 bits locations (65,536)
 * Addresses are numbered from 0 (0x0000) to 65,535 (0xFFFF) (2^16 - 1)
 * Each address can contain a value of 16 bits
 *
 *
 * Registers: 10 total registers
 *            8 general purpose (R0 - R7)
 *            1 Program Counter (PC)
 *            1 Condition Flags (COND)
 *
 * Instruction Set (Op Code): 16 bit instructions
 * [15:12] stores the opcode
 * [11:0] stores the arguments
 *
 *
 * Program Counter (PC): 16 bit register containing the address of the next
 * instructions.
 * PC starts at address 0x3000.
 *
 */

class Virtual_Machine {
  using Registers       = std::array<tl::u16, REG_SIZE>;
  using Memory_Location = std::array<tl::u16, LAS>;

 public:
  auto run() -> void;
  auto read_file(const char *file) -> bool;
  auto memory_size() const -> tl::usize { return this->memory_.size(); }

  // debug
  tl::u16 get_addr(tl::u16 idx) const;

 private:
  // method
  auto sign_extend(tl::u16 x, int bit_count) const noexcept -> tl::u16;
  auto update_flags(tl::u16 r) noexcept -> void;
  auto read_memory(tl::u16 addr) -> tl::u16;
  auto write_memory(tl::u16 addr, tl::u16 content) -> void;
  auto destination(const std::bitset<16> &instr) const noexcept -> tl::u16;
  auto abort() noexcept -> void;

  // data
  Memory_Location memory_{};
  Registers register_{};
  bool running_{false};
};
}  // namespace vm
