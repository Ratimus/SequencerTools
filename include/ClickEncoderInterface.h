// ------------------------------------------------------------------------
// ClickEncoderInterface.h
//
// Nov. 2022
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#ifndef ClickEncoderInterface_h
#define ClickEncoderInterface_h

#include <Arduino.h>
#include <memory>
#include "ClickEncoder.h"
#include <MagicButton.h>

enum encEvnts
{
  None        = 0,
  Click       = BIT0,
  DblClick    = BIT1,
  Left        = BIT2,
  ShiftLeft   = BIT3,
  Right       = BIT4,
  ShiftRight  = BIT5,
  Press       = BIT6,
  ClickHold   = BIT7,
  Hold        = BIT8,
};


class ClickEncoderInterface
{
private:

  std::shared_ptr<ClickEncoder> pEncoder;  // Associated hardware clickEncoder
  volatile ButtonState          btnState;  // Variable to store the state of the button
  int8_t       sensivity;

  volatile int oldPos;
  volatile int pos;

  inline void update()
  {
    pos     += pEncoder->getClickCount();
    btnState = pEncoder->readButtonState();
  }

public:

  // Constructor using ref. to existing encoder driver object
  ClickEncoderInterface(ClickEncoder *Enc, int8_t sense);

  // Constructor to create and manage its own encoder driver object
  ClickEncoderInterface(uint8_t A,
                        uint8_t B,
                        uint8_t BTN,
                        int8_t sense,
                        uint8_t stepsPerNotch,
                        bool usePullResistors);

  encEvnts getEvent(void);

  inline void setSensivity(int8_t s)
  {
    sensivity = s;
  }

  void flush();

  inline void service()
  {
    pEncoder->service();
    update();
  }
};

#endif
