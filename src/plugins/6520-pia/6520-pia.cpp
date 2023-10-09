#include "plugins/6520-pia.hpp"

Pia::Pia(plugin_callback_t plugin_callback) : MemoryMappedDevice(false, 4) {
  this->plugin_callback = plugin_callback;

  *this->ctrl_a = 0x80;
  *this->ctrl_b = 0x80;
}

Byte Pia::read(Word offset) {
  Byte val = mapped_regs[offset];
  if (offset == ORA) {
    // CRA bit 2 == 1 -> PORTA selected
    if (*ctrl_a & 0b100) {
      val = read_port_a();
    }
    // CRA bit 2 == 0 -> DDRA selected
    else {
      val = ddr_a;
    }
  } else if (offset == ORB) {
    // CRB bit 2 == 1 -> PORTB selected
    if (*ctrl_b & 0b100) {
      val = read_port_b();
    }
    // CRB bit 2 == 0 -> DDRB selected
    else {
      val = ddr_b;
    }
  }

  // clear bits 6 and 7 of CRA/CRB upon read
  if (offset == ORA || offset == CRA) {
    *ctrl_a &= ~0xc0;
  } else if (offset == ORB || offset == CRB) {
    *ctrl_b &= ~0xc0;
  }

  return val;
}

Byte Pia::write(Word offset, Byte val) {
  if (offset == ORA) {
    // CRA bit 2 == 1 -> IORA selected
    if (*ctrl_a & 0b100) {
      auto val_keep = val & ~ddr_a;
      auto val_out = val & ddr_a;

      write_port_a(val_out);
      return *port_a = (*port_a & ddr_a) | val_keep;
    }
    // CRA bit 2 == 0 -> DDRA selected
    else {
      return ddr_a = val;
    }
  } else if (offset == ORB) {
    // CRB bit 2 == 1 -> IORB selected
    if (*ctrl_b & 0b100) {
      auto val_keep = val & ~ddr_b;
      auto val_out = val & ddr_b;

      write_port_b(val_out);
      return *port_b = (*port_b & ddr_b) | val_keep;
    }
    // CRB bit 2 == 0 -> DDRB selected
    else {
      return ddr_b = val;
    }
  } else if (offset == CRA || offset == CRB) {
    val = (mapped_regs[offset] & 0xc0) | (val & ~0xc0);
  }

  return mapped_regs[offset] = val;
}

/**
 * @param cb false: set CA1; true: set CB1
 */
void Pia::set_cx1(bool cb, bool val) {
  bool *cx = &(cb ? cb1 : ca1);
  Byte *ctrl = cb ? ctrl_b : ctrl_a;

  if ((((*ctrl & 0b10) && (!*cx && val)) || // positive transition enabled
       (!(*ctrl & 0b10) && (*cx && !val))   // negative transition enabled
       )) {
    *ctrl |= 0x80;

    // IRQ enabled
    if (*ctrl & 0b01) {
      flag_interrupt();
    }
  }

  *cx = val;
}

void Pia::flag_interrupt() { plugin_callback(CPU_INTERRUPT, (void *)false); }
