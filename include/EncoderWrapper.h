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
      eventSource(Rotary),
      _evt(encEvnts::NUM_ENC_EVNTS)
  {
    _evt = encoderInterface.getEvent();
  }

  int available(void) override
  {
    return (peek() != -1);
  }

  int peek(void) override
  {
    switch(_evt)
    {
      case encEvnts::Click:
      {
        return options->navCodes[enterCmd].ch;
      }

      case encEvnts::DblClick:
      {
        return options->navCodes[escCmd].ch;
      }

      case encEvnts::Right:
      case encEvnts::ShiftRight:
      {
        return options->navCodes[downCmd].ch;
      }

      case encEvnts::Left:
      case encEvnts::ShiftLeft:
      {
        return options->navCodes[upCmd].ch;
      }

      case encEvnts::ClickHold:
      {
        return options->navCodes[escCmd].ch;
      }

      case encEvnts::Hold:
      case encEvnts::Press:
      {
        return options->navCodes[idxCmd].ch;  // Experiment; I don't even know what this does
      }

      case encEvnts::NUM_ENC_EVNTS:
      default:
      {
        return -1;
      }
    }
  }

  int read() override
  {
    _evt = encoderInterface.getEvent();
    const int ret = peek();

    if (ret == options->navCodes[idxCmd].ch)
    {
      eventSource.registerEvent(RotaryEvent::EventType::BUTTON_LONG_PRESSED);
    }

    if (ret == options->navCodes[escCmd].ch)
    {
      eventSource.registerEvent(RotaryEvent::EventType::BUTTON_DOUBLE_CLICKED);
    }

    if (ret == options->navCodes[upCmd].ch)
    {
      eventSource.registerEvent(RotaryEvent::EventType::ROTARY_CCW);
    }

    if (ret == options->navCodes[downCmd].ch)
    {
      eventSource.registerEvent(RotaryEvent::EventType::ROTARY_CW);
    }

    if (ret == options->navCodes[escCmd].ch)
    {
      eventSource.registerEvent(RotaryEvent::EventType::BUTTON_DOUBLE_CLICKED);
    }

    if (ret == options->navCodes[enterCmd].ch)
    {
      eventSource.registerEvent(RotaryEvent::EventType::BUTTON_CLICKED);
    }

    return ret;
  }

  void flush() override
  {
    encoderInterface.flush();
    _evt = encEvnts::NUM_ENC_EVNTS;
  }

  // So much for the Interface Segregation principle...
  size_t write(uint8_t v) override
  {
    return 1;
  }

  void service() { encoderInterface.service(); }
};
}//namespace Menu

#endif /* #ifdef __rotaryEventIn_h__ */

#endif /* EncoderWrapper_h */
