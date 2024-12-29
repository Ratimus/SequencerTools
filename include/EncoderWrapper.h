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

#ifdef __rotaryEventIn_h__

#include <Arduino.h>
#include "ClickEncoderInterface.h"
#include <menuDefs.h>
#include <RotaryEvent.h>

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
    switch(_evt)
    {
      case encEvnts::Click:
        REI.registerEvent(RotaryEvent::EventType::BUTTON_CLICKED);
        return options->navCodes[enterCmd].ch;

      case encEvnts::DblClick:
        REI.registerEvent(RotaryEvent::EventType::BUTTON_DOUBLE_CLICKED);
        return options->navCodes[escCmd].ch;

      case encEvnts::Right:
      case encEvnts::ShiftRight:
        REI.registerEvent(RotaryEvent::EventType::ROTARY_CW);
        return options->navCodes[downCmd].ch;

      case encEvnts::Left:
      case encEvnts::ShiftLeft:
        REI.registerEvent(RotaryEvent::EventType::ROTARY_CCW);
        return options->navCodes[upCmd].ch;

      case encEvnts::ClickHold:
        REI.registerEvent(RotaryEvent::EventType::BUTTON_DOUBLE_CLICKED);
        return options->navCodes[escCmd].ch;

      case encEvnts::Hold:
      case encEvnts::Press:
        REI.registerEvent(RotaryEvent::EventType::BUTTON_LONG_PRESSED);
        return options->navCodes[idxCmd].ch;  // Experiment; I don't even know what this does

      case encEvnts::NUM_ENC_EVNTS:
      default:
        return -1;
    }
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
