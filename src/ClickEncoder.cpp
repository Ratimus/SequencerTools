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

// ----------------------------------------------------------------------------
// Acceleration configuration (for 1000Hz calls to ::service())
//
const uint16_t ENC_ACCEL_TOP (3072);  // max. acceleration: *12 (val >> 8)
const uint8_t  ENC_ACCEL_INC (25);
const uint8_t  ENC_ACCEL_DEC (2);

// ----------------------------------------------------------------------------

#if ENC_DECODER != ENC_NORMAL
#  ifdef ENC_HALFSTEP
     // decoding table for hardware with flaky notch (half resolution)
     const int8_t ClickEncoder::table[16] __attribute__((__progmem__)) = {
       0, 0, -1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, -1, 0, 0
     };
#  else
     // decoding table for normal hardware
     const int8_t ClickEncoder::table[16] __attribute__((__progmem__)) = {
       0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0
     };
#  endif // not def ENC_HALFSTEP
#endif   // ENC_DECODER != ENC_NORMAL

// ----------------------------------------------------------------------------

ClickEncoder::ClickEncoder(
  uint8_t A, uint8_t B, uint8_t BTN, uint8_t stepsPerNotch, bool activeLow) :
   doubleClickEnabled(true),
   accelerationEnabled(true),
   delta(0),
   last(0),
   acceleration(0),
   steps(stepsPerNotch),
   pinA(A),
   pinB(B),
   pullup(activeLow),
   hwButton(BTN, activeLow, doubleClickEnabled)
{
  uint8_t configType = pullup ? INPUT_PULLUP : INPUT;
  pinMode(pinA,   configType);
  pinMode(pinB,   configType);

  if (digitalRead(pinA) != pullup)
  {
    last = 3;
  }

  if (digitalRead(pinB) != pullup)
  {
    last ^=1;
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

#if ENC_DECODER == ENC_FLAKY
  last = (last << 2) & 0x0F;

  if (digitalRead(pinA) == pinsActive) {
    last |= 2;
  }

  if (digitalRead(pinB) == pinsActive) {
    last |= 1;
  }

  uint8_t tbl = pgm_read_byte(&table[last]);
  if (tbl) {
    delta += tbl;
    moved = true;
  }
#elif ENC_DECODER == ENC_NORMAL
  int8_t curr = (digitalRead(pinA) != pullup) ? 3 : 0;

  if (digitalRead(pinB) != pullup)
  {
    curr ^= 1;
  }

  int8_t diff = last - curr;

  if (diff & 1)             // bit 0 = step
  {
    last   = curr;
    delta += (diff & 2) - 1; // bit 1 = direction (+/-)
    moved  = true;
  }
#else
# error "Error: define ENC_DECODER to ENC_NORMAL or ENC_FLAKY"
#endif // ENC_DECODER == ENC_FLAKY

  if (accelerationEnabled && moved)
  {
    // increment accelerator if encoder has been moved
    if (acceleration <= (ENC_ACCEL_TOP - ENC_ACCEL_INC))
    {
      acceleration += ENC_ACCEL_INC;
    }
  }
  hwButton.service();
}


// Update button state. If our button variable is free,
// copy the button's state into it and free the hw button
// for further updates. Don't free *our* encBtnState variable
// until someone else reads *us*
void ClickEncoder :: updateButton()
{
  hwButton.service();
  if (btnStateCleared)
  {
    ButtonState tmp = hwButton.readAndFree();
    encBtnState     = tmp;
    btnStateCleared = false;
  }
}
// ----------------------------------------------------------------------------

int16_t ClickEncoder :: getClicks(void)
{
  int16_t val;

  cli();
  val = delta;

  if (steps == 2) delta = val & 1;
  else if (steps == 4) delta = val & 3;
  else delta = 0; // default to 1 step per notch
  updateButton();
  sei();

  if (steps == 4) val >>= 2;
  if (steps == 2) val >>= 1;

  int16_t r = 0;
  int16_t accel = ((accelerationEnabled) ? (acceleration >> 8) : 0);

  if (val < 0) {
    r -= 1 + accel;
  }
  else if (val > 0) {
    r += 1 + accel;
  }

  return r;
}

// ----------------------------------------------------------------------------
// Resets buttonState and returns value prior to reset; encBtnState output state
// persists until this function is called && encBtnState has been released
ButtonState ClickEncoder :: getButton(void)
{
  ButtonState ret = this->encBtnState;
  btnStateCleared = true;
  return ret;
}