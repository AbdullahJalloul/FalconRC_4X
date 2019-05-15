#ifndef CONFIG_H
#define CONFIG_H

#define RC_VERSION 230

#define DEBUG      0 // 0-Disable, 1-Channels Valume, 2-Calibration
#define CHECK_BAT  0 // 0-Disable, 1-Enable

#define ADC_FILTER 1 // 0-Disable, 1-Enable

#define SERIAL_BAUDRATE 19200

#if (DEBUG == 1)
  #define RC_PERIOD         20   // 20 ms
#else
  #if (ADC_FILTER == 1)
    #define RC_PERIOD       4    // 4 ms
  #else
    #define RC_PERIOD       5    // 5 ms
  #endif
#endif

#define BATCHECK_PERIOD 5000 // 5 sec

#define EEPROM_CHECKSUM 0x55

#define CHANNELS        4

#define CE_PIN          9
#define CSN_PIN         10

#define CAL_BTN         2
#define LED_PIN         3
#define BAT_PIN         A4

#define Red_LED_ON      PORTD |= (1<<3)
#define Red_LED_OFF     PORTD &=~(1<<3)
#define Red_LED_TOG     PORTD ^= (1<<3)
/*
#define Red_LED_ON      PORTB |= (1<<5)
#define Red_LED_OFF     PORTB &=~(1<<5)
#define Red_LED_TOG     PORTB ^= (1<<5)
*/
#define R1              10000.0 // resistance of R1 (10K)
#define R2              2200.0  // resistance of R2 (2.2K)



#endif /* CONFIG_H */
