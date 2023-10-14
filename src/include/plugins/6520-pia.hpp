#pragma once
#include <functional>
#include <map>
#include <optional>
#include <vector>

#include "mem-dev.hpp"
#include "plugin-callback.hpp"
#include "plugins/plugin-types.hpp"

#define Cx1_CTRL (BIT0 | BIT1)
#define Cx1_NEG_EDGE 0
#define Cx1_POS_EDGE BIT1
#define Cx1_IRQ_EN BIT0
#define Cx1_IRQ_FLAG BIT7

#define Cx2_CTRL (BIT3 | BIT4 | BIT5)

#define Cx2_MODE_INPUT 0
#define Cx2_IN_IRQ_EN BIT3
#define Cx2_IN_NEG_EDGE 0
#define Cx2_IN_POS_EDGE BIT4
#define Cx2_IN_IRQ_FLAG BIT6

#define Cx2_MODE_OUTPUT BIT5
// Cx2 handshake mode
#define Cx2_OUT_PULSE BIT3
// Cx2 manual output mode
#define Cx2_OUT_MANUAL BIT4
#define Cx2_OUT_LOW 0
#define Cx2_OUT_HIGH BIT3

#define ORx_SEL_DDR 0
#define ORx_SEL_PORT BIT2

class EXPORT Pia : public MemoryMappedDevice {
public:
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

  std::function<Byte(void)> read_port_a = [&] { return *port_a; };
  std::function<void(Byte)> on_write_port_a = [&](Byte val) { return; };

  std::function<Byte(void)> read_port_b = [&] { return *port_b; };
  std::function<void(Byte)> on_write_port_b = [&](Byte val) { return; };

  bool ca1 = 1;
  bool ca2 = 1;
  bool cb1 = 1;
  bool cb2 = 1;

  inline void set_ca1(bool val) { set_cx1(false, val); }
  inline void set_ca2(bool val) { set_cx2(false, val); }
  inline void set_cb1(bool val) { set_cx1(true, val); }
  inline void set_cb2(bool val) { set_cx2(true, val); }

  void update();

private:
  Byte *port_a = mapped_regs + ORA;
  Byte *ctrl_a = mapped_regs + CRA;
  Byte *port_b = mapped_regs + ORB;
  Byte *ctrl_b = mapped_regs + CRB;

  Byte ddr_a;
  Byte ddr_b;

  Byte read_orx(bool orb);
  Byte write_orx(bool orb, Byte val);

  void set_cx1(bool cb, bool val);
  void set_cx2(bool cb, bool val);
};
