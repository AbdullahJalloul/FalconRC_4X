
#include "TxEeprom.h"

datasetConfig_t  TxEeprom::DatasetConfig;
datasetChannel_t TxEeprom::DatasetChannels[CHANNELS];


const datasetConfig_t DatasetConfigDefault PROGMEM = {
  {0, 0, 0},  // TxId 0
  72,         // min voltBat 7.2V
  0           // chack is Calibration
};

const datasetChannel_t DatasetChannelsDefault[CHANNELS] PROGMEM = {
// ICN, MIN, MID, MAX,  DR, EXP, EPL, EPH, SUB, REV
{    0, 384, 511, 384, 100,   0, 140, 140,   0,   0 },  // channel 1 , Throttle (THROTTLE)  
{    1, 384, 511, 384, 100,  30, 140, 140,   0,   0 },  // channel 2 , Rudder   (YAW)
{    2, 384, 511, 384, 100,  30, 140, 140,   0,   0 },  // channel 3 , Elevator (PITCH)
{    3, 384, 511, 384, 100,  30, 140, 140,   0,   0 }   // channel 4 , Aileron  (ROLL)  
};

void TxEeprom::DefaultDataset() {
  memcpy_P((void *)&DatasetChannels, (PGM_VOID_P *)&DatasetChannelsDefault, sizeof(DatasetChannels));
  memcpy_P((void *)&DatasetConfig, (PGM_VOID_P *)&DatasetConfigDefault, sizeof(DatasetConfig));
}

void TxEeprom::UpdateDataset() {
  uint8_t SaveSREG = SREG;
  cli();
  uint8_t  Checksum = EEPROM_CHECKSUM;
  uint16_t addr = 2;
  uint8_t *dataset = (uint8_t *)&DatasetChannels;
  uint16_t siz = sizeof(datasetChannel_t) * CHANNELS;

  while (siz--) {
    eeprom_write_byte((uint8_t *)addr++, *dataset);
    Checksum ^= *dataset++;
  }

  dataset = (uint8_t *)&DatasetConfig;
  siz = sizeof(datasetConfig_t);

  while (siz--) {
    eeprom_write_byte((uint8_t *)addr++, *dataset);
    Checksum ^= *dataset++;
  }

  eeprom_write_byte((uint8_t *)addr, Checksum);
  SREG = SaveSREG;
}

// return value: false = ok, true = initialized
uint8_t TxEeprom::GetDataset() {
  uint8_t SaveSREG = SREG;
  cli();
  uint8_t ret = RUN;
  uint8_t  Checksum = EEPROM_CHECKSUM;
  uint16_t addr = 2;
  uint8_t *dataset = (uint8_t *)&DatasetChannels;
  uint16_t siz = sizeof(datasetChannel_t) * CHANNELS;

  while (siz--) {
    *dataset = eeprom_read_byte((const uint8_t *)addr++ );
    Checksum ^= *dataset++;
  }

  siz = sizeof(datasetConfig_t);
  dataset = (uint8_t *)&DatasetConfig;

  while (siz--) {
    *dataset = eeprom_read_byte((const uint8_t *)addr++ );
    Checksum ^= *dataset++;
  }

  if (Checksum != eeprom_read_byte((const uint8_t *)addr)) {
    DefaultDataset();
    UpdateDataset();
    ret = CALIBRATED;
  }

  SREG = SaveSREG;
  return ret;
}

