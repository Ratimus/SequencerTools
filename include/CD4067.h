#pragma once
#include <Arduino.h>
#include <DirectIO.h>


static const uint8_t GRAY_CODE[16] = {0,1,3,2,6,7,5,4,12,13,15,14,10,11,9,8};

class HW_Mux
{
  uint8_t ADDR[4];

  const uint8_t IO_0;
  const uint8_t IO_1;

  void muxEnable(uint8_t channel, uint8_t delayMicros = 0);

  volatile uint16_t MUXREG0;
  volatile uint16_t MUXREG1;
  friend void updateHW(void * param);

public:

  // Updates MUXREG with values of all 16 inputs
  void service()
  {
    cli();
    for (auto n: GRAY_CODE)
    {
      muxEnable(n, 10);
      (void)directRead(IO_0);
      (void)directRead(IO_1);
      bitWrite(MUXREG0, n, !directRead(IO_0));
      if (IO_1 != 255)
      {
        bitWrite(MUXREG1, n, !directRead(IO_1));
      }
    }
    sei();
  }

  HW_Mux(const uint8_t* const addrPins, uint8_t ioPin_0, uint8_t ioPin_1 = 255);
  uint16_t getReg(void)
  {
    return getReg0();
  }

  uint16_t getReg0(void);
  uint16_t getReg1(void);
};
