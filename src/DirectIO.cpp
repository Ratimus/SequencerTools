#include "DirectIO.h"

void IRAM_ATTR directWriteLow_IRAM(uint32_t pin)
{
  if ( pin < 32 )
  {
    GPIO.out_w1tc       = ((uint32_t)1 << pin);
  }
  else if ( pin < 34 )
  {
    GPIO.out1_w1tc.val  = ((uint32_t)1 << (pin - 32));
  }
}


void IRAM_ATTR directWriteHigh_IRAM(uint32_t pin)
{
  if ( pin < 32 )
  {
    GPIO.out_w1ts       = ((uint32_t)1 << pin);
  }
  else if ( pin < 34 )
  {
    GPIO.out1_w1ts.val  = ((uint32_t)1 << (pin - 32));
  }
}


uint32_t IRAM_ATTR directRead_IRAM(uint32_t pin)
{
  if ( pin < 32 )
  {
    return (GPIO.in >> pin) & 0x1;
  }
  else if ( pin < 40 )
  {
    return (GPIO.in1.val >> (pin - 32)) & 0x1;
  }
  return 0;
}
