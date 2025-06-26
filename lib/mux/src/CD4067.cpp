#include <mux/CD4067.h>

static const uint8_t  GRAY_CODE[16] = {0,  1,  3,  2,  6,  7,  5, 4,
                                       12, 13, 15, 14, 10, 11, 9, 8};
static const uint32_t BITMASK_32[]  = {
    1U << 0,  1U << 1,  1U << 2,  1U << 3,  1U << 4,  1U << 5,  1U << 6,
    1U << 7,  1U << 8,  1U << 9,  1U << 10, 1U << 11, 1U << 12, 1U << 13,
    1U << 14, 1U << 15, 1U << 16, 1U << 17, 1U << 18, 1U << 19, 1U << 20,
    1U << 21, 1U << 22, 1U << 23, 1U << 24, 1U << 25, 1U << 26, 1U << 27,
    1U << 28, 1U << 29, 1U << 30, 1U << 31};

CD4067::CD4067(int8_t IO_PIN):
  pESP_ADC(std::unique_ptr<ESP32AnalogRead>(nullptr)),
  IO_PIN(IO_PIN),
  pin_mask(0),
  pin_mode(0)
{
  pinMode(IO_PIN, INPUT);
}

CD4067::CD4067(std::unique_ptr<ESP32AnalogRead> pESP_ADC):
  pESP_ADC(std::move(pESP_ADC)),
  IO_PIN(-1),
  pin_mask(0),
  pin_mode(0)
{ ; }


// NOTE: if a pin was previously enabled as analog, this will make it digital
void CD4067::enable_pin(uint8_t pin, bool is_analog)
{
  if (is_analog)
  {
    assert(pESP_ADC);
    ANALOG_REG[pin] = 0;
    pin_mode |= (uint16_t)BITMASK_32[pin];
  }
  else
  {
    pin_mode &= (uint16_t)~BITMASK_32[pin];
  }

  pin_mask |= (uint16_t)BITMASK_32[pin];
}

void CD4067::disable_pin(uint8_t pin)
{
  pin_mask &= (uint16_t)~BITMASK_32[pin];
  pin_mode &= (uint16_t)~BITMASK_32[pin];
}

void IRAM_ATTR CD4067::read_pin(uint8_t pin)
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
      if (pESP_ADC->readRaw() < 1000)
      {
        MUXREG &= (uint16_t)~BITMASK_32[pin];
        return;
      }

      MUXREG |= (uint16_t)BITMASK_32[pin];
      return;
    }

    ANALOG_REG[pin] = pESP_ADC->readRaw();
    return;
  }

  if (directRead_IRAM(IO_PIN))
  {
    MUXREG |= (uint16_t)BITMASK_32[pin];
    return;
  }

  MUXREG &= (uint16_t)~BITMASK_32[pin];
}

inline uint16_t IRAM_ATTR CD4067::get_val(uint8_t pin)
{
  uint16_t ret         = 0;
  uint16_t access_mask = (uint16_t)BITMASK_32[pin];

  if (!(pin_mask & access_mask))
  {
    return ret;
  }

  if (!(pin_mode & access_mask))
  {
    ret = MUXREG & access_mask;
  }
  else
  {
    ret = ANALOG_REG[pin];
  }

  return ret;
}

void IRAM_ATTR MultiMux::muxEnable(uint8_t channel, uint8_t delayMicros)
{
  static uint8_t CURRENT_CHANNEL = 0b00001111;
  uint8_t        diff            = CURRENT_CHANNEL ^ channel;
  for (auto n(0); n < 4; ++n)
  {
    uint8_t mask = (0x01 << n);
    if ((diff & mask) == 0)
    {
      continue;
    }

    if (channel & mask)
    {
      directWriteHigh_IRAM(ADDR[n]);
    }
    else
    {
      directWriteLow_IRAM(ADDR[n]);
    }
  }

  CURRENT_CHANNEL = channel;
  ets_delay_us(delayMicros);
}

void IRAM_ATTR MultiMux::service()
{
  for (auto n : GRAY_CODE)
  {
    muxEnable(n, 10);
    for (auto &mux : vMux)
    {
      mux.read_pin(n);
    }
  }
}

// If you only have one mux, you don't need to pass the object index
uint16_t IRAM_ATTR MultiMux::get_val(uint8_t pin, uint8_t object_index)
{
  return vMux[object_index].get_val(pin);
}
