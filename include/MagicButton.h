// ------------------------------------------------------------------------
// MagicButton.h
//
// Nov. 2022
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#pragma once

#include <Arduino.h>
#include <memory>
#include <DirectIO.h>

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

// Button configuration (values for 1ms timer service calls)
//
const uint16_t DOUBLECLICKTIME(150); // Count two clicks as doubleclick if both received within this time
const uint16_t PRESSTIME(250);
const uint16_t HOLDTIME(300); // Report held button after time

class MagicButton
{
protected:

  int8_t    pin;  // HW pin
  bool      pullup; // Enable pullup resistor if active low
  bool      doubleClickable;
  uint64_t  dbnceIntvl; // Debounce interval

  volatile ButtonState state[2];
  volatile bool buttonDown; // Raw data. We don't need to see it.
  volatile bool outputCleared;
  volatile uint64_t debounceTS;
  volatile uint16_t buff; // Moving window to record multiple readings

  virtual bool readPin()
  {
    if (pin < 0)
    {
      return 0;
    }

    cli();
    bool ret = pullup ^ (bool)directRead(pin);
    sei();
    return ret;
  }

#ifdef DEBUG_BUTTON_STATES
  ButtonState tmpState[2];
#endif

public:
  // Constructor
  MagicButton(int8_t pin,
              bool pullup,
              bool doubleClickable) : pin(pin),
                                      pullup(pullup),
                                      doubleClickable(doubleClickable),
                                      dbnceIntvl(25),
                                      state{ButtonState::Open, ButtonState::Open},
                                      buttonDown(0),
                                      outputCleared(1),
                                      debounceTS(0),
                                      buff(0)
  {
    if (pin != -1)
    {
      pinMode(pin, pullup ? INPUT_PULLUP : INPUT);
    }
  }

  void service();
  ButtonState read();
};
