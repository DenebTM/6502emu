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

    // Cx2 in manual output mode
    if ((val & 0b111000) == 0b110000) {
      (offset == CRA ? ca1 : cb1) = (val & 0b1000) > 3;
    }
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

  // reading ORA and CA2 in output mode
  if (!orb && (*ctrl & 0b100000) == 0b100000) {
    ca2 = 0;
  }

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

  // writing ORB and CB2 in output mode
  if (orb && (*ctrl & 0b100000) == 0b100000) {
    cb2 = 0;
  }

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
  bool *cx1 = &(cb ? cb1 : ca1);

  if ((((*ctrl & 0b10) && (!*cx1 && val)) || // positive transition enabled
       (!(*ctrl & 0b10) && (*cx1 && !val))   // negative transition enabled
       )) {
    *ctrl |= 0x80;

    // IRQ enabled
    if (*ctrl & 0b01) {
      flag_interrupt();
    }

    // Cx2 is in handshake mode -> set high by Cx1 active transition
    if ((*ctrl & 0b111000) == 0b100000) {
      bool *cx2 = &(cb ? cb2 : ca2);
      *cx2 = 1;
    }
  }

  *cx1 = val;
}

/**
 * @param cb false: set CA2; true: set CB2
 */
void Pia::set_cx2(bool cb, bool val) {
  Byte *ctrl = cb ? ctrl_b : ctrl_a;
  // Cx2 is in output mode
  if (*ctrl & 0b100000)
    return;

  bool *cx2 = &(cb ? cb2 : ca2);

  if ((((*ctrl & 0b10000) && (!*cx2 && val)) || // positive transition enabled
       (!(*ctrl & 0b10000) && (*cx2 && !val))   // negative transition enabled
       )) {
    *ctrl |= 0x40;

    // IRQ enabled
    if (*ctrl & 0b01000) {
      flag_interrupt();
    }
  }

  *cx2 = val;
}

void Pia::flag_interrupt() { plugin_callback(CPU_INTERRUPT, (void *)false); }

void Pia::update() {
  static bool ca2_pulse_low = false;
  static bool cb2_pulse_low = false;

  // CA2/CB2 in pulse output mode and currently low -> return to high next clock cycle
  if (ca2 == 0 && (*ctrl_a & 0b111000) == 0b101000) {
    ca2_pulse_low = true;
  }
  if (cb2 == 0 && (*ctrl_b & 0b111000) == 0b101000) {
    cb2_pulse_low = true;
  }

  // CA2/CB2 in pulse output mode and has been low for one clock cycle -> return to high
  if (ca2_pulse_low) {
    ca2_pulse_low = false;
    ca2 = 1;
  }
  if (cb2_pulse_low) {
    cb2_pulse_low = false;
    cb2 = 1;
  }
}
