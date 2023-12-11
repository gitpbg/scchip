#pragma once

#include <stdint.h>

#include <cstdint>
#include <systemc>

namespace scchip {
using Byte = uint8_t;
using Address = sc_dt::sc_uint<12>;
}  // namespace scchip