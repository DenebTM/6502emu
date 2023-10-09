#include "plugins/6520-pia.hpp"

Pia::Pia(plugin_callback_t plugin_callback) : MemoryMappedDevice(false, 4) {
  this->plugin_callback = plugin_callback;

  *this->ctrl_a = 0x80;
  *this->ctrl_b = 0x80;
}

Byte Pia::read(Word offset) {
  Byte val = mapped_regs[offset];
  if (offset == ORA || offset == ORB) {
    val = read_orx(offset == ORB);
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
  if (offset == ORA || offset == ORB) {
    return write_orx(offset == ORB, val);
  } else if (offset == CRA || offset == CRB) {
    val = (mapped_regs[offset] & 0xc0) | (val & ~0xc0);
  }

  return mapped_regs[offset] = val;
}

/**
 * @param orb false: set PORTA/DDRA; true: set PORTB/DDRB
 */
Byte Pia::read_orx(bool orb) {
  Byte *ctrl = orb ? ctrl_b : ctrl_a;
  Byte *ddr = &(orb ? ddr_b : ddr_a);
  auto &read_port = orb ? read_port_b : read_port_a;

  // CRx bit 2 == 1 -> PORTx selected
  if (*ctrl & 0b100) {
    return read_port();
  }
  // CRx bit 2 == 0 -> DDRx selected
  return *ddr;
}

/**
 * @param orb false: set PORTA/DDRA; true: set PORTB/DDRB
 */
Byte Pia::write_orx(bool orb, Byte val) {
  Byte *ctrl = orb ? ctrl_b : ctrl_a;
  Byte *ddr = &(orb ? ddr_b : ddr_a);
  Byte *port = orb ? port_b : port_a;
  auto &write_port = orb ? write_port_b : write_port_a;

  // CRx bit 2 == 1 -> PORTx selected
  if (*ctrl & 0b100) {
    auto val_keep = val & ~*ddr;
    auto val_out = val & *ddr;

    write_port(val_out);
    return *port = (*port & *ddr) | val_keep;
  }
  // CRx bit 2 == 0 -> DDRx selected
  return *ddr = val;
}

/**
 * @param cb false: set CA1; true: set CB1
 */
void Pia::set_cx1(bool cb, bool val) {
  Byte *ctrl = cb ? ctrl_b : ctrl_a;
  bool *cx = &(cb ? cb1 : ca1);

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
