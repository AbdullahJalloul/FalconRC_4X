#include "Types.h"
#include "Config.h"
#include "TxEeprom.h"
#include "Transmitter.h"

extern TxEeprom eeprom;

#define ADC_VREF_TYPE (1<<REFS0) // AVCC with external capacitor at AREF pin

// Pre-calculated expo points based on x^3 and x^(1/3)
const uint8_t Transmitter::s_expoPos[EXPO_POINTS] = { 0, 1, 2, 4, 8, 14, 21, 32, 46, 63, 83, 108, 137, 171, 210};
const uint8_t Transmitter::s_expoNeg[EXPO_POINTS] = { 101, 128, 147, 161, 174, 185, 194, 203, 211, 219, 226, 232, 239, 245, 251};

//***************************************************************
void Transmitter::begin() {
  ADMUX  = ADC_VREF_TYPE;
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS0);
}
//***************************************************************
uint16_t Transmitter::ReadADC(uint8_t channel) {

  ADMUX   = ADC_VREF_TYPE | channel;
  ADCSRA |= (1 << ADSC);

#if (ADC_FILTER == 1)
  static uint16_t tmp_ana[2][CHANNELS];
  static uint16_t anaFilt[CHANNELS];

  anaFilt[channel] = (anaFilt[channel] / 2 + tmp_ana[1][channel]) & 0xFFFE; // gain of 2 on last conversion - clear last bit
  tmp_ana[1][channel] = (tmp_ana[1][channel] + tmp_ana[0][channel]) >> 1;

  while ((ADCSRA & (1 << ADIF)) == 0);
  ADCSRA |= (1 << ADIF);

  uint16_t v = ADCW; 
  tmp_ana[0][channel] = (tmp_ana[0][channel] + v) >> 1;

  return anaFilt[channel]>>1; // [0 .. 2040] -> [0 .. 1020]
#else
  while ((ADCSRA & (1 << ADIF)) == 0);
  ADCSRA |= (1 << ADIF);
  return ADCW;
#endif
}

//***************************************************************
void Transmitter::CalibratedStick(int8_t calibState) {

  static int16_t loVals[4], midVals[4], hiVals[4];

  for (uint8_t i = 0; i < 4; i++) { // get low and high vals for sticks.
    int16_t a = ReadADC(eeprom.DatasetChannels[i].ICN);
    loVals[i] = min(a, loVals[i]);
    hiVals[i] = max(a, hiVals[i]);
  }

  switch (calibState) {
  case 1:
    for (uint8_t i = 0; i < 4; i++) { // get mid
      int16_t a = ReadADC(eeprom.DatasetChannels[i].ICN);
      midVals[i] = a;
      loVals[i]  =  15000;
      hiVals[i]  = -15000;
    }
    break;

  case 2:
    for (uint8_t i = 0; i < 4; i++) {
      if (abs(loVals[i] - hiVals[i]) > 10) {
        eeprom.DatasetChannels[i].MIN = loVals[i];
        eeprom.DatasetChannels[i].MID = midVals[i];
        eeprom.DatasetChannels[i].MAX = hiVals[i];
      }
    }
    break;
  }
}

//***************************************************************
int16_t Transmitter::ReadControl(uint8_t channel) {
  datasetChannel_t *chan = (datasetChannel_t *)&eeprom.DatasetChannels[channel];

  int16_t raw = ReadADC(chan->ICN);
  // reverse if needed
  //if(chan->REVERS) raw = 1023 - raw;

  uint16_t m_min = chan->MIN;
  uint16_t m_max = chan->MAX;
  uint16_t m_center = chan->MID;

  // early abort
  if (raw <= m_min) return -256;
  if (raw >= m_max) return  256;

  // calculate distance from center and maximum distance from center
  uint16_t out = raw > m_center ?   raw - m_center : m_center -   raw;
  uint16_t max = raw > m_center ? m_max - m_center : m_center - m_min;

  // change the range from [0 - max] to [0 - 256]
  // first bring the value down to below 256, or we'll be getting overflows, we'll compensate later
  int bits = 0;
  while (out >= 256) {
    out >>= 1;
    ++bits;
  }

  out <<= 8;  // multiply by 256
  out /= max; // bring down to new range

  // bring the value back up if we did any truncating before
  while (bits > 0) {
    out <<= 1;
    --bits;
  }

  return (raw < m_center) ? -out : out;
}

//***************************************************************
uint8_t Transmitter::ComputeChannel(uint8_t channel) {

  datasetChannel_t *chan = (datasetChannel_t *)&eeprom.DatasetChannels[channel];

  int16_t retVal = ReadControl(channel);

  // Dual rate and Exponential do not apply to the throttle channel
  if ( channel != THROTTLE) {
    retVal = Expo(retVal, chan->EXPO);      // apply Exponential
    retVal = DualRate(retVal, chan->DUAL);  // apply dual rate
  }

  retVal = EndPoint(channel, retVal);

  retVal = (retVal+255)/2; // we bring it up to [0 - 255]

  return retVal;
}

//***************************************************************

int16_t Transmitter::Expo(int16_t p_value, int8_t expo) {
  if (expo == 0) { // early abort
    return p_value;
  }

  // select the expo array
  const uint8_t* exparr = expo > 0 ? s_expoPos : s_expoNeg;
  if (expo < 0) { // ABS
    expo = -expo;
  }

  // save sign
  int16_t neg = p_value < 0 ? 1 : 0;
  if (neg) {
    p_value = -p_value;
  }

  uint8_t index = p_value >> 4;   // divide by EXPO_POINTS + 1
  uint8_t rem   = p_value & 0x0F; // remainder of divide by EXPO_POINTS + 1

  // linear interpolation on array values
  uint16_t lowval = static_cast<uint16_t>(index == 0 ? 0 : (index > EXPO_POINTS ? 256 : exparr[index - 1]));
  ++index;
  uint16_t highval = static_cast<uint16_t>(index == 0 ? 0 : (index > EXPO_POINTS ? 256 : exparr[index - 1]));

  lowval  = lowval * ((EXPO_POINTS + 1) - rem);
  highval = highval * rem;

  uint16_t expoval = (lowval + highval) >> 4; // divide by EXPO_POINTS + 1

  // get weighted average between linear and expo value
  uint16_t out = ((p_value * (100 - expo)) + (expoval * expo)) / 100;

  return neg ? -out : out;
}

//***************************************************************
int16_t Transmitter::DualRate(int16_t p_value, uint8_t rate) {
  // there's a risk in overflows here, since 256 * 140 > 32K
  // so we do this unsigned..
  uint8_t neg = p_value < 0;
  uint16_t val = static_cast<uint16_t>(neg ? (-p_value) : p_value);
  val = (val * rate) / 100;
  return neg ? -static_cast<int16_t>(val) : static_cast<int16_t>(val);
}

//***************************************************************
int16_t Transmitter::EndPoint(uint8_t channel, int16_t pValue) {
  datasetChannel_t *chan = (datasetChannel_t *)&eeprom.DatasetChannels[channel];

  // apply subtrim
  pValue += chan->SUBTRIM;

  // apply endpoints
  uint8_t ep = (pValue > 0) ? chan->EPH : chan->EPL;

  bool neg = pValue < 0;
  uint16_t val = static_cast<uint16_t>(neg ? (-pValue) : pValue);
  val = (val * ep) / 140;

  // clamp values
  if (val > 255) val = 255;
  pValue = neg ? -static_cast<int16_t>(val) : static_cast<int16_t>(val);

  // apply channel reverse
  pValue = (chan->REVERS) ? -pValue : pValue;

  return pValue;
}
//***************************************************************

uint8_t Transmitter::CheckBattery() {
  float adcVal = (analogRead(BAT_PIN) * 5.0) / 1024.0;
  uint16_t BatVolt = adcVal / (R2 / (R1 + R2)) * 10;
  return (uint8_t)BatVolt;
}
//***************************************************************


