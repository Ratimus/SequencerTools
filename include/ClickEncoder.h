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
#include <memory>
#include <RatFuncs.h>

// ----------------------------------------------------------------------------

#define ENC_NORMAL        BIT1   // use Peter Danneger's decoder
#define ENC_FLAKY         BIT2   // use Table-based decoder

// ----------------------------------------------------------------------------

#ifndef ENC_DECODER
#  define ENC_DECODER     ENC_FLAKY
#  ifndef ENC_HALFSTEP
#    define ENC_HALFSTEP  0 // use table for half step per default
#  endif
#else
#  define ENC_DECODER     ENC_NORMAL
#endif


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

  virtual bool readA()
  {
    return ((bool)directRead(pinA) != activeLow);
  }

  virtual bool readB()
  {
    return ((bool)directRead(pinB) != activeLow);
  }

  // Call every 1 ms in ISR
  virtual void service(void);
  void updateButton(void);

  // Get current state and free for further updates
  int16_t      getClickCount  (void);
  ButtonState  readButtonState(void);

  void setDoubleClickEnabled(const bool &d)
  {
    doubleClickEnabled = d;
  }

  const bool getDoubleClickEnabled()
  {
    return doubleClickEnabled;
  }

  const bool getAccelerationEnabled()
  {
    return accelerationEnabled;
  }

  void setAccelerationEnabled(const bool &a);

protected:

  const    uint8_t  pinA;
  const    uint8_t  pinB;
  const    bool     activeLow;

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

  std::shared_ptr<MagicButton> hwButton;
  volatile ButtonState encBtnState;
};


class MuxedEncoder : public ClickEncoder
{
  static inline std::shared_ptr<HW_Mux> _SHARED_MUX = NULL;
  const uint16_t _BITMASK[2];
  uint16_t _REGISTER;

public:

  virtual bool readA() override
  {
    if (_REGISTER & _BITMASK[0])
    {
      return true;
    }

    return false;
  }

  virtual bool readB() override
  {
    if (_REGISTER & _BITMASK[1])
    {
      return true;
    }

    return false;
  }

  MuxedEncoder(const uint8_t * const pinNums,
               uint8_t stepsPerNotch):
      ClickEncoder(-1, -1, -1, stepsPerNotch, true),
      _BITMASK{uint16_t((uint16_t)1 << pinNums[0]), uint16_t((uint16_t)1 << pinNums[1])}
  {
    hwButton = std::make_shared<MuxedButton>(pinNums[2]);
  }

  void init(void)
  {
    if (readA())
    {
      last = 3;
    }

    if (readB())
    {
      last ^= 1;
    };
  }

  static void setMux(HW_Mux *pMux)
  {
    _SHARED_MUX = std::shared_ptr<HW_Mux>(pMux);
  }

  virtual void service() override
  {
    _REGISTER = _SHARED_MUX->getReg();
    ClickEncoder::service();
  }
};

// ----------------------------------------------------------------------------

#endif // __have__ClickEncoder_h__
