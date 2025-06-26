// ------------------------------------------------------------------------
// DirectIO.h
// Faster versions of Arduino's digitalWrite() functions for ESP32.
// Quick-and-dirty 'cuz it's register-based.
//
// Aug. 2024
// Ryan "Ratimus" Richardson
//
// Usage
//  Instead of: digitalWrite(pin, HIGH)
//  Use:        directWriteHigh(pin)
//  Instead of: digitalWrite(pin, LOW)
//  Use:        directWriteLow(pin)
// ------------------------------------------------------------------------
#pragma once

#ifndef DIRECT_IO
#define DIRECT_IO 1
#endif

#include <Arduino.h>

#include "soc/gpio_struct.h"
#include "stdint.h"


static inline __attribute__((always_inline))
void directWriteLow(uint32_t pin)
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


static inline __attribute__((always_inline))
void directWriteHigh(uint32_t pin)
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


static inline __attribute__((always_inline))
uint32_t directRead(uint32_t pin)
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

void IRAM_ATTR directWriteLow_IRAM(uint32_t pin);
void IRAM_ATTR directWriteHigh_IRAM(uint32_t pin);
uint32_t IRAM_ATTR directRead_IRAM(uint32_t pin);
