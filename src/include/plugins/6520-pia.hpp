#pragma once
#include <functional>
#include <map>
#include <optional>
#include <vector>

#include "mem-dev.hpp"
#include "plugin-callback.hpp"

struct Pia : public MemoryMappedDevice {
  enum PiaRegister {
    ORA, // Port B / DDRA
    CRA, // Control register A
    ORB, // Port B / DDRB
    CRB, // Control register B
  };

  Pia(plugin_callback_t plugin_callback);

  int pre_read(Word offset) { return 0; }
  int post_write(Word offset) { return 0; }

  Byte read(Word offset) override;

  Byte write(Word offset, Byte val) override;

  void flag_interrupt();

  plugin_callback_t plugin_callback;

  std::function<Byte(void)> read_port_a = [&]() { return *port_a; };
  std::function<Byte(Byte)> write_port_a = [&](Byte val) { return *port_a = val; };

  std::function<Byte(void)> read_port_b = [&]() { return *port_b; };
  std::function<Byte(Byte)> write_port_b = [&](Byte val) { return *port_b = val; };

  inline void set_ca1(bool val) { set_cx1(false, val); }
  inline void set_cb1(bool val) { set_cx1(true, val); }

  // TODO: ca2, cb2

private:
  Byte *port_a = mapped_regs + ORA;
  Byte *ctrl_a = mapped_regs + CRA;
  Byte *port_b = mapped_regs + ORB;
  Byte *ctrl_b = mapped_regs + CRB;

  Byte ddr_a;
  Byte ddr_b;

  bool ca1 = false;
  bool ca2 = false;
  bool cb1 = false;
  bool cb2 = false;

  Byte read_orx(bool orb);
  Byte write_orx(bool orb, Byte val);
  void set_cx1(bool cb, bool val);
};
