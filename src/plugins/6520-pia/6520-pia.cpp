#include "plugins/6520-pia.hpp"

Pia::Pia(plugin_callback_t plugin_callback) : MemoryMappedDevice(false, 4) {
  this->plugin_callback = plugin_callback;

  /**
   * initial configuration:
   * - CA1/CA2/CB1/CB2 in input mode, negative edge, IRQ disabled
   * - all pins of Port A and B are inputs
   * - DDRA visible on ORA, DDRB visible on ORB
   */
  *this->ctrl_a = 0;
  *this->ctrl_b = 0;
  this->ddr_a = 0;
  this->ddr_b = 0;
  this->ca1 = 1;
  this->cb1 = 1;
  this->ca2 = 1;
  this->cb2 = 1;
}

Byte Pia::read(Word offset) {
  Byte val = mapped_regs[offset];
  if (offset == ORA || offset == ORB) {
    val = read_orx(offset == ORB);
  }

  // clear Cx1/Cx2 IRQ flags upon read
  if (offset == ORA || offset == CRA) {
    *ctrl_a &= ~(Cx1_IRQ_FLAG | Cx2_IN_IRQ_FLAG);
  } else if (offset == ORB || offset == CRB) {
    *ctrl_b &= ~(Cx1_IRQ_FLAG | Cx2_IN_IRQ_FLAG);
  }

  return val;
}

Byte Pia::write(Word offset, Byte val) {
  if (offset == ORA || offset == ORB) {
    return write_orx(offset == ORB, val);
  } else if (offset == CRA || offset == CRB) {
    // preserve Cx1/Cx2 input IRQ flags, affect only control registers
    val = (val & (Cx1_CTRL | Cx2_CTRL | ORx_SEL_PORT)) | (mapped_regs[offset] & (Cx1_IRQ_FLAG | Cx2_IN_IRQ_FLAG));

    // Cx2 in manual output mode -> value determined by CRx bit 3
    if ((val & (Cx2_MODE_OUTPUT | Cx2_OUT_MANUAL)) == (Cx2_MODE_OUTPUT | Cx2_OUT_MANUAL)) {
      bool *cx2 = &((offset == CRA) ? ca2 : cb2);
      *cx2 = (val & Cx2_OUT_HIGH) > 0;
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

  // reading ORA and CA2 in handshake output mode
  if (!orb && (*ctrl & (Cx2_MODE_OUTPUT | Cx2_OUT_MANUAL)) == Cx2_MODE_OUTPUT) {
    ca2 = 0;
  }

  // CRx bit 2 == 1 -> PORTx selected
  if (*ctrl & ORx_SEL_PORT) {
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
  auto &on_write_port = orb ? on_write_port_b : on_write_port_a;

  // writing ORB and CB2 in handshake output mode
  if (orb && (*ctrl & (Cx2_MODE_OUTPUT | Cx2_OUT_MANUAL)) == Cx2_MODE_OUTPUT) {
    cb2 = 0;
  }

  // CRx bit 2 == 1 -> PORTx selected
  if (*ctrl & ORx_SEL_PORT) {
    auto val_out = val & *ddr;
    on_write_port(val_out);
    return *port = (*port & ~*ddr) | val_out;
  }
  // CRx bit 2 == 0 -> DDRx selected
  return *ddr = val;
}

/**
 * @param cb false -> set CA1; true -> set CB1
 */
void Pia::set_cx1(bool cb, bool val) {
  Byte *ctrl = cb ? ctrl_b : ctrl_a;
  bool *cx1 = &(cb ? cb1 : ca1);

  if ((((*ctrl & Cx1_POS_EDGE) && (!*cx1 && val)) || // positive transition enabled
       (!(*ctrl & Cx1_POS_EDGE) && (*cx1 && !val))   // negative transition enabled
       )) {
    *ctrl |= Cx1_IRQ_FLAG;

    // IRQ enabled
    if (*ctrl & Cx1_IRQ_EN) {
      flag_interrupt();
    }

    // Cx2 is in handshake mode -> set high by Cx1 active transition
    if ((*ctrl & Cx2_CTRL) == Cx2_MODE_OUTPUT) {
      bool *cx2 = &(cb ? cb2 : ca2);
      *cx2 = 1;
    }
  }

  *cx1 = val;
}

/**
 * @param cb false -> set CA2; true -> set CB2
 */
void Pia::set_cx2(bool cb, bool val) {
  Byte *ctrl = cb ? ctrl_b : ctrl_a;
  // Cx2 is in output mode, ignore attempt to set it externally
  if (*ctrl & Cx2_MODE_OUTPUT)
    return;

  bool *cx2 = &(cb ? cb2 : ca2);

  if ((((*ctrl & Cx2_IN_POS_EDGE) && (!*cx2 && val)) || // positive transition enabled
       (!(*ctrl & Cx2_IN_POS_EDGE) && (*cx2 && !val))   // negative transition enabled
       )) {
    *ctrl |= Cx2_IN_IRQ_FLAG;

    // IRQ enabled
    if (*ctrl & Cx2_IN_IRQ_EN) {
      flag_interrupt();
    }
  }

  *cx2 = val;
}

void Pia::flag_interrupt() { plugin_callback(CPU_INTERRUPT, (void *)false); }

void Pia::update() {
  static bool ca2_pulse_low = false;
  static bool cb2_pulse_low = false;

  // CA2 in pulse output mode and currently low -> return to high next clock cycle
  if (ca2 == 0 && (*ctrl_a & Cx2_CTRL) == (Cx2_MODE_OUTPUT | Cx2_OUT_PULSE)) {
    ca2_pulse_low = true;
  } else if (ca2_pulse_low) {
    ca2_pulse_low = false;
    ca2 = 1;
  }

  // CB2 in pulse output mode and currently low -> return to high next clock cycle
  if (cb2 == 0 && (*ctrl_b & Cx2_CTRL) == (Cx2_MODE_OUTPUT | Cx2_OUT_PULSE)) {
    cb2_pulse_low = true;
  } else if (cb2_pulse_low) {
    cb2_pulse_low = false;
    cb2 = 1;
  }
}
