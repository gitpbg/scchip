#include "ram.h"

#include <sysc/datatypes/fx/sc_fxdefs.h>
#include <sysc/kernel/sc_simcontext.h>

#include "common.h"

namespace scchip {

RAM::RAM(size_t size, const sc_core::sc_module_name ram_name)
    : sc_core::sc_module(ram_name), m_data(size, 0) {
  SC_METHOD(process);
  sensitive << clock.pos();
};

RAM::~RAM() {}

void RAM::on_reset() {
  data_out.write(0);
  std::fill(m_data.begin(), m_data.end(), 0);
}

void RAM::process() {
  std::cout << name() << " Tick\n";
  SC_ASSERT_((read_enable.read() && write_enable.read()),
             "Both Read and Write cannot be enabled");
  if (reset.read()) {
    on_reset();
    return;
  }
  if (write_enable.read()) {
    auto addr = write_address.read();
    auto data = data_in.read();
    // std::cout << sc_core::sc_time_stamp() << " Read " << (char)data << "\n";
    m_data.at(addr) = data;
  }
  if (read_enable.read()) {
    auto addr = read_address.read();
    auto data = m_data.at(addr);
    std::cout << sc_core::sc_time_stamp()
              << " read enabled data = " << (int)data << " @ " << addr << "\n";
    data_out.write(data);
    read_available.write(true);
  }
}

int RAM::backdoor_write(Address addr, const std::string& str) {
  int start = addr;
  int num = str.length();
  int end = start + num;
  if (end >= m_data.size()) {
    end = m_data.size() - 1;
  }
  int ct = 0;
  while (start < end) {
    m_data.at(start) = str.at(ct);
    start++;
    ct++;
  }
  return ct;
}

int RAM::backdoor_read(Address addr, int num, std::string& str) {
  int start = addr;
  int end = start + num;
  if (end >= m_data.size()) {
    end = m_data.size() - 1;
  }
  while (start < end) {
    str.push_back(m_data.at(start));
    start++;
  }
  return str.length();
}
}  // namespace scchip