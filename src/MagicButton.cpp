// ------------------------------------------------------------------------
// MagicButton.cpp
//
// Nov. 2022
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#include "MagicButton.h"

// Read, debounce, and set output state. Once set, final output state will
// persist until reported and reset by separate call to readAndFree()
void MagicButton :: service()
{
  if (pin < 0) return;

  buff = (buff << 1) | (pullup ^ digitalRead(pin));

  cli();
  if (millis() - debounceTS >= dbnceIntvl)
  {
    if (!buttonDown)
    {
      if ((buff & debounceUP) == debounceUP)
      {
        buttonDown = 1;
        debounceTS = millis();
      }
    }
    else
    {
      if ((buff | debounceDN) == debounceDN)
      {
        buttonDown = 0;
        debounceTS = millis();
      }
    }
  }
  sei();

  cli();
  long timeSinceChange(millis() - debounceTS);
  switch(state[0])
  {
    // Register initial button state change
    case ButtonState::Open:          // Not pressed, and output has been read
      state[1] = ButtonState::Open;  // Reset output state
      if (buttonDown)
      {
        state[0] = ButtonState::Closed;
      }
      break;

    // Register single click if button went from closed to open and we
    // didn't get a second click within DOUBLECLICKTIME
    // Register a long press if button stays closed for PRESSTIME
    case ButtonState::Closed:
      if (!buttonDown)
      {
        if (!doubleClickEnabled)
        {
          state[1] = ButtonState::Clicked;
          state[0] = ButtonState::Released;
          outputCleared = false;
        }
        else
        {
          state[0] = ButtonState::Clicked;
        }
      }
      else if (timeSinceChange >= PRESSTIME)
      {
        state[0] = ButtonState::Pressed;
      }
      break;

    case ButtonState::Clicked:
      if (buttonDown)
      {
        if (timeSinceChange < DOUBLECLICKTIME)
        {
          state[0] = ButtonState::DoubleClicked;
        }
      }
      else if (timeSinceChange >= DOUBLECLICKTIME)
      {
        state[1] = ButtonState::Clicked;
        state[0] = ButtonState::Released;
        outputCleared = false;
      }
      break;

    case ButtonState::DoubleClicked:
      if (timeSinceChange >= DOUBLECLICKTIME)
      {
        if (buttonDown)
        {
          state[1] = ButtonState::ClickedAndHeld;
          state[0] = ButtonState::ClickedAndHeld;
          outputCleared = false;
        }
        else if (!buttonDown)
        {
          state[1] = ButtonState::DoubleClicked;
          state[0] = ButtonState::Released;
          outputCleared = false;
        }
      }
      break;

    case ButtonState::Pressed:
      if (!buttonDown)
      {
          state[1] = ButtonState::Pressed;
          state[0] = ButtonState::Released;
          outputCleared = false;
      }
      else if (timeSinceChange >= HOLDTIME - PRESSTIME)
      {
        state[1] = ButtonState::Held;
        state[0] = ButtonState::Held;
        outputCleared = false;
      }
      break;

    case ButtonState::ClickedAndHeld:
    case ButtonState::Held:
      if (!buttonDown)
      {
        state[0] = ButtonState::Released;
      }
      break;

    case ButtonState::Released:
      if (outputCleared)
      {
        state[0] = ButtonState::Open;
        state[1] = ButtonState::Open;
      }
      // State persists until external read and clear
      break;

    default:
      break;
  }
  sei();
  //////////////////////////////////////////
  // SERIAL DEBUGGING
  /*  //<---------there's the asterisk!!!
  if (state[1] != tmpState[1])
  {
    Serial.print("button state: ");
    switch(state[1])
    {
      case ButtonState::ClickedAndHeld:
        Serial.println("CLICK CLIIIIIIIIIIII...");
        break;

      case ButtonState::Clicked:
        Serial.println("CLICK");
        break;

      case ButtonState::Closed:
        Serial.println("CLOSED");
        break;

      case ButtonState::DoubleClicked:
        Serial.println("CLICK CLICK");
        break;

      case ButtonState::Held:
        Serial.println("HELD");
        break;

      case ButtonState::Open:
        Serial.println("OPEN");
        break;

      case ButtonState::Pressed:
        Serial.println("PRESSED");
        break;

      case ButtonState::Released:
        Serial.println("RELEASED");
        break;

      default:
        break;
     }
  }
  */ //<---------there's the other one!
  //////////////////////////////////////////
};

// Report current state and free to record further clicks.
// State only resets if button has been released, else
// HELD or PRESSED will be returned on each call
ButtonState MagicButton :: readAndFree(void)
{
  ButtonState retVal;
  cli();
  if (state[0] == ButtonState::Released)
  {
    outputCleared = true;
  }
  retVal = state[1];
  service();
  sei();
  return retVal;
}

