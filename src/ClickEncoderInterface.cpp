// ------------------------------------------------------------------------
// ClickEncoderInterface.cpp
//
// Nov. 2022
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#include "ClickEncoderInterface.h"
#include <memory>

// Constructor
ClickEncoderInterface::ClickEncoderInterface(ClickEncoder *Enc, int8_t sense):
  pEncoder(Enc),
  pos(0),
  oldPos(0),
  heldClicked(0),
  mutex(xSemaphoreCreateRecursiveMutex())
{ ; }


ClickEncoderInterface::ClickEncoderInterface(HW_InterruptedClickEncoder *Enc, int8_t sense):
  pEncoder(Enc),
  pos(0),
  oldPos(0),
  heldClicked(0),
  mutex(xSemaphoreCreateRecursiveMutex())
{ ; }


ClickEncoderInterface::ClickEncoderInterface(
    uint8_t A,
    uint8_t B,
    uint8_t BTN,
    int8_t sense,
    uint8_t stepsPerNotch,
    bool usePullResistors):
  pEncoder(std::make_shared<ClickEncoder>(A, B, BTN, stepsPerNotch, usePullResistors)),
  pos(0),
  oldPos(0),
  heldClicked(0),
  mutex(xSemaphoreCreateRecursiveMutex())
{ ; }


encEvnts ClickEncoderInterface::getEvent(void)
{
  if (!lock())
  {
    Serial.println("shit no encoder semtake");
    return encEvnts::None;
  }

  ButtonState prevState    = btnState;
  oldPos                   = pos;
  pos                      = pEncoder->readPosition();
  btnState                 = pEncoder->readButton();
  ButtonState currentState = btnState;
  int deltaPos             = pos - oldPos;
  unlock();

  // Right Click
  if (deltaPos <= -1)
  {
    if (currentState == ButtonState::Held)
    {
      // Hold+Turn
      heldClicked = 1;
      return encEvnts::ShiftRight;
    }

    return encEvnts::Right;
  }

  // Left Click
  if (deltaPos >= 1)
  {
    if (currentState == ButtonState::Held)
    {
      // Hold+Turn
      heldClicked = 1;
      return encEvnts::ShiftLeft;
    }

    return encEvnts::Left;
  }

  if (prevState == currentState)
  {
    return encEvnts::None;
  }

  if (currentState != ButtonState::Open)
  {
    return encEvnts::None;
  }

  if (prevState == ButtonState::Clicked)
  {
    return encEvnts::Click;
  }

  if (prevState == ButtonState::DoubleClicked)
  {
    return encEvnts::DblClick;
  }

  if (prevState == ButtonState::ClickedAndHeld)
  {
    return encEvnts::ClickHold;
  }

  if (!heldClicked)
  {
    if (prevState == ButtonState::Pressed)
    {
      return encEvnts::Press;
    }

    return encEvnts::Hold;
  }

  heldClicked = 0;
  return encEvnts::None;
}


void ClickEncoderInterface::flush()
{
  assert(xSemaphoreTakeRecursive(mutex, 10) == pdTRUE);
  btnState = ButtonState::Open;
  pEncoder->readPosition();
  pEncoder->readButton();
  oldPos = pos;
  xSemaphoreGiveRecursive(mutex);
}
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