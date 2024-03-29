#include "vm.hpp"

#include <algorithm>
#include <bitset>
#include <cstdio>  // std::getchar, std::FILE
#include <ranges>

#include "fmt/format.h"
#include "utils.hpp"

namespace vm {

auto Virtual_Machine::run() -> void {  // NOLINT
  this->register_[Register::PC] = PC_START;

  this->running_ = true;

  fmt::print("{}\n", "Starting lc-3 virtual machine");

  while (this->running_) {
    // load the instruction from memory
    const auto instruction =
      std::bitset<16>(this->read_memory(this->register_[Register::PC]));

    this->register_[Register::PC]++;  // increment the memory in PC

    // Instruction set is 16 bit. the first 4 bits store the opcode.
    // To get the opcodes we right-shift by 12.
    const auto op = instruction >> 12;

    switch (op.to_ulong()) {
      case Op_Code::BR: {
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto condition = (instruction >> 9).to_ulong() & 0x7;
        if (condition & this->register_[Register::COND]) {
          this->register_[Register::PC] += sign_extend(
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            (static_cast<tl::u16>(instruction.to_ulong()) & 0x1FF),
            9);
        }
        break;
      }
      case Op_Code::ADD: {
        // 5th bit in the instructions: immediate mode or register mode
        // 0 -> register mode
        // 1 -> immediate mode (reads the value from the instructions)

        // we do this because the registers are only 3 bits (INDEX)
        // 0b111
        const auto dr = destination(instruction);
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto sr1 = (instruction >> 6).to_ulong() & 0x7;

        if (instruction.test(5)) {  // immediate mode
          // bit [4:0] & 0b11111
          // NOLINTNEXTLINE(hicpp-signed-bitwise)
          const auto imm5 = instruction.to_ulong() & 0x1F;
          this->register_[dr] =
            this->register_[sr1] +
            sign_extend(static_cast<tl::u16>(imm5), 5);  // imm5 is 5bits.

        } else {  // register mode

          // NOLINTNEXTLINE(hicpp-signed-bitwise)
          const auto sr2      = instruction.to_ulong() & 0x7;
          this->register_[dr] = this->register_[sr1] + this->register_[sr2];
        }

        // set condition flags
        this->update_flags(static_cast<tl::u16>(dr));
        break;
      }
      case Op_Code::LD: {
        // loads the content of an address
        const auto dr = destination(instruction);
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto pc_offset9 = instruction.to_ulong() & 0x1FF;
        const auto sext_pc_offset9 =
          sign_extend(static_cast<tl::u16>(pc_offset9), 9);
        const auto mem_location =
          tl::u16(this->register_[Register::PC] + sext_pc_offset9);

        this->register_[dr] = this->read_memory(mem_location);
        this->update_flags(static_cast<tl::u16>(dr));

        break;
      }
      case Op_Code::ST: {
        // store register in memory
        const auto sr = destination(instruction);
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto pc_offset9 = instruction.to_ulong() & 0x1FF;
        const auto sext_pc_offset9 =
          sign_extend(static_cast<tl::u16>(pc_offset9), 9);

        const auto dest =
          tl::u16(this->register_[Register::PC] + sext_pc_offset9);
        this->write_memory(dest, this->register_[sr]);
        break;
      }
      case Op_Code::JSR: {
        this->register_[Register::R7] = this->register_[Register::PC];

        if (instruction.test(11)) {  // pc_offset11 mode, JSR

          // NOLINTNEXTLINE(hicpp-signed-bitwise)
          const auto pc_offset11 = instruction.to_ulong() & 0x7FF;
          const auto sext_pc_offset11 =
            sign_extend(static_cast<tl::u16>(pc_offset11), 11);
          this->register_[Register::PC] += sext_pc_offset11;
        } else {  // baseR mode, JSRR

          // NOLINTNEXTLINE(hicpp-signed-bitwise)
          const auto base_register = (instruction >> 6).to_ulong() & 0x7;
          this->register_[Register::PC] =
            this->register_[static_cast<tl::u16>(base_register)];
        }
        break;
      }
      case Op_Code::AND: {
        const auto dr = destination(instruction);
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto sr1 = (instruction >> 6).to_ulong() & 0x7;

        if (instruction.test(5)) {  // immediate mode
          // bit [4:0] & 0b11111
          // NOLINTNEXTLINE(hicpp-signed-bitwise)
          const auto imm5 = instruction.to_ulong() & 0x1F;
          this->register_[dr] =
            this->register_[sr1] &
            sign_extend(static_cast<tl::u16>(imm5), 5);  // imm5 is 5bits.

        } else {  // register mode

          // NOLINTNEXTLINE(hicpp-signed-bitwise)
          const auto sr2      = instruction.to_ulong() & 0x7;
          this->register_[dr] = this->register_[sr1] & this->register_[sr2];
        }

        // set condition flags
        this->update_flags(static_cast<tl::u16>(dr));

        break;
      }
      case Op_Code::LDR: {
        const auto dr = destination(instruction);

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto base_register = (instruction >> 6).to_ulong() & 0x7;

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto offset6      = instruction.to_ulong() & 0x3F;
        const auto sext_offset6 = sign_extend(static_cast<tl::u16>(offset6), 6);
        this->register_[dr]     = this->read_memory(
          static_cast<tl::u16>(this->register_[base_register] + sext_offset6));
        this->update_flags(dr);
        break;
      }
      case Op_Code::STR: {
        const auto sr = destination(instruction);
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto base_register = (instruction >> 6).to_ulong() & 0x7;
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto offset6      = instruction.to_ulong() & 0x3F;
        const auto sext_offset6 = sign_extend(static_cast<tl::u16>(offset6), 6);

        const auto dest =
          static_cast<tl::u16>(this->register_[base_register] + sext_offset6);
        this->write_memory(dest, this->register_[sr]);
        break;
      }
      case Op_Code::NOT: {
        const auto dr = destination(instruction);

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto sr = (instruction >> 6).to_ulong() & 0x7;

        this->register_[dr] = ~this->register_[(static_cast<tl::u16>(sr))];
        this->update_flags(dr);
        break;
      }
      case Op_Code::LDI: {
        // like LD but the content of the address is another
        // address. Then it loads the content of the address
        // of the address.
        // [addr] -> [addr] -> content
        const auto dr = destination(instruction);

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto pc_offset9 = instruction.to_ulong() & 0x1FF;
        const auto sext_pc_offset9 =
          sign_extend(static_cast<tl::u16>(pc_offset9), 9);

        // location of an address that stores the address of the value to load
        // into DR.
        const auto mem_location =
          tl::u16(this->register_[Register::PC] + sext_pc_offset9);

        // We need to read_memory twice. See comment above.
        this->register_[dr] =
          this->read_memory(this->read_memory(mem_location));

        this->update_flags(static_cast<tl::u16>(dr));
        break;
      }
      case Op_Code::STI: {
        // stores an address of an address that contains an register.
        const auto sr = destination(instruction);

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto pc_offset9 = instruction.to_ulong() & 0x1FF;
        const auto sext_pc_offset9 =
          sign_extend(static_cast<tl::u16>(pc_offset9), 9);

        const auto dest =
          tl::u16(this->register_[Register::PC] + sext_pc_offset9);
        this->write_memory(this->read_memory(dest), this->register_[sr]);
        break;
      }
      case Op_Code::JMP: {
        // handles RET too
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto base_register = (instruction >> 6).to_ulong() & 0x7;
        this->register_[Register::PC] =
          this->register_[static_cast<tl::u16>(base_register)];
        break;
      }
      case Op_Code::LEA: {
        // the address itself is stored in the register.
        // instead of loading the content of the address.
        const auto dr = destination(instruction);

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const auto pc_offset9 = instruction.to_ulong() & 0x1FF;
        const auto sext_pc_offset9 =
          sign_extend(static_cast<tl::u16>(pc_offset9), 9);

        this->register_[dr] = this->register_[Register::PC] + sext_pc_offset9;
        this->update_flags(dr);
        break;
      }
      case Op_Code::TRAP: {
        this->register_[Register::R7] = this->register_[Register::PC];

        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        switch (instruction.to_ulong() & 0xFF) {
          case Trap::getc: {
            // Read a single character from the keyboard. The character is not
            // echoed onto the console. Its ASCII code is copied into R0. The
            // high eight bits of R0 are cleared.
            const auto ch                 = std::getchar();
            this->register_[Register::R0] = static_cast<tl::u16>(ch);
            break;
          }
          case Trap::out: {
            // Write a character in R0[7:0] to the console display.
            const auto content = this->register_[Register::R0];

            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            const auto ch = content & 0x7F;
            fmt::print("{}", static_cast<char>(ch));
            break;
          }
          case Trap::puts: {
            // Write a string of ASCII characters to the console display. The
            // characters are contained in consecutive memory locations, one
            // character per memory location, starting with the address
            // specified in R0. Writing terminates with the occurrence of x0000
            // in a memory location.
            auto start_addr = this->register_[Register::R0];

            // increment the address until we find an address with nothing in it
            while (this->memory_[start_addr]) {
              const auto ch = static_cast<char>(this->read_memory(start_addr));
              fmt::print("{}", ch);
              ++start_addr;
            }
            break;
          }
          case Trap::in: {
            // Print a prompt on the screen and read a single character from the
            // keyboard. The character is echoed onto the console monitor, and
            // its ASCII code is copied into R0. The high eight bits of R0 are
            // cleared.
            fmt::print("Enter a character: ");
            const auto ch = std::getchar();
            fmt::print("\n{}", ch);
            this->register_[Register::R0] = static_cast<tl::u16>(ch);
            break;
          }
          case Trap::putsp: {
            // Write a string of ASCII characters to the console. The characters
            // are contained in consecutive memory locations, two characters per
            // memory location, starting with the address specified in R0. The
            // ASCII code contained in bits [7:0] of a memory location is
            // written to the console first. Then the ASCII code contained in
            // bits [15:8] of that memory location is written to the console. (A
            // character string consisting of an odd number of characters to be
            // written will have x00 in bits [15:8] of the memory location
            // containing the last character to be written.) Writing terminates
            // with the occurrence of x0000 in a memory location.

            auto start_addr = this->register_[Register::R0];

            while (this->memory_[start_addr]) {
              const auto value = this->memory_[start_addr];

              // NOLINTNEXTLINE(hicpp-signed-bitwise)
              const auto first_ch = value & 0xFF;
              fmt::print("{}", static_cast<char>(first_ch));

              // NOLINTNEXTLINE(hicpp-signed-bitwise)
              const auto second_ch = value >> 8;

              if (second_ch) { fmt::print("{}", static_cast<char>(second_ch)); }
              ++start_addr;
            }

            break;
          }
          case Trap::halt: {
            fmt::print("{}\n", "vm halted, bye!");
            this->running_ = false;
            break;
          }
        }
        break;
      }
      // NOLINTNEXTLINE(bugprone-branch-clone)
      case Op_Code::RES:
        // unused
        this->abort();
        break;
      case Op_Code::RTI:
        // unused
        this->abort();
        break;
      default:
        // bad opcode
        this->abort();
        break;
    }
  }
}

auto Virtual_Machine::read_memory(tl::u16 addr) -> tl::u16 {
  // check if the memory is a mapped register
  if (this->memory_[addr] == Mapped_Reg::key_status_reg) {
    if (check_key()) {
      // NOLINTNEXTLINE(hicpp-signed-bitwise)
      this->memory_[Mapped_Reg::key_status_reg] = (1 << 15);
      this->memory_[Mapped_Reg::key_data_reg] =
        static_cast<tl::u16>(std::getchar());
    } else {
      this->memory_[Mapped_Reg::key_status_reg] = 0;
    }
  }
  return this->memory_[addr];
}

auto Virtual_Machine::write_memory(tl::u16 addr, tl::u16 content) -> void {
  this->memory_[addr] = content;
}

auto Virtual_Machine::read_file(const char *file) -> bool {
#ifdef _WIN32
  std::FILE *in = nullptr;
  fopen_s(&in, file, "rb");
#else
  std::FILE *in = std::fopen(file, "rb");
#endif

  auto read{false};

  if (in) {
    // convert to little-endian, since the instructions are big-endian.
    constexpr auto swap16 = [](tl::u16 x) {
      // NOLINTNEXTLINE(hicpp-signed-bitwise)
      return static_cast<tl::u16>((x << 8) | (x >> 8));
    };

    // first value in the file is the starting memory.
    auto origin = tl::u16{};
    fread(&origin, sizeof(origin), 1, in);
    origin = swap16(origin);

    // std::numeric_limits<tl::u16>::max() - origin
    const auto max_read = 52648;

    auto temp_buffer = std::array<tl::u16, max_read>();
    fread(temp_buffer.data(), sizeof(tl::u16), max_read, in);

    auto rng = temp_buffer | std::views::transform(swap16);

    std::ranges::copy_n(
      begin(rng), temp_buffer.size(), begin(this->memory_) + origin);

    read = true;
    std::fclose(in);  // NOLINT
  } else {
    fmt::print(stderr, "{}\n", "cannot open file.");
  }

  return read;
}

auto Virtual_Machine::update_flags(tl::u16 r) noexcept -> void {
  if (this->register_[r] == 0) {
    this->register_[Register::COND] = Condition_Flag::ZRO;

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
  } else if (this->register_[r] >> 15) {
    // 1 in the left-most bit indicates negative
    this->register_[Register::COND] = Condition_Flag::NEG;
  } else {
    this->register_[Register::COND] = Condition_Flag::POS;
  }
}

auto Virtual_Machine::abort() noexcept -> void {
  fmt::print(stderr, "{}\n", "BAD OPCODE. Aborting");
  this->running_ = false;
}
}  // namespace vm
