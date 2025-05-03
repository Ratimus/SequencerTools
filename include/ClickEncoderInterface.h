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
#include <ClickEncoder.h>
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
protected:

  std::shared_ptr<ClickEncoder> pEncoder;  // Associated hardware clickEncoder

  volatile ButtonState          btnState;  // Variable to store the state of the button
  volatile int oldPos;
  volatile int pos;

  bool heldClicked;

public:

  inline void init()
  {
    cli();
    pos     += pEncoder->readPosition();
    btnState = pEncoder->readButton();
    sei();
  }

  // Constructor using ref. to existing encoder driver object
  explicit ClickEncoderInterface(ClickEncoder *Enc, int8_t sense);

  ClickEncoderInterface(uint8_t A,
                        uint8_t B,
                        uint8_t BTN,
                        int8_t sense,
                        uint8_t stepsPerNotch,
                        bool usePullResistors);

  encEvnts getEvent(void);

  void flush();

  void service()
  {
    pEncoder->service();
  }
};

#endif
