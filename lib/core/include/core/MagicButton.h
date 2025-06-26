// ------------------------------------------------------------------------
// MagicButton.h
//
// Nov. 2022
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#pragma once

#include <Arduino.h>
#include <memory>
#include <atomic>

#include "DirectIO.h"

// #define DEBUG_BUTTON_STATES

typedef enum Button_e
{
  Open = 0,
  Closed,
  Pressed,
  Clicked,
  Held,
  DoubleClicked,
  ClickedAndHeld,
  Released
} ButtonState;
// ButtonStates you may see in the wild:
//  Open
//  Clicked
//  DoubleClicked
//  ClickedAndHeld
//  Held
//  Pressed (this one's rare--you're lucky if you spot him!)
//     [You'll need to give the user feedback when this state is entered if you're going to use it,
//     [else they're almost guaranteed to blow past it into the HELD state
//
// ButtonStates that are extinct in the wild and exist only in captivity:
//  Closed
//  Released
//

const uint16_t debounceUP(0b0111111111111111); // More leading zeros will increase sensitivity
const int16_t debounceDN(~debounceUP);

class MagicButton
{
protected:
  // Button configuration (values for 1ms timer service calls)
  uint16_t DOUBLECLICKTIME = 150;
  uint16_t PRESSTIME       = 250;
  uint16_t HOLDTIME        = 300;

  int8_t    pin = -1;  // HW pin
  bool      pullup = true; // Enable pullup resistor if active low
  bool      doubleClickable = true;
  uint64_t  dbnceIntvl = 25; // Debounce interval

  std::atomic<ButtonState> state[2] = {ButtonState::Open, ButtonState::Open};
  std::atomic<bool> outputCleared;
  bool buttonDown = false;
  uint64_t debounceTS = 0;
  uint16_t buff = 0; // Moving window to record multiple readings

  virtual bool IRAM_ATTR readPin()
  {
    if (pin < 0)
    {
      return 0;
    }
    return (pullup ^ (bool)directRead_IRAM(pin));
  }

#ifdef DEBUG_BUTTON_STATES
  ButtonState tmpState[2];
#endif

public:
  MagicButton() = default;

  // Constructor
  MagicButton(int8_t pin,
              bool pullup,
              bool doubleClickable,
              bool init_pull = true);

  ~MagicButton() = default;

  void init(int8_t pin,
            bool pullup,
            bool doubleClickable,
            bool init_pull = true);

  void make_toggle(uint16_t time = 20)
  {
    DOUBLECLICKTIME = 1;
    PRESSTIME       = 2;
    HOLDTIME        = time;
  }

  void IRAM_ATTR service();
  ButtonState read();
};
