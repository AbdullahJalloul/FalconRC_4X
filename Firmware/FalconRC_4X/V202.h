// Arduino versioning.
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

enum {
  FLAG_CAMERA = 0x01,
  FLAG_VIDEO  = 0x02, 
  FLAG_FLIP   = 0x04,
  FLAG_UNK9   = 0x08,
  FLAG_LED    = 0x10,
  FLAG_UNK10  = 0x20,
  FLAG_BIND   = 0xC0
};

class nRF24;
class V202_TX {
  nRF24& radio;
  uint8_t txid[3];
  uint8_t rf_channels[16];
  bool packet_sent;
  uint8_t rf_ch_num;
public:
  V202_TX(nRF24& radio_) :
    radio(radio_)
  {}
  void setTXId(uint8_t txid_[3]);
  void begin();
  void command(uint8_t throttle, int8_t yaw, int8_t pitch, int8_t roll, uint8_t flags);
};
