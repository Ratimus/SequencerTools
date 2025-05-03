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
#include <MagicButton.h>
#include <memory>
#include <RatFuncs.h>

// ----------------------------------------------------------------------------

class ClickEncoder
{
public:

  // Constructor
  ClickEncoder(int8_t A,
               int8_t B,
               int8_t BTN,
               uint8_t stepsPerNotch = 4,
               bool usePullResistor  = true);

  // Call every 1 ms in ISR
  virtual void service(void);

  // Get current state and free for further updates
  int16_t      readPosition (void);
  ButtonState  readButton   (void);

protected:

  const    uint8_t  pinA;
  const    uint8_t  pinB;
  const    bool     activeLow;

  volatile int16_t  delta;
  volatile uint16_t acceleration;
  volatile int16_t  position;
  volatile long     lastEncoded;
  volatile long     MSB;
  volatile long     LSB;

  bool              accelerationEnabled;
  uint8_t           steps;

  bool              doubleClickable;

  virtual bool readA();
  virtual bool readB();

  std::unique_ptr<MagicButton> hwButton;

public:

  void onPinChange();
};