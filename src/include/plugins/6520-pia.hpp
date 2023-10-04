#pragma once
#include <functional>
#include <map>
#include <optional>
#include <vector>

#include "mem-dev.hpp"
#include "plugin-callback.hpp"

struct Pia : public MemoryMappedDevice {
  Pia(plugin_callback_t plugin_callback);
  Pia(plugin_callback_t plugin_callback, //
      std::function<Byte(void)> read_port_a, std::function<Byte(Byte)> write_port_a,
      std::function<Byte(void)> read_port_b, std::function<Byte(Byte)> write_port_b);

  Byte *port_a = mapped_regs + 0;
  Byte *ctrl_a = mapped_regs + 1;
  Byte *port_b = mapped_regs + 2;
  Byte *ctrl_b = mapped_regs + 3;

  int pre_read(Word offset) { return 0; }
  int post_write(Word offset) { return 0; }

  Byte read(Word offset) override;

  Byte write(Word offset, Byte val) override;

  void flag_interrupt();

  plugin_callback_t plugin_callback;

  std::function<Byte(void)> read_port_a;
  std::function<Byte(Byte)> write_port_a;

  std::function<Byte(void)> read_port_b;
  std::function<Byte(Byte)> write_port_b;
};
