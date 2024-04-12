// ------------------------------------------------------------------------
// MagicButton.h
//
// Nov. 2022
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#ifndef MagicButton_h
#define MagicButton_h

#include <Arduino.h>


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


const uint16_t debounceUP(0b0111111111111111);  // More leading zeros will increase sensitivity
const int16_t  debounceDN(~debounceUP);

// Button configuration (values for 1ms timer service calls)
//
const uint16_t DOUBLECLICKTIME(150);  // Count two clicks as doubleclick if both received within this time
const uint16_t PRESSTIME(250);
const uint16_t HOLDTIME(350);        // Report held button after time

class MagicButton
{
private:

  uint8_t  pin;         // HW pin
  bool     pullup;      // Set TRUE to enable pullup resistor if active low
  uint8_t  dbnceIntvl;  // How long to lockout bounce AFTER press/release
  bool     doubleClickEnabled;

  volatile ButtonState state[2];
  volatile bool buttonDown;  // Raw data. We don't need to see it, we don't want to see it.
  volatile bool outputCleared;
  volatile long debounceTS;
  volatile uint16_t buff;        // Moving window to record multiple readings

public:
  // Constructor
  MagicButton(uint8_t pin, bool pullup = 1, bool doubleClickable = 1):  // Active LOW if pullup == TRUE
    pin(pin),
    pullup(pullup)
  {
    buff       = 0;
    debounceTS = 0;
    buttonDown = 0;
    dbnceIntvl = 25; // ms
    state[0]   = ButtonState::Open;
    state[1]   = state[0];
    doubleClickEnabled = doubleClickable;
    outputCleared = true;

    pinMode(pin, pullup ? INPUT_PULLUP : INPUT);
  }

  void service();
  ButtonState readAndFree();
};


#endif
