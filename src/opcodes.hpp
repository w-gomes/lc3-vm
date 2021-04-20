#pragma once

namespace vm {
enum my_enum {
  Apple,
};

enum Register {
  R0 = 0,
  R1,
  R2,
  R3,
  R4,
  R5,
  R6,
  R7,
  PC,    // program counter
  COND,  // condition flags
};

enum Op_Code {
  BR = 0,  // branch
  ADD,     // add
  LD,      // load
  ST,      // store
  JSR,     // jump register
  AND,     // bitwise and &
  LDR,     // load register
  STR,     // store register
  RTI,     // unused
  NOT,     // bitwise not
  LDI,     // load indirect
  STI,     // store indirect
  JMP,     // jump
  RES,     // reserved (unused)
  LEA,     // load effective address
  TRAP,    // execute trap
};

enum Condition_Flag {
  POS = 1 << 0,  // Positive
  ZRO = 1 << 1,  // Zero
  NEG = 1 << 2,  // Negative
};

// sort of Operating system API for the lc3.
enum Trap {
  getc  = 0x20,  // get character from keyboard, not echoed to console
  out   = 0x21,  // output character
  puts  = 0x22,  // output word string
  in    = 0x23,  // get character from keyboard, echoed to console
  putsp = 0x24,  // output a byte string
  halt  = 0x25,  // halt the program
};

enum Mapped_Reg {
  key_status_reg = 0xFE00,  // keyboard status, if a key is pressed
  key_data_reg   = 0xFE02,  // keyboard data, the pressed key
};

enum Mask {
  Three_Bits   = 0x7,
  Five_Bits    = 0x1F,
  Six_Bits     = 0x3F,
  Eight_Bits   = 0xFF,
  Nine_Bits    = 0x1FF,
  Sixteen_Bits = 0xFFFF,
};
}  // namespace vm
