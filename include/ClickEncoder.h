// ----------------------------------------------------------------------------
// Rotary Encoder Driver with Acceleration
// Supports Click, DoubleClick, Long Click
// Integrates debounced "MagicButton" and interfaces with ClickEncoderInterface
//
// Ryan "Ratimus" Richardson
// Nov. 2022
// ----------------------------------------------------------------------------
// Based on work by:
// (c) 2010 karl@pitrich.com
// (c) 2014 karl@pitrich.com
//
// Timer-based rotary encoder logic by Peter Dannegger
// http://www.mikrocontroller.net/articles/Drehgeber
// ----------------------------------------------------------------------------

#ifndef __have__ClickEncoder_h__
#define __have__ClickEncoder_h__

// ----------------------------------------------------------------------------

#include <Arduino.h>
#include <MagicButton.h>

// ----------------------------------------------------------------------------

#define ENC_NORMAL        (1 << 1)   // use Peter Danneger's decoder
#define ENC_FLAKY         (1 << 2)   // use Table-based decoder

// ----------------------------------------------------------------------------

#ifndef ENC_DECODER
// #  define ENC_DECODER     ENC_FLAKY
#  define ENC_DECODER     ENC_NORMAL
#endif

#if ENC_DECODER == ENC_FLAKY
#  ifndef ENC_HALFSTEP
#    define ENC_HALFSTEP  1        // use table for half step per default
#  endif
#endif

// ----------------------------------------------------------------------------

class ClickEncoder
{
public:

  // Constructor
  ClickEncoder(uint8_t A, uint8_t B, uint8_t BTN,
               uint8_t stepsPerNotch = 1, bool activeLow = true);

  // Call every 1 ms in ISR
  void    service(void);
  void    updateButton(void);

  // Get current state and free for further updates
  int16_t getClicks(void);
  ButtonState  getButton(void);

  void setDoubleClickEnabled(const bool &d) { doubleClickEnabled = d; }
  const bool getDoubleClickEnabled() { return doubleClickEnabled; }
  const bool getAccelerationEnabled() { return accelerationEnabled; }

  void setAccelerationEnabled(const bool &a)
  {
    accelerationEnabled = a;
    if (accelerationEnabled == false)
    {
      acceleration = 0;
    }
  }


private:

  const    uint8_t  pinA;
  const    uint8_t  pinB;
  const    bool     pullup;
  volatile int16_t  delta;
  volatile int16_t  last;
  volatile uint16_t acceleration;

  bool    accelerationEnabled;
  uint8_t steps;
#if ENC_DECODER != ENC_NORMAL
  static const int8_t table[16];
#endif
  bool btnStateCleared;
  bool doubleClickEnabled;

  MagicButton          hwButton;
  volatile ButtonState encBtnState;
};

// ----------------------------------------------------------------------------

#endif // __have__ClickEncoder_h__
