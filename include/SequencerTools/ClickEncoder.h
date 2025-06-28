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
#include <atomic>
#include <memory>

#include <SequencerTools/MagicButton.h>
#include <SequencerTools/RatFuncs.h>

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
  virtual void IRAM_ATTR service(void);

  // Get current state and free for further updates
  int32_t readPosition(void);
  ButtonState readButton(void);

protected:
  const int8_t pinA;
  const int8_t pinB;
  uint8_t steps;

  const bool activeLow;
  bool doubleClickable = true;

  uint32_t MSB;
  uint32_t LSB;
  uint32_t lastEncoded = 0;
  int16_t delta = 0;
  std::atomic<int32_t> position = 0;

  virtual bool IRAM_ATTR readA();
  virtual bool IRAM_ATTR readB();

  std::unique_ptr<MagicButton> hwButton;

public:
  void IRAM_ATTR onPinChange();
};
