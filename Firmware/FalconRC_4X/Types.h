#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>


enum state {
  RUN,
  CALIBRATED,
  LOW_BAT,
  GOOD_BAT
};

enum rc {
  THROTTLE,
  YAW,
  PITCH,
  ROLL
};

enum type{
  ICT_OFF = 0,
  ICT_ANALOG,
  ICT_ANA_SW,
  ICT_DIGITAL
};

typedef struct {
  uint8_t   ICN;      // pin number
  uint16_t  MIN;      // low value for the calibration
  uint16_t  MID;      // midle value for the calibration 
  uint16_t  MAX;      // high value for the calibration
  uint8_t   DUAL;     // dual rate reduction percentage applied to the end point values, [0, 140]
  int8_t    EXPO;     // exponential percentage applied to the input value, [-100, 100]
  uint8_t   EPL;      // end point low value, [0, 140]
  uint8_t   EPH;      // end point high value, [0, 140] 
  int8_t    SUBTRIM;  // subtrim centering offset, [-100, 100]
  uint8_t   REVERS;   // 1 = reversed, 0 = normal 
} datasetChannel_t;

typedef struct {
  uint8_t  TX_ID[3];
  uint8_t  MIN_VBAT;
  uint8_t  IS_CALIB;      // chack is Calibration
} datasetConfig_t;

typedef struct {
  uint8_t throttle;
  int8_t  yaw;
  int8_t  pitch;
  int8_t  roll;
  uint8_t flags;    // 0xC0 => binding
} packet_t;

#endif /* TYPES_H */
