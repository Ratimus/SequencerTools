#include "CD4067.h"
#include <DirectIO.h>


CD4067::CD4067(ESP32AnalogRead *pESP_ADC, uint16_t mask = 0, uint16_t mode):
pESP_ADC(pESP_ADC),
IO_PIN(-1),
pin_mask(mask)
{
  // Pins are active via pin_mask, but we need to add map items for analog
  for (uint8_t n = 0; n < 16; ++n)
  {
    if (mode & (uint16_t)BITMASK_32[n])
    {
      enable_pin(n, true);
    }
  }
}


void CD4067::enable_pin(uint8_t pin, bool is_analog)
{
  if (is_analog)
  {
    assert (pESP_ADC);
    ANALOG_REG[pin] = 0;
  }

  pin_mask |= (uint16_t)BITMASK_32[pin];
}


void CD4067::disable_pin(uint8_t pin)
{
  // Note that this doesn't remove analog pins from the map
  pin_mask &= (uint16_t)~BITMASK_32[pin];
}


void CD4067::read_pin(uint8_t pin)
{
  // Inactive pin
  if (!(pin_mask & (uint16_t)BITMASK_32[pin]))
  {
    return;
  }

  if (pESP_ADC)
  {
    // Digital pin
    if (!(pin_mode & (uint16_t)BITMASK_32[pin]))
    {
      MUXREG ^= (pESP_ADC->readRaw() < 1000);
      return;
    }

    ANALOG_REG[pin] = pESP_ADC->readRaw();
    return;
  }

  MUXREG ^= directRead(IO_PIN);
}


inline uint16_t CD4067::get(uint8_t pin)
{
  uint16_t ret = 0;
  uint16_t access_mask = (uint16_t)BITMASK_32[pin];
  if (!(pin_mask & access_mask))
  {
    return ret;
  }

  cli();
  if (!(pin_mode & access_mask))
  {
    ret = MUXREG & access_mask;
  }
  else
  {
    ret = ANALOG_REG[pin];
  }
  sei();

  return ret;
}


void MultiMux::muxEnable(uint8_t channel, uint8_t delayMicros)
{
  static uint8_t CURRENT_CHANNEL = 0b00001111;
  uint8_t diff = CURRENT_CHANNEL ^ channel;
  for (auto n(0); n < 4; ++n)
  {
    uint8_t mask = (0x01 << n);
    if (diff ^ mask)
    {
      continue;
    }

    if (channel & mask)
    {
      directWriteHigh(ADDR[n]);
    }
    else
    {
      directWriteLow(ADDR[n]);
    }
  }

  CURRENT_CHANNEL = channel;
  delayMicroseconds(delayMicros);
}


// If you only have one mux, you don't need to pass the object index
inline uint16_t MultiMux::get_val(uint8_t pin, uint8_t object_index)
{
  return vMux[object_index].get(pin);
}
