#pragma once
#include <stdint.h>

auto check_key() -> uint16_t;

auto disable_input_buffering() -> void;

auto restore_input_buffering() -> void;

auto handle_interrupt(int signal) -> void;
