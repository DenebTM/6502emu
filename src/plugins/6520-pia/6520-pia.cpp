#include "plugins/6520-pia.hpp"

Pia::Pia(plugin_callback_t plugin_callback) : MemoryMappedDevice(false, 4) {
  this->plugin_callback = plugin_callback;

  *this->ctrl_a = 0x80;
  *this->ctrl_b = 0x80;
}

Byte Pia::read(Word offset) {
  Byte val = mapped_regs[offset];
  if (offset == PortA) {
    val = read_port_a();
  } else if (offset == PortB) {
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
  if (offset == PortA) {
    return write_port_a(val);
  } else if (offset == PortB) {
    return write_port_b(val);
  } else if (offset == CtrlA || offset == CtrlB) {
    val = (mapped_regs[offset] & 0x80) | (val & ~0xc0);
  }

  return mapped_regs[offset] = val;
}

void Pia::flag_interrupt() {
  if (*ctrl_b & 0x01) {
    *ctrl_b |= 0x80;
    plugin_callback(CPU_INTERRUPT, (void *)false);
  }
}
