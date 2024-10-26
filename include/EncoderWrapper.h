// ------------------------------------------------------------------------
// EncoderWrapper.h
//
// Nov. 2022
// Ryan "Ratimus" Richardson
// This file defines a wrapper class to allow communication between a
//  ClickEncoderInterface and an instance of the ARDUINO_MENU_LIBRARY,
//  allowing the encoder to generate RotaryEvents for use with that library
// ------------------------------------------------------------------------
#ifndef EncoderWrapper_H
#define EncoderWrapper_H

#include <Arduino.h>
#include "ClickEncoderInterface.h"
#include <RotaryEvent.h>

#ifdef __rotaryEventIn_h__
namespace Menu
{
class EncoderWrapper : public menuIn
{
private:

  ClickEncoderInterface &EI;
  RotaryEvent &REI;
  encEvnts _evt;

public:

  EncoderWrapper(
    ClickEncoderInterface &EncoderInterface,
    RotaryEvent &Rotary) :
  EI(EncoderInterface),
  REI(Rotary),
  _evt(encEvnts::NUM_ENC_EVNTS)
  { update(); }

  inline void update()
  {
    _evt = EI.getEvent();
  }

  int available(void)
  {
    return peek() != -1;
  }

  int peek(void)
  {
    return _evt;
  }

  int read()
  {
    update();
    return peek();
  }

  void flush()
  {
    EI.flush();
  }

  size_t write(uint8_t v)
  {
    return 1;
  }
};
}//namespace Menu

#endif /* ifdef __rotaryEventIn_h__ */

#endif /* EncoderWrapper_h */
