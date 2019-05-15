
#include "Types.h"
#include "Config.h"
#include "TxEeprom.h"
#include "Transmitter.h"
#include "SerialProtocol.h"

extern TxEeprom eeprom;
extern Transmitter rc;
extern uint8_t  ch_values[CHANNELS];
extern int8_t   calibState;
extern uint8_t  Status;

#if (CHECK_BAT == 1)
extern uint8_t  VoltBattery;
#endif

void SerialProtocol::begin(unsigned long baud) {
  Serial.begin(baud);
  state = IDLE;
}

uint8_t SerialProtocol::read08() {
  return inBuf[indRX++];
}

uint16_t SerialProtocol::read16() {
  uint16_t  Val_int =  read08();
  Val_int |= (uint16_t)read08() << 8;
  return Val_int;
}

void SerialProtocol::serial08(uint8_t a) {
  checksum ^= a;
  Serial.write(a);
}

void SerialProtocol::serial16(uint16_t a) {
  serial08((a    ) & 0xFF);
  serial08((a >> 8) & 0xFF);
}

void SerialProtocol::fromSerialToStruct(uint8_t *cb, uint8_t size) {
  while (size--) *cb++ = read08();
}

void SerialProtocol::fromStructToSerial(uint8_t *cb, uint8_t size) {
  while (size--) serial08(*cb++);
}

void SerialProtocol::headSerialResponse(uint8_t err, uint8_t size) {
  Serial.write('$');
  Serial.write('M');
  Serial.write(err ? '!' : '>');
  checksum = 0;  // start calculating a new checksum
  serial08(size);
  serial08(cmdMSP);
}

void SerialProtocol::headSerialReply(uint8_t size) {
  headSerialResponse(0, size);
}

void SerialProtocol::SerialProtocol::CheckCommand() {
  int bytesRXBuff = Serial.available();
  uint8_t c;

  while ( bytesRXBuff-- ) {
    c = Serial.read();

    if (state == IDLE) {
      state = (c == '$') ? HEADER_START : IDLE;
    } else if (state == HEADER_START) {
      state = (c == 'M') ? HEADER_M : IDLE;
    } else if (state == HEADER_M) {
      state = (c == '>' || c == '<') ? HEADER_ARROW : IDLE;
    } else if (state == HEADER_ARROW) {
      if (c > INBUF_SIZE) {  // now we are expecting the payload size
        state = IDLE;
        continue;
      }
      dataSize = c;
      offset = 0;
      checksum = 0;
      indRX = 0;
      checksum ^= c;
      state = HEADER_SIZE;  // the command is to follow
    } else if (state == HEADER_SIZE) {
      cmdMSP = c;
      checksum ^= c;
      state = HEADER_CMD;
    } else if (state == HEADER_CMD && offset < dataSize) {
      checksum ^= c;
      inBuf[offset++] = c;
    } else if (state == HEADER_CMD && offset >= dataSize) {
      // compare calculated and transferred checksum
      if (checksum == c) {
        evaluateCommand();  // we got a valid packet, evaluate it
      }
      state = IDLE;
      bytesRXBuff = 0; // no more than one MSP and per cycle
    }
  }
}

void SerialProtocol::evaluateCommand() {
  switch (cmdMSP) {
    case RC_IDENT:
      headSerialReply(1);
      serial08(RC_VERSION);
      break;
    case RC_CONFIG:
      headSerialReply(sizeof(datasetConfig_t));
      fromStructToSerial((uint8_t *)&eeprom.DatasetConfig, sizeof(datasetConfig_t));
      break;
    case RC_RAW_CHANNELS:
      headSerialReply(sizeof(ch_values));
      fromStructToSerial((uint8_t *)&ch_values, sizeof(ch_values));
      break;
    case RC_DSCHANNELS:
      for (uint8_t i = 0; i < CHANNELS; i++) {
        headSerialReply(sizeof(datasetChannel_t) + 1);
        serial08(i);
        fromStructToSerial((uint8_t *)&eeprom.DatasetChannels[i], sizeof(datasetChannel_t));
        Serial.write(checksum);
      }
      break;
    case RC_RESET_CONF:
      headSerialReply(0);
      eeprom.DefaultDataset();
      break;
    case RC_BATTERY:
      headSerialReply(1);
#if (CHECK_BAT == 1)
      serial08(VoltBattery);
#else
      serial08(0);
#endif
      break;
    case RC_CALIBRATION:
      headSerialReply(0);
      Status = read08();
      calibState = read08();
      break;
    case RC_SET_CONFIG:
      headSerialReply(0);
      fromSerialToStruct((uint8_t *)&eeprom.DatasetConfig, sizeof(datasetConfig_t));
      break;
    case RC_SET_DSCHANNELS:
      {
        headSerialReply(0);
        uint8_t index = read08();
        fromSerialToStruct((uint8_t *)&eeprom.DatasetChannels[index].DUAL, 6);
      }
      break;
    case RC_EEPROM_WRITE:
      headSerialReply(0);
      eeprom.UpdateDataset();
      break;
  }
  Serial.write(checksum);
}

