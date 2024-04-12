// ------------------------------------------------------------------------
// ClickEncoderInterface.h
//
// Nov. 2022
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#ifndef ClickEncoderInterface_h
#define ClickEncoderInterface_h

#include <Arduino.h>
#include "ClickEncoder.h"
#include <MagicButton.h>

enum encEvnts
{
  Click,
  DblClick,
  Left,
  ShiftLeft,
  Right,
  ShiftRight,
  Press,
  ClickHold,
  Hold,
  NUM_ENC_EVNTS
};


class ClickEncoderInterface
{
private:

  ClickEncoder &enc;       // Associated hardware clickEncoder
  ButtonState  btnState;   // Variable to store the state of the button
  int8_t       sensivity;

  int oldPos;
  int pos;

  inline void update()
  {
    pos     += enc.getClicks();
    btnState = enc.getButton();
  }

public:

  // Constructor
  ClickEncoderInterface(ClickEncoder &rEnc, int sense);

  encEvnts getEvent(void);

  inline void setSensivity(int s) { sensivity = s; }

  void flush()
  {
    btnState = ButtonState::Open;
    enc.getClicks();
    enc.getButton();
    oldPos = pos;
  }
};

#endif
