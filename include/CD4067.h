#pragma once
#include <Arduino.h>
#include <freertos/task.h>
#include <DirectIO.h>


static const uint8_t GRAY_CODE[16] = {0,1,3,2,6,7,5,4,12,13,15,14,10,11,9,8};

class HW_Mux
{
  uint8_t ADDR[4];
  const uint8_t IO;

  void muxEnable(uint8_t channel, uint8_t delayMicros = 0);

  volatile uint16_t MUXREG;
  friend void updateHW(void * param);

public:

  // Updates MUXREG with values of all 16 inputs
  void service()
  {
    if (pdTRUE == xSemaphoreTakeRecursive(resourceMutex, 10))
    {
      for (auto n: GRAY_CODE)
      {
        muxEnable(n, 10);
        (void)directRead(IO);
        bitWrite(MUXREG, n, !directRead(IO));
      }
      xSemaphoreGiveRecursive(resourceMutex);
    }
    else
    {
      Serial.println("muxenable fail");
    }
  }

  SemaphoreHandle_t resourceMutex;
  HW_Mux(const uint8_t* const addrPins, uint8_t ioPin);
  uint16_t getReg(void);
};
