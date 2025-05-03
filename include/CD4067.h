#pragma once
#include <Arduino.h>
#include <DirectIO.h>
#include "ESP32AnalogRead.h"

// If you don't like STL, rewrite classes as templates that take the number of items as parameters
#include <vector>
#include <map>
#include <memory>


static const uint8_t GRAY_CODE[16] = {0,1,3,2,6,7,5,4,12,13,15,14,10,11,9,8};
static const uint32_t BITMASK_32[] = {
  1U <<  0, 1U <<  1, 1U <<  2, 1U <<  3, 1U <<  4, 1U <<  5, 1U <<  6, 1U <<  7,
  1U <<  8, 1U <<  9, 1U << 10, 1U << 11, 1U << 12, 1U << 13, 1U << 14, 1U << 15,
  1U << 16, 1U << 17, 1U << 18, 1U << 19, 1U << 20, 1U << 21, 1U << 22, 1U << 23,
  1U << 24, 1U << 25, 1U << 26, 1U << 27, 1U << 28, 1U << 29, 1U << 30, 1U << 31};


class CD4067
{
  ESP32AnalogRead *pESP_ADC;
  const int8_t IO_PIN;
  uint16_t pin_mask;        // 0=active, 1=inactive
  uint16_t pin_mode;        // 0=analog, 1=digital

public:

  volatile uint16_t MUXREG;
  std::map<uint8_t, volatile uint16_t> ANALOG_REG;

  CD4067(int8_t IO_PIN, uint16_t mask = 0):
    pESP_ADC(0),
    IO_PIN(IO_PIN),
    pin_mask(mask),
    pin_mode(0)
  { ; }

  CD4067(ESP32AnalogRead *pESP_ADC, uint16_t mask = 0, uint16_t mode = 0);

  void enable_pin(uint8_t pin, bool is_analog = 0);
  void disable_pin(uint8_t pin);
  void read_pin(uint8_t pin);
  inline uint16_t get(uint8_t pin);
};


class MultiMux
{
  uint8_t ADDR[4];

  void muxEnable(uint8_t channel, uint8_t delayMicros = 0);
  std::vector<CD4067> vMux;

public:

  MultiMux()
  { ; }

  void add_digital_mux(int8_t IO_PIN, uint16_t mask = 0)
  {
    vMux.emplace_back(IO_PIN, mask);
  }

  void add_analog_mux(int8_t IO_PIN, uint16_t mask = 0, uint16_t mode = 0)
  {
    auto pmux = std::make_unique<ESP32AnalogRead>(IO_PIN);
    vMux.emplace_back(std::move(pmux), mask, mode);
  }

  void service()
  {
    cli();
    for (auto n: GRAY_CODE)
    {
      muxEnable(n, 10);
      for (auto &mux: vMux)
      {
        mux.read_pin(n);
      }
    }
    sei();
  }

  inline uint16_t get_val(uint8_t pin, uint8_t object_index = 0);
};
