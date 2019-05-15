#ifndef TRANSMITTER_H
#define TRANSMITTER_H

// Arduino versioning.
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define EXPO_POINTS 15
    
class Transmitter {
  public:
    void begin();
    void CalibratedStick(int8_t calibState);
    uint8_t ComputeChannel(uint8_t channel);
    uint8_t CheckBattery();
    
  private:
    static const uint8_t s_expoPos[EXPO_POINTS];
    static const uint8_t s_expoNeg[EXPO_POINTS];
    
    int16_t  ReadControl(uint8_t channel);
    uint16_t ReadADC(uint8_t channel);
    int16_t  Expo(int16_t p_value, int8_t expo);
    int16_t  DualRate(int16_t p_value, uint8_t rate);
    int16_t  EndPoint(uint8_t channel, int16_t pValue);
};

extern Transmitter rc;

#endif
