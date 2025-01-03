// ------------------------------------------------------------------------
// ClickEncoderInterface.cpp
//
// Nov. 2022
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#include "ClickEncoderInterface.h"

// Constructor
ClickEncoderInterface::ClickEncoderInterface(ClickEncoder *Enc, int sense):
  pEncoder(Enc),
  sensivity(sense),
  pos(0),
  oldPos(0)
{
  update();
}


ClickEncoderInterface::ClickEncoderInterface(
    uint8_t A,
    uint8_t B,
    uint8_t BTN,
    int sense,
    uint8_t stepsPerNotch,
    bool usePullResistors):
  pEncoder(std::make_shared<ClickEncoder>(A, B, BTN, stepsPerNotch, usePullResistors))
{;}


encEvnts ClickEncoderInterface::getEvent(void)
{
  static bool heldClicked(0);
  ButtonState prevState = btnState;

  update();

  if (pos != oldPos)
  {
    int d = pos - oldPos;

    if (d <= -sensivity)                 // Right Click
    {
      oldPos -= sensivity;
      if (btnState == ButtonState::Held) // Hold+Turn
      {
        heldClicked = 1;
        return encEvnts::ShiftRight;
      }
      return encEvnts::Right;
    }

    if (d >= sensivity)             // Left Click
    {
      oldPos += sensivity;
      if (btnState == ButtonState::Held) // Hold+Turn
      {
        heldClicked = 1;
        return encEvnts::ShiftLeft;
      }

      return encEvnts::Left;
    }
  }

  if (prevState != btnState)
  {
    if (btnState == ButtonState::Open)
    {
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
    }
  }

  return encEvnts::NUM_ENC_EVNTS;
}

void ClickEncoderInterface::flush()
{
  btnState = ButtonState::Open;
  pEncoder->getClickCount();
  pEncoder->readButtonState();
  oldPos = pos;
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