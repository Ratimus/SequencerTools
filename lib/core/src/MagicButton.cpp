// ------------------------------------------------------------------------
// MagicButton.cpp
//
// Nov. 2022
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#include "MagicButton.h"
#include "DirectIO.h"

MagicButton::MagicButton(int8_t pin,
                         bool pullup,
                         bool doubleClickable,
                         bool init_pull)
{
  init(pin, pullup, doubleClickable, init_pull);
}

void MagicButton::init(int8_t pin,
                       bool pullup,
                       bool doubleClickable,
                       bool init_pull)
{
  this->pin = pin;
  this->pullup = pullup;
  this->doubleClickable = doubleClickable;
  state[0].store(ButtonState::Open, std::memory_order_release);
  state[1].store(ButtonState::Open, std::memory_order_release);

  if (pin > -1)
  {
    pinMode(pin, (pullup && init_pull) ? INPUT_PULLUP : INPUT);
  }
}


// Read, debounce, and set output state. Once set, final output state will
// persist until reported and reset by separate call to read()
void IRAM_ATTR MagicButton::service()
{
  long long timeStamp = 0;

  // Shift buffer by one and tack the current value on the end
  buff = (buff << 1) | (uint16_t)readPin();

  timeStamp = millis();

  if ((timeStamp >= (long long)debounceTS) &&
      ((timeStamp - (long long)debounceTS) >= (long long)dbnceIntvl))
  {
    if (!buttonDown)
    {
      if ((buff & debounceUP) == debounceUP)
      {
        buttonDown = 1;
        debounceTS = timeStamp;
      }
    }
    else
    {
      if ((buff | debounceDN) == debounceDN)
      {
        buttonDown = 0;
        debounceTS = timeStamp;
      }
    }
  }

  long timeSinceChange = timeStamp - debounceTS;

  switch (state[0].load(std::memory_order_acquire))
  {
  // Register initial button state change
  case ButtonState::Open: // Not pressed, and output has been read
  {
    // Reset output state
    state[1].store(ButtonState::Open, std::memory_order_release);
    if (buttonDown)
    {
      state[0].store(ButtonState::Closed, std::memory_order_release);
    }
    break;
  }

  // Register single click if button went from closed to open and we
  // didn't get a second click within DOUBLECLICKTIME
  // Register a long press if button stays closed for PRESSTIME
  case ButtonState::Closed:
  {
    if (!buttonDown)
    {
      if (!doubleClickable)
      {
        state[1].store(ButtonState::Clicked, std::memory_order_release);
        state[0].store(ButtonState::Released, std::memory_order_release);
        outputCleared.store(false, std::memory_order_release);
      }
      else
      {
        state[0].store(ButtonState::Clicked, std::memory_order_release);
      }
    }
    else if (timeSinceChange >= PRESSTIME)
    {
      state[0].store(ButtonState::Pressed, std::memory_order_release);
    }
    break;
  }

  case ButtonState::Clicked:
  {
    if (buttonDown)
    {
      if (timeSinceChange < DOUBLECLICKTIME)
      {
        state[0].store(ButtonState::DoubleClicked, std::memory_order_release);
      }
    }
    else if (timeSinceChange >= DOUBLECLICKTIME)
    {
      state[1].store(ButtonState::Clicked, std::memory_order_release);
      state[0].store(ButtonState::Released, std::memory_order_release);
      outputCleared.store(false, std::memory_order_release);
    }
    break;
  }

  case ButtonState::DoubleClicked:
  {
    if (timeSinceChange >= DOUBLECLICKTIME)
    {
      if (buttonDown)
      {
        state[1].store(ButtonState::ClickedAndHeld, std::memory_order_release);
        state[0].store(ButtonState::ClickedAndHeld, std::memory_order_release);
        outputCleared.store(false, std::memory_order_release);
      }
      else if (!buttonDown)
      {
        state[1].store(ButtonState::DoubleClicked, std::memory_order_release);
        state[0].store(ButtonState::Released, std::memory_order_release);
        outputCleared.store(false, std::memory_order_release);
      }
    }
    break;
  }

  case ButtonState::Pressed:
  {
    if (!buttonDown)
    {
      state[1].store(ButtonState::Pressed, std::memory_order_release);
      state[0].store(ButtonState::Released, std::memory_order_release);
      outputCleared.store(false, std::memory_order_release);
    }
    else if (timeSinceChange >= HOLDTIME - PRESSTIME)
    {
      state[1].store(ButtonState::Held, std::memory_order_release);
      state[0].store(ButtonState::Held, std::memory_order_release);
      outputCleared.store(false, std::memory_order_release);
    }
    break;
  }

  case ButtonState::ClickedAndHeld:
  case ButtonState::Held:
  {
    if (!buttonDown)
    {
      state[0].store(ButtonState::Released, std::memory_order_release);
    }
    break;
  }

  case ButtonState::Released:
  {
    if (outputCleared.load(std::memory_order_release))
    {
      state[0].store(ButtonState::Open, std::memory_order_release);
      state[1].store(ButtonState::Open, std::memory_order_release);
    }
    // State persists until external read and clear
    break;
  }

  default:
    break;
  }

#ifdef DEBUG_BUTTON_STATES

  //////////////////////////////////////////
  // SERIAL DEBUGGING
  ButtonState current = state[1].load(std::memory_order_acquire);
  if (current != tmpState[1])
  {
    switch (current)
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
  tmpState[0] = state[0].load(std::memory_order_acquire);
  tmpState[1] = current;
  //////////////////////////////////////////
#endif
};

// Report current state and free to record further clicks.
// State only resets if button has been released, else
// HELD or PRESSED will be returned on each call
ButtonState MagicButton::read(void)
{
  ButtonState s0 = state[0].load(std::memory_order_acquire);
  if (s0 == ButtonState::Released)
  {
    outputCleared.store(true, std::memory_order_release);
  }
  return state[1].load(std::memory_order_acquire);
}
