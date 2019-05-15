#ifndef SERIALPROTOCOL_H
#define SERIALPROTOCOL_H

// Arduino versioning.
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define INBUF_SIZE 32

#define RC_IDENT          10  //out message    Returns Rc version 
#define RC_CONFIG         11  //out message    Returns Dataset Config parameters
#define RC_RAW_CHANNELS   12  //out message    Returns Rc channels pulse width values
#define RC_DSCHANNELS     13  //out message    Returns Dataset Channels
#define RC_RESET_CONF     14  // in message    no param
#define RC_BATTERY        15  //out message    Returns Battery Voltages 
#define RC_CALIBRATION    16  // in message    no param

#define RC_SET_CONFIG     20  //in message     Dataset Config parameters      
#define RC_SET_DSCHANNELS 21  //in message     Set Dataset Channels
#define RC_EEPROM_WRITE   22  //in message     no param


enum SerialState_t {
  IDLE,
  HEADER_START,
  HEADER_M,
  HEADER_ARROW,
  HEADER_SIZE,
  HEADER_CMD
};

class SerialProtocol {
  private:
    uint8_t read08();
    uint16_t read16();
    void serial08(uint8_t a);
    void serial16(uint16_t a);
    void fromSerialToStruct(uint8_t *cb, uint8_t size);
    void fromStructToSerial(uint8_t *cb, uint8_t size);
    void headSerialResponse(uint8_t err, uint8_t size);
    void headSerialReply(uint8_t size);
    void evaluateCommand();

    uint8_t indRX;
    uint8_t inBuf[INBUF_SIZE];
    uint8_t cmdMSP;
    uint8_t offset;
    uint8_t checksum;
    uint8_t dataSize;
    SerialState_t state;
  public:
    void begin(unsigned long baud);
    void CheckCommand();
};

#endif /* SERIALPROTOCOL_H */
