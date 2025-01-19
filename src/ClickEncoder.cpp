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

#include "ClickEncoder.h"
#include <DirectIO.h>

// ----------------------------------------------------------------------------
// Acceleration configuration (for 1000Hz calls to ::service())
//
const uint16_t ENC_ACCEL_TOP (3072);  // max. acceleration: *12 (val >> 8)
const uint8_t  ENC_ACCEL_INC (25);
const uint8_t  ENC_ACCEL_DEC (2);

// ----------------------------------------------------------------------------

#if ENC_DECODER != ENC_NORMAL
#    if ENC_HALFSTEP
        // Decoding table for hardware with flaky notch (half resolution)
        const int8_t ClickEncoder::table[16]  // __attribute__((__progmem__))
        {
          0, 0, -1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, -1, 0, 0
        };
#   else
        // Decoding table for normal hardware
        const int8_t ClickEncoder::table[16]  //  __attribute__((__progmem__))
        {
          0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0
        };
#   endif   /* ENC_HALFSTEP */
#endif      /* ENC_DECODER != ENC_NORMAL */

// ----------------------------------------------------------------------------

ClickEncoder::ClickEncoder(int8_t A,
                           int8_t B,
                           int8_t BTN,
                           uint8_t stepsPerNotch,
                           bool usePulllResistor) :
   doubleClickEnabled(true),
   accelerationEnabled(false),
   delta(0),
   last(0),
   acceleration(0),
   steps(stepsPerNotch),
   pinA(A),
   pinB(B),
   activeLow(usePulllResistor)
{
  if (pinA != -1)
  {
    hwButton = std::make_shared<MagicButton>(BTN, activeLow, doubleClickEnabled);
    uint8_t configType = activeLow ? INPUT_PULLUP : INPUT;
    pinMode(pinA,   configType);
    pinMode(pinB,   configType);

    if ((bool)directRead(pinA) != activeLow)
    {
      last = (BIT0 | BIT1);
    }

    if ((bool)directRead(pinB) != activeLow)
    {
      last ^= BIT0;
    }
  }
}

// ----------------------------------------------------------------------------
// call this every 1 millisecond via timer ISR
//
void ClickEncoder::service(void)
{
  if (accelerationEnabled)      // decelerate every tick
  {
    acceleration -= ENC_ACCEL_DEC;

    if (acceleration & 0x8000)  // handle overflow of MSB is set
    {
      acceleration = 0;
    }
  }

  bool moved = false;

  // cli();
#if ENC_DECODER == ENC_FLAKY
  last = (last << 2) & 0x0F;

  if (!readA())
  {
    last |= BIT1;
  }

  if (!readB())
  {
    last |= BIT0;
  }

  if (table[last])
  {
    delta += table[last];
    moved = true;
  }
#elif ENC_DECODER == ENC_NORMAL
  int8_t curr = readA() ? (BIT0 | BIT1) : 0;
  if (readB())
  {
    curr ^= BIT0;
  }

  int8_t diff = last - curr;

  if (diff & BIT0)             // bit 0 = step
  {
    last   = curr;
    delta += (diff & BIT1) - 1; // bit 1 = direction (+/-)
    moved  = true;
  }
#else
# error "Error: define ENC_DECODER to ENC_NORMAL or ENC_FLAKY"
#endif // ENC_DECODER == ENC_FLAKY
  // sei();

  if (accelerationEnabled && moved)
  {
    // increment accelerator if encoder has been moved
    if (acceleration <= (ENC_ACCEL_TOP - ENC_ACCEL_INC))
    {
      acceleration += ENC_ACCEL_INC;
    }
  }
  hwButton->service();
}


  // Update button state. If our button variable is free,
  // copy the button's state into it and free the hw button
  // for further updates. Don't free *our* encBtnState variable
  // until someone else reads *us*
void ClickEncoder::updateButton()
{
  hwButton->service();
  if (btnStateCleared)
  {
    ButtonState tmp = hwButton->read();
    encBtnState     = tmp;
    btnStateCleared = false;
  }
}
// ----------------------------------------------------------------------------

int16_t ClickEncoder::getClickCount(void)
{
  updateButton();
  // cli();
  int16_t val = delta;

  if (steps == 2)
  {
    delta = val & BIT0;
  }
  else if (steps == 4)
  {
    delta = val & (BIT0 | BIT1);
  }
  else
  {
    delta = 0; // default to 1 step per notch
  }
  // sei();

  if (steps == 4)
  {
    val >>= 2;
  }
  else if (steps == 2)
  {
    val >>= 1;
  }

  int16_t r = 0;
  int16_t accel = ((accelerationEnabled) ? (acceleration >> 8) : 0);

  if (val < 0)
  {
    r -= 1 + accel;
  }
  else if (val > 0)
  {
    r += 1 + accel;
  }

  return r;
}

// ----------------------------------------------------------------------------
// Resets buttonState and returns value prior to reset; encBtnState output state
// persists until this function is called && encBtnState has been released
ButtonState ClickEncoder::readButtonState(void)
{
  ButtonState ret = this->encBtnState;
  btnStateCleared = true;
  return ret;
}

void ClickEncoder::setAccelerationEnabled(const bool &a)
{
  accelerationEnabled = a;
  if (accelerationEnabled == false)
  {
    acceleration = 0;
  }
}