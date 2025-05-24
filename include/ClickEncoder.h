// ----------------------------------------------------------------------------
// Rotary Encoder Driver with Acceleration
// Supports Click, DoubleClick, Long Click
// Integrates debounced "MagicButton" and interfaces with ClickEncoderInterface
//
// Ryan "Ratimus" Richardson
// Nov. 2022
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
#pragma once

#include <Arduino.h>
#include "MagicButton.h"
#include <memory>
#include "RatFuncs.h"

// ----------------------------------------------------------------------------

class ClickEncoder
{
public:
  // Constructor
  ClickEncoder(int8_t A,
               int8_t B,
               int8_t BTN,
               uint8_t stepsPerNotch = 4,
               bool usePullResistor = true);

  // Call every 1 ms in ISR
  virtual void service(void);

  // Get current state and free for further updates
  int16_t readPosition(void);
  ButtonState readButton(void);

protected:
  const int8_t pinA;
  const int8_t pinB;
  uint8_t steps;

  const bool activeLow;
  bool accelerationEnabled;
  bool doubleClickable;

  volatile uint32_t MSB;
  volatile uint32_t LSB;
  volatile uint32_t lastEncoded;
  volatile int16_t delta;
  volatile int16_t position;
  volatile uint16_t acceleration;

  virtual bool readA();
  virtual bool readB();

  std::unique_ptr<MagicButton> hwButton;

public:
  void onPinChange();
};
