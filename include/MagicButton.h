// ------------------------------------------------------------------------
// MagicButton.h
//
// Nov. 2022
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#ifndef MagicButton_h
#define MagicButton_h

#include <Arduino.h>
#include <CD4067.h>
#include <memory>
#include <DirectIO.h>
#include <freertos/semphr.h>


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


const uint16_t debounceUP(0b0111111111111111);  // More leading zeros will increase sensitivity
const int16_t  debounceDN(~debounceUP);

// Button configuration (values for 1ms timer service calls)
//
const uint16_t DOUBLECLICKTIME(150);  // Count two clicks as doubleclick if both received within this time
const uint16_t PRESSTIME(250);
const uint16_t HOLDTIME(350);        // Report held button after time

class MagicButton
{
protected:
  inline static const TickType_t PATIENCE = 10;

  int8_t   pin;         // HW pin
  bool     pullup;      // Set TRUE to enable pullup resistor if active low
  uint8_t  dbnceIntvl;  // How long to lockout bounce AFTER press/release
  bool     doubleClickable;

  volatile ButtonState state[2];
  volatile bool buttonDown;  // Raw data. We don't need to see it, we don't want to see it.
  volatile bool outputCleared;
  volatile long long debounceTS;
  volatile uint16_t buff;        // Moving window to record multiple readings

  bool lock()
  {
    if (xSemaphoreTakeRecursive(mutex, PATIENCE) != pdTRUE)
    {
      return false;
    }

    return true;
  }

  void unlock()
  {
    xSemaphoreGiveRecursive(mutex);
  }

  virtual bool readPin()
  {
    if (pin < 0)
    {
      return 0;
    }

    lock();
    bool ret = pullup ^ (bool)directRead(pin);
    unlock();
    return ret;
  }

  SemaphoreHandle_t mutex;

#ifdef DEBUG_BUTTON_STATES
  ButtonState tmpState[2];
#endif

public:
  // Constructor
  MagicButton(int8_t pin,
              bool pullup,
              bool doubleClickable):
    buff(0),
    pin(pin),
    debounceTS(0),
    buttonDown(0),
    dbnceIntvl(25),
    pullup(pullup),
    outputCleared(1),
    doubleClickable(doubleClickable),
    mutex(xSemaphoreCreateRecursiveMutex()),
    state{ButtonState::Open, ButtonState::Open}
  {
    if (pin != -1)
    {
      pinMode(pin, pullup ? INPUT_PULLUP : INPUT);
    }
  }

  void service();
  ButtonState read();
};


class MuxedButton : public MagicButton
{
  static inline std::shared_ptr<HW_Mux> _SHARED_MUX = NULL;
  const uint16_t _BITMASK;
  static inline uint16_t _REGISTER = 0;

public:

  MuxedButton(uint16_t bit):
    MagicButton(-1, true, true),
    _BITMASK((uint16_t)1 << bit)
  { ; }

  static void setMux(HW_Mux *pMux)
  {
    _SHARED_MUX = std::shared_ptr<HW_Mux>(pMux);
  }

  // You don't need to call this if you have other stuff on this mux and you already updated it
  static void updateReg()
  {
    _SHARED_MUX->service();
  }

  virtual bool readPin(void) override
  {
    lock();
    _REGISTER = _SHARED_MUX->getReg();
    bool ret = _REGISTER & _BITMASK;
    unlock();
    return ret;
  }
};

#endif
