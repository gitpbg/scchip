#pragma once
#include <vector>

#include "common.h"

namespace scchip {
class RAM : public sc_core::sc_module {
 public:
  SC_HAS_PROCESS(RAM);
  RAM(size_t size, const sc_core::sc_module_name name = "ram");
  ~RAM();

  void process();
  void on_reset();
  int backdoor_write(Address addr, const std::string& str);
  int backdoor_read(Address addr, int num, std::string& str);

  sc_core::sc_in<bool> clock;
  sc_core::sc_in<bool> reset;
  sc_core::sc_in<bool> write_enable;
  sc_core::sc_in<bool> read_enable;
  sc_core::sc_out<bool> read_available;
  sc_core::sc_in<Address> write_address;
  sc_core::sc_in<Address> read_address;
  sc_core::sc_in<Byte> data_in;
  sc_core::sc_out<Byte> data_out;

 protected:
  std::vector<Byte> m_data;
};
}  // namespace scchip