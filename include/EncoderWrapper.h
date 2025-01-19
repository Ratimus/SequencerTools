// ------------------------------------------------------------------------
// EncoderWrapper.h
//
// Nov. 2022
// Author: Ratimus
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

  ClickEncoderInterface &encoderInterface;
  RotaryEvent &eventSource;
  encEvnts _evt;

public:

  EncoderWrapper(
    ClickEncoderInterface &EncoderInterface,
    RotaryEvent &Rotary) :
      encoderInterface(EncoderInterface),
      eventSource(Rotary)
  {
    _evt = encoderInterface.getEvent();
  }

  int peek(void) override { return eventSource.peek(); }
  int available(void) override { return peek() != 0; }
  int read() override { return eventSource.read(); }

  void flush() override
  {
    encoderInterface.flush();
    eventSource.flush();
  }

  size_t write(uint8_t v) override
  {
    eventSource.registerEvent(static_cast<RotaryEvent::EventType>(v));
    return 1;
  }

  void service()
  {
    encoderInterface.service();
    _evt = encoderInterface.getEvent();
    switch(_evt)
    {
      case encEvnts::Click:
      {
        eventSource.registerEvent(RotaryEvent::EventType::BUTTON_CLICKED);
        break;
      }
      case encEvnts::DblClick:
      {
        eventSource.registerEvent(RotaryEvent::EventType::BUTTON_DOUBLE_CLICKED);
        break;
      }
      case encEvnts::Hold:
      {
        eventSource.registerEvent(RotaryEvent::EventType::BUTTON_LONG_PRESSED);
        break;
      }
      case encEvnts::Left:
      {
        eventSource.registerEvent(RotaryEvent::EventType::ROTARY_CCW);
        break;
      }
      case encEvnts::Right:
      {
        eventSource.registerEvent(RotaryEvent::EventType::ROTARY_CW);
        break;
      }
      default:
      break;
    }
  }
};
}//namespace Menu

#endif /* #ifdef __rotaryEventIn_h__ */

#endif /* EncoderWrapper_h */
