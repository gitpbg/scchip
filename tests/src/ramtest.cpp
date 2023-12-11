#include <sysc/communication/sc_clock.h>
#include <sysc/communication/sc_signal_ports.h>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_time.h>
#include <sysc/tracing/sc_trace.h>

#include <string>

#include "common.h"
#include "ram.h"

#define STATE_START 0
#define STATE_RESET 1
// #define STATE_RESET_DISABLE 2
#define STATE_WRITE 3
#define STATE_READ_ADDRESS_ONLY 4
#define STATE_READ_DATA_UPDATE_ADDRESS 5
#define STATE_DONE 6

class RAMDriver : public sc_core::sc_module {
 public:
  SC_HAS_PROCESS(RAMDriver);
  RAMDriver(const std::string& wdat, int waddr, int raddr, int rbytes,
            const sc_core::sc_module_name rdname = "ramdriver")
      : sc_core::sc_module(rdname),
        state(STATE_START),
        wdata(wdat),
        rdata(""),
        write_addr(waddr),
        read_addr(raddr),
        read_bytes(rbytes) {
    SC_METHOD(process);
    sensitive << clock.pos();
  }

  ~RAMDriver() {}

  void process() {
    std::cout << name() << " Tick\n";

    if (state == STATE_START) {
      pos = 0;
      addr_start = write_addr;
      state = STATE_RESET;
      state_count = 5;
    } else if (state == STATE_RESET) {
      reset.write(true);
      state_count--;
      if (state_count == 0) {
        reset.write(false);
        state = STATE_WRITE;
        state_count = wdata.length();
        addr_start = write_addr;
        pos = 0;
      }
    } else if (state == STATE_WRITE) {
      write_address.write(addr_start + pos);
      data_out.write(wdata.at(pos));
      write_enable.write(true);
      pos++;
      state_count--;
      if (state_count == 0) {
        state = STATE_READ_ADDRESS_ONLY;
        pos = 0;
        addr_start = read_addr;
      }
    } else if (state == STATE_READ_ADDRESS_ONLY) {
      write_enable.write(false);
//      read_enable.write(true);
//      read_address.write(addr_start + pos);
      state = STATE_READ_DATA_UPDATE_ADDRESS;
    } else if (state == STATE_READ_DATA_UPDATE_ADDRESS) {
      read_enable.write(true);
      read_address.write(addr_start + pos);
      if (read_available->read()) {
        rdata.push_back(data_in->read());
        //read_address.write(addr_start + pos);
      }
      pos++;
      //      std::cout << "rdata len " << rdata.length() << " read_bytes "
      //                << read_bytes << "\n";
      if (rdata.length() >= read_bytes) {
        // std::cout << "STATE DONE\n";
        state = STATE_DONE;
      } else {
      }
    } else if (state == STATE_DONE) {
      std::cout << "Read " << rdata << "\n";
      sc_core::sc_stop();
    }
    // std::cout << "State = " << state << "\n";
  }

  sc_core::sc_out<bool> reset;
  sc_core::sc_in<bool> clock;
  sc_core::sc_out<bool> write_enable;
  sc_core::sc_out<bool> read_enable;
  sc_core::sc_in<bool> read_available;
  sc_core::sc_out<scchip::Address> write_address;
  sc_core::sc_out<scchip::Address> read_address;
  sc_core::sc_out<scchip::Byte> data_out;
  sc_core::sc_in<scchip::Byte> data_in;

  int state;
  int state_count = 0;
  int pos;
  int addr_start;
  const std::string& wdata;
  std::string rdata;
  int write_addr, read_addr, read_bytes;
};

int backdoor_test() {
  scchip::RAM ram(4096);
  std::string data("hello world");
  std::string readback("");
  int rv;
  rv = ram.backdoor_write(10, data);
  // std::cout << rv << " len = " << data.length() << "\n";
  rv = ram.backdoor_read(10, data.length(), readback);
  // std::cout << rv << " data = " << readback << "\n";
  return (data == readback) ? 0 : -1;
}

class TestBench {
 public:
  TestBench(const std::string& wdat) : ram(4096), ramdriver(wdat, 10, 100, 6) {
    ram.clock.bind(clock);
    ram.reset.bind(reset);
    ram.read_enable.bind(read_enable);
    ram.read_available.bind(read_available);
    ram.read_address.bind(read_address);

    ram.write_address.bind(write_address);
    ram.write_enable.bind(write_enable);

    ram.data_in.bind(write_data);
    ram.data_out.bind(read_data);

    ramdriver.clock.bind(clock);
    ramdriver.reset.bind(reset);

    ramdriver.read_enable.bind(read_enable);
    ramdriver.read_available.bind(read_available);
    ramdriver.read_address.bind(read_address);
    ramdriver.data_in.bind(read_data);

    ramdriver.write_enable.bind(write_enable);
    ramdriver.write_address.bind(write_address);
    ramdriver.data_out.bind(write_data);
  }

  ~TestBench() {}

  int run() {
    /*int rv = backdoor_test();
    if (rv != 0) {
      return rv;
    }*/
    int rv = 0;

    sc_core::sc_trace_file* tf = sc_core::sc_create_vcd_trace_file("ramtest");
    sc_core::sc_trace(tf, clock, "clock");
    sc_core::sc_trace(tf, reset, "reset");
    sc_core::sc_trace(tf, write_enable, "we");
    sc_core::sc_trace(tf, write_address, "writeaddr");
    sc_core::sc_trace(tf, read_enable, "re");
    sc_core::sc_trace(tf, read_address, "readaddr");
    sc_core::sc_trace(tf, read_available, "ra");
    sc_core::sc_trace(tf, write_data, "wdata");
    sc_core::sc_trace(tf, read_data, "rdata");
    sc_core::sc_start(0, sc_core::SC_NS);
    while (write_enable.read() == false) {
      sc_core::sc_start(1, sc_core::SC_NS);
    }
    sc_core::sc_start(0, sc_core::SC_NS);
    std::cout << sc_core::sc_time_stamp() << " writes started\n";
    while (write_enable.read() == true) {
      sc_core::sc_start(1, sc_core::SC_NS);
    }
    sc_core::sc_start(0, sc_core::SC_NS);
    std::cout << sc_core::sc_time_stamp() << " writes done\n";
    ram.backdoor_write(100, "Prasad");

    sc_core::sc_start(0, sc_core::SC_NS);
    sc_core::sc_start(30, sc_core::SC_NS);
    sc_core::sc_close_vcd_trace_file(tf);
    return 0;
  }

  scchip::RAM ram;
  RAMDriver ramdriver;
  sc_core::sc_clock clock;
  sc_core::sc_signal<bool> reset;
  sc_core::sc_signal<bool> write_enable;
  sc_core::sc_signal<bool> read_enable;
  sc_core::sc_signal<bool> read_available;
  sc_core::sc_signal<scchip::Address> write_address;
  sc_core::sc_signal<scchip::Address> read_address;
  sc_core::sc_signal<scchip::Byte> write_data;
  sc_core::sc_signal<scchip::Byte> read_data;
};

int sc_main(int argc, char* argv[]) {
  std::string write_data("Gharpure");
  TestBench tb(write_data);
  int rv = tb.run();
  std::string readback;
  tb.ram.backdoor_read(10, write_data.length() + 1, readback);
  std::cout << "Readback = " << readback << "\n";
  return rv;
}
