

#ifndef TXEEPROM_H
#define TXEEPROM_H

// Arduino versioning.
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <avr/io.h> 
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#include "Types.h"
#include "Config.h"

extern const datasetChannel_t DatasetChannelsDefault[CHANNELS] PROGMEM;
extern const datasetConfig_t DatasetConfigDefault PROGMEM;

class TxEeprom {
  private:
    
  public:
    static datasetConfig_t  DatasetConfig;
    static datasetChannel_t DatasetChannels[CHANNELS];
    
    void DefaultDataset();
    void UpdateDataset();
    uint8_t GetDataset();  
};
//extern TxEeprom eeprom;

#endif /* TXEEPROM_H */
