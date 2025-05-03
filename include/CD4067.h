#pragma once
#include <Arduino.h>
#include <DirectIO.h>
#include "ESP32AnalogRead.h"
#include <stdint.h>

// If you don't like STL, rewrite classes as templates that take the number of items as parameters
#include <vector>
#include <map>
#include <memory>


class CD4067
{
  std::unique_ptr<ESP32AnalogRead> pESP_ADC;
  const int8_t IO_PIN;
  uint16_t pin_mask;        // 0=active, 1=inactive
  uint16_t pin_mode;        // 0=analog, 1=digital

public:

  volatile uint16_t MUXREG;
  std::map<uint8_t, volatile uint16_t> ANALOG_REG;

  CD4067(int8_t IO_PIN);
  CD4067(std::unique_ptr<ESP32AnalogRead> pESP_ADC);

  void enable_pin(uint8_t pin, bool is_analog = 0);
  void disable_pin(uint8_t pin);
  void read_pin(uint8_t pin);
  inline uint16_t get_val(uint8_t pin);
};


class MultiMux
{
  uint8_t ADDR[4];

  void muxEnable(uint8_t channel, uint8_t delayMicros = 0);
  std::vector<CD4067> vMux;

public:

  MultiMux(const uint8_t * const ADDR_PINS)
  {
    for (uint8_t n = 0; n < 4; ++n)
    {
      ADDR[n] = ADDR_PINS[n];
    }
  }

  // Adds a CD4067 with a digital-only IO pin
  uint8_t add_digital_mux(int8_t IO_PIN)
  {
    vMux.emplace_back(IO_PIN);
    return vMux.size() - 1;
  }

  // Adds a CD4067 with an analog read-enabled IO pin
  uint8_t add_analog_mux(int8_t IO_PIN)
  {
    auto pmux = std::make_unique<ESP32AnalogRead>(IO_PIN);
    vMux.emplace_back(std::move(pmux));
    return vMux.size() - 1;
  }

  // If you have a pin that's already enabled as an analog, this makes it digital
  void add_digital_pin(int8_t io_pin, uint8_t mux_idx)
  {
    vMux[mux_idx].enable_pin(io_pin);
  }

  // No input validation; this will fail an assertion if you screw it up
  void add_analog_pin(int8_t io_pin, uint8_t mux_idx)
  {
    vMux[mux_idx].enable_pin(io_pin, true);
  }

  void service();

  uint16_t get_val(uint8_t pin, uint8_t object_index = 0);
};

