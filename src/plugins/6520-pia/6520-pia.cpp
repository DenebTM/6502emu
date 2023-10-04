#include "plugins/6520-pia.hpp"

Pia::Pia(plugin_callback_t plugin_callback)
    : Pia(
          plugin_callback,                                                    //
          [&]() { return *port_a; }, [&](Byte val) { return *port_a = val; }, //
          [&]() { return *port_b; }, [&](Byte val) { return *port_a = val; }) {}

Pia::Pia(plugin_callback_t plugin_callback, //
         std::function<Byte(void)> read_port_a, std::function<Byte(Byte)> write_port_a,
         std::function<Byte(void)> read_port_b, std::function<Byte(Byte)> write_port_b)
    : MemoryMappedDevice(false, 4) {
  this->plugin_callback = plugin_callback;
  this->read_port_a = read_port_a;
  this->write_port_a = write_port_a;
  this->read_port_b = read_port_b;
  this->write_port_b = write_port_b;

  *this->ctrl_a = 0x80;
  *this->ctrl_b = 0x80;
}

Byte Pia::read(Word offset) {
  Byte val = mapped_regs[offset];
  if (mapped_regs + offset == port_a) {
    val = read_port_a();
  } else if (mapped_regs + offset == port_b) {
    val = read_port_b();
  }

  if (*ctrl_a & 0x01) {
    *ctrl_a &= ~0x80;
  }
  if (*ctrl_b & 0x01) {
    *ctrl_b &= ~0x80;
  }

  return val;
}

Byte Pia::write(Word offset, Byte val) {
  Byte *reg = mapped_regs + offset;

  if (reg == port_a) {
    // val = (*reg & 0xf0) | (val & ~0xf0);
    return write_port_a(val);
  } else if (reg == port_b) {
    // val = *reg;
    return write_port_b(val);
  } else if (reg == ctrl_a || reg == ctrl_b) {
    val = (*reg & 0x80) | (val & ~0xc0);
  }

  return *reg = val;
}

void Pia::flag_interrupt() {
  {
    if (*ctrl_b & 0x01) {
      *ctrl_b |= 0x80;
      plugin_callback(CPU_INTERRUPT, (void *)false);
    }
  }
}
