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


#include <Arduino.h>
#include "ClickEncoderInterface.h"
#include <menuDefs.h>
#include <freertos/semphr.h>

namespace Menu
{
class EncoderWrapper : public menuIn
{
  SemaphoreHandle_t mutex;
  std::list<uint8_t> events;

public:

  size_t write(uint8_t v) override {return 0;}
  ClickEncoderInterface &encoderInterface;

  EncoderWrapper(ClickEncoderInterface &EncoderInterface):
    encoderInterface(EncoderInterface),
    mutex(xSemaphoreCreateRecursiveMutex())
  { ; }

  int peek(void) override
  {
    int ret = encEvnts::None;
    if (xSemaphoreTakeRecursive(mutex, 10) != pdTRUE)
    {
      return ret;
    }

    if (!events.empty())
    {
      ret = events.back();
    }
    xSemaphoreGiveRecursive(mutex);
    return ret;
  }

  int available(void) override
  {
    return peek() != encEvnts::None;
  }

  int read() override
  {
    if (!available())
    {
      return Menu::options->navCodes[noCmd].ch;
    }


    if (xSemaphoreTakeRecursive(mutex, 10)!= pdTRUE)
    {
      return encEvnts::None;
    }

    int ret = peek();
    events.pop_back();
    xSemaphoreGiveRecursive(mutex);
    return ret;
  }

  void flush() override
  {
    while (available())
    {
      xSemaphoreTakeRecursive(mutex, 10);
      events.pop_back();
      xSemaphoreGiveRecursive(mutex);
    }

    encoderInterface.flush();
  }

  void service()
  {
    xSemaphoreTakeRecursive(mutex, 10);
    encoderInterface.service();
    switch(encoderInterface.getEvent())
    {
      case encEvnts::Click:
      {
        events.push_front(Menu::options->navCodes[enterCmd].ch);
        break;
      }
      case encEvnts::Hold:
      case encEvnts::DblClick:
      {
        events.push_front(Menu::options->navCodes[escCmd].ch);
        break;
      }
      case encEvnts::Right:
      case encEvnts::ShiftRight:
      {
        events.push_front(Menu::options->navCodes[upCmd].ch);
        break;
      }
      case encEvnts::Left:
      case encEvnts::ShiftLeft:
      {
        events.push_front(Menu::options->navCodes[downCmd].ch);
        break;
      }
      default:
        break;
    }
    xSemaphoreGiveRecursive(mutex);
  }
};

}//namespace Menu

#endif /* EncoderWrapper_h */
