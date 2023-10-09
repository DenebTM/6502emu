#pragma once
#include <functional>
#include <map>
#include <optional>
#include <vector>

#include "mem-dev.hpp"
#include "plugin-callback.hpp"

struct Pia : public MemoryMappedDevice {
  enum PiaRegister {
    PortA,
    CtrlA,
    PortB,
    CtrlB,
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
  std::function<Byte(Byte)> write_port_b = [&](Byte val) { return *port_a = val; };

private:
  Byte *port_a = mapped_regs + PortA;
  Byte *ctrl_a = mapped_regs + CtrlA;
  Byte *port_b = mapped_regs + PortB;
  Byte *ctrl_b = mapped_regs + CtrlB;
};
