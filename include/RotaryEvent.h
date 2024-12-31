/* -*- C++ -*- */
/**************

RotaryEvent.h

March 2020 M. Smit - github@gangkast.nl
Generic Rotary/Button input

purpose:

  Having a generic rotary event-based implementation,
  leaving rotary and button libraries up to the user.

bare-bones usage example:

/// In main.cpp:
  // Create an instance of ClickEncoderInterface for a rotary encoder
  ClickEncoderInterface encoderInterface(NAV_ENC_L_PIN,
                                         NAV_ENC_R_PIN,
                                         NAV_ENC_SW_PIN,
                                         SENSITIVITY,
                                         STEPS_PER_NOTCH,
                                         ACTIVE_LOW);

  // Create an instance of RotaryEvent to interface with the ArduinoMenu library
  RotaryEvent eventSource(
    RotaryEvent::EventType::BUTTON_CLICKED |        // select
    RotaryEvent::EventType::BUTTON_DOUBLE_CLICKED | // back
    RotaryEvent::EventType::BUTTON_LONG_PRESSED |   // also back
    RotaryEvent::EventType::ROTARY_CCW |            // up
    RotaryEvent::EventType::ROTARY_CW               // down
  );

  // Create an instance of EncoderWrapper to bundle the interface and the event object
  EncoderWrapper navEncoder(encoderInterface, eventSource);

  // In menusystem.h, use the *** RotaryEvent *** object as a menu input

  // In an ISR, call navEncoder.service() every 1ms or so. This takes care of the hardware.
  // Before you update your menu in void loop() or wherever, call navEncoder.read()

  Yeah, it's kinda gross and clunky, but it keeps ArduinoMenu Library working

***/
#ifndef __rotaryEventIn_h__
#define __rotaryEventIn_h__
#include "menuDefs.h"

namespace Menu
{
template<class T> inline T  operator~  (T a)       { return (T)~(int)a; }
template<class T> inline T  operator|  (T a, T b)  { return (T)((int)a | (int)b); }
template<class T> inline T  operator&  (T a, T b)  { return (T)((int)a & (int)b); }
template<class T> inline T  operator^  (T a, T b)  { return (T)((int)a ^ (int)b); }
template<class T> inline T& operator|= (T& a, T b) { return (T&)((int&)a |= (int)b); }
template<class T> inline T& operator&= (T& a, T b) { return (T&)((int&)a &= (int)b); }
template<class T> inline T& operator^= (T& a, T b) { return (T&)((int&)a ^= (int)b); }

class RotaryEvent : public menuIn
{
public:
  enum EventType
  {
    ROTARY_NONE               = 0,
    BUTTON_CLICKED            = 1 << 0,
    BUTTON_DOUBLE_CLICKED     = 1 << 1,
    BUTTON_LONG_PRESSED       = 1 << 2,
    ROTARY_CW                 = 1 << 3,
    ROTARY_CCW                = 1 << 4,
  };

  EventType config;
  EventType events;  // we could do a fifo if we miss events

  RotaryEvent(EventType c):
      config(c),
      events(ROTARY_NONE)
  {
    // config for future use. we could raise if
    // we are missing essential stuff
    // and we need to absorb an arg anyway...
  }

  void registerEvent(EventType e)
  {
    events |= e; // add it to the current events
  }

  int peek(void) override { return events; }
  int available(void) override { return peek() != 0; }

  int read() override
  {
    // enterCmd
    if (events & EventType::BUTTON_CLICKED)
    {
      events &= ~EventType::BUTTON_CLICKED;  // remove from events
      return options->navCodes[enterCmd].ch;
    }

    // escCmd
    if (events & (EventType::BUTTON_DOUBLE_CLICKED|EventType::BUTTON_LONG_PRESSED))
    {
      events &= ~(EventType::BUTTON_DOUBLE_CLICKED|EventType::BUTTON_LONG_PRESSED); // remove from events
      return options->navCodes[escCmd].ch;
    }

    // downCmd
    if (events & EventType::ROTARY_CW)
    {
      events &= ~EventType::ROTARY_CW;        // remove from events
      return options->navCodes[upCmd].ch;     // down sends up on menu? bug?
    }

    // upCmd
    if (events & EventType::ROTARY_CCW)
    {
      events &= ~EventType::ROTARY_CCW;       // remove from events
      return options->navCodes[downCmd].ch;   // up sends down on menu? bug?
    }

    return -1;
  }

  void flush() override {}
  size_t write(uint8_t v) override {return 0;}
}; // class

}//namespace Menu

#endif /* ifndef __rotaryEventIn_h__ */