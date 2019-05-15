/************************************************************
 ** Project :FalconRC 4X
 ** V202 protocol 4ch Transmitter Ver:2.3
 ** Author  : Eng Abdullah Jalloul (2015-11-22)
 ** Website : http://horizon4electronics.blogspot.com
 ************************************************************/
#include <SPI.h>
#include "nRF24L01.h"
#include "V202.h"
#include "Types.h"
#include "Config.h"
#include "TxEeprom.h"
#include "Transmitter.h"
#include "SerialProtocol.h"
//***************************************************************

Transmitter rc;
TxEeprom    eeprom;
packet_t    packet;
nRF24       radio(CE_PIN, CSN_PIN);
V202_TX     radioTx(radio);
SerialProtocol SerialCom;

bool     bind = true;
int8_t   direction = 1;
int8_t   calibState = -1;
#if (CHECK_BAT == 1)
uint8_t  VoltBattery, BatState = GOOD_BAT;
#endif
uint8_t  Status;

uint8_t  ch_values[CHANNELS];
uint16_t ledTimer = 0;    // Blink Timer
uint8_t ledHalfPeriod = 0;

uint32_t previousTime;
uint32_t LastTime, LastBatTime;

//***************************************************************
void setup() {
  pinMode(CAL_BTN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  SerialCom.begin(SERIAL_BAUDRATE);
  rc.begin();
  Status = eeprom.GetDataset();

  if (!eeprom.DatasetConfig.IS_CALIB) {
    Status = CALIBRATED;
  }

  radioTx.setTXId(eeprom.DatasetConfig.TX_ID);
  radioTx.begin();

#if (DEBUG > 0)
  Serial.write("Reading status\n");
  uint8_t res = radio.read_register(STATUS);
  Serial.write("Result: ");
  Serial.println(res);
#endif

  LastTime = millis();
  LastBatTime = LastTime;
}
//***************************************************************

void loop() { // LOOP MAIN
  uint32_t currentTime = millis();

  if (currentTime - LastTime >= RC_PERIOD) {
    LastTime = currentTime;
    if (Status == RUN) {
      for (uint8_t n = 0; n < CHANNELS; n++) {
        ch_values[n] = rc.ComputeChannel(n);
      }

      packet.throttle = ch_values[THROTTLE];
      packet.yaw      = ch_values[YAW]   < 0x80 ? 0x7f - ch_values[YAW]   : ch_values[YAW];
      packet.pitch    = ch_values[PITCH] < 0x80 ? 0x7f - ch_values[PITCH] : ch_values[PITCH];
      packet.roll     = ch_values[ROLL]  < 0x80 ? 0x7f - ch_values[ROLL]  : ch_values[ROLL] ;
      packet.flags    = FLAG_LED;

#if (DEBUG == 1)
      Serial.write("sticks: ");
      Serial.print(ch_values[THROTTLE]);
      Serial.write(", ");
      Serial.print(ch_values[YAW]);
      Serial.write(", ");
      Serial.print(ch_values[PITCH]);
      Serial.write(", ");
      Serial.println(ch_values[ROLL]);
#endif

      if (bind) {
        packet.flags = FLAG_BIND;
        if (direction > 0) {
          if (packet.throttle >= 235) {
            direction = -1;
          }
        }
        else {
          if (packet.throttle <= 20) {
            direction = 1;
            bind = false;
          }
        }
      }
      radioTx.command(packet.throttle, packet.yaw, packet.pitch, packet.roll, packet.flags);
    }
    else if (Status == CALIBRATED) {
      rc.CalibratedStick(calibState);
    }

    blink_led();
  }

#if (CHECK_BAT == 1)
  if (currentTime - LastBatTime > BATCHECK_PERIOD) {
    LastBatTime = currentTime;
    VoltBattery = rc.CheckBattery();
    if (VoltBattery <= eeprom.DatasetConfig.MIN_VBAT) {
      BatState = LOW_BAT;
    }
    else {
      BatState = GOOD_BAT;
    }
  }
#endif

  SerialCom.CheckCommand();
  CheckCalibrateButton();

} // END_LOOP_MAIN
//***************************************************************
void CheckCalibrateButton()  {
  static bool Hold = false;
  static int lastReading;
  static long presTime;

  int reading = digitalRead(CAL_BTN);

  // first pressed
  if (reading == LOW && lastReading == HIGH) {
    presTime = millis();
    Hold = false;
  }

  // hold
  if (reading == LOW && lastReading == LOW && Hold == false) {
    if ((millis() - presTime) > 1000) {
      presTime = millis();
      Hold = true;
      Red_LED_TOG;
      ++calibState;
      switch (calibState) {
        case 0:
#if (DEBUG != 0)
          Serial.println(F("START CALIBRATION"));
          Serial.println(F("[BUTTON] TO START"));
#endif
          break;

        case 1:
#if (DEBUG != 0)
          Serial.println(F("SET MIDPOINT"));
#endif
          Status = CALIBRATED;
          break;

        case 2:
#if (DEBUG != 0)
          Serial.println(F("MOVE STICKS"));
#endif
          break;

        case 3:
          Status = RUN;
          calibState = -1;
          eeprom.DatasetConfig.IS_CALIB = 1;
          eeprom.UpdateDataset();
#if (DEBUG != 0)
          Serial.println(F("END CALIBRATION"));
#endif
          break;

      }
    }
  }

  lastReading = reading;
}

//***************************************************************

void blink_led () {
  if (Status == RUN) {
    ledHalfPeriod = 10;
  }
  else if (Status == CALIBRATED) {
    ledHalfPeriod = 50;
  }
#if (CHECK_BAT == 1)
  if (BatState == LOW_BAT) {
    ledHalfPeriod = 0; // Low Bat
    Red_LED_OFF;
    return;
  }
#endif
  if ( ++ledTimer > ledHalfPeriod ) {
    Red_LED_TOG;	// Toggle status LED
    ledTimer = 0;	// Blink led timer reset
  }

}
//***************************************************************









