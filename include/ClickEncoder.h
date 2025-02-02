// ----------------------------------------------------------------------------
// Rotary Encoder Driver with Acceleration
// Supports Click, DoubleClick, Long Click
// Integrates debounced "MagicButton" and interfaces with ClickEncoderInterface
//
// Ryan "Ratimus" Richardson
// Nov. 2022
// ----------------------------------------------------------------------------

#ifndef CLICK_ENCODER_DOT_AITCH
#define CLICK_ENCODER_DOT_AITCH

// ----------------------------------------------------------------------------

#include <Arduino.h>
#include <MagicButton.h>
#include <memory>
#include <RatFuncs.h>


#define MUTEX_TIMEOUT 25
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

  SemaphoreHandle_t encoderMutex;

  // Call every 1 ms in ISR
  virtual void service(void);

  // Get current state and free for further updates
  int16_t      readPosition (void);
  ButtonState  readButton   (void);

  bool lock(void);
  void unlock(void);

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

  std::shared_ptr<MagicButton> hwButton;

public:

  void onPinChange();
};


#endif // __have__ClickEncoder_h__
