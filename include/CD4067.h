#pragma once
#include <Arduino.h>
#include <freertos/task.h>


static const uint8_t GRAY_CODE[16] = {0,1,3,2,6,7,5,4,12,13,15,14,10,11,9,8};
void IRAM_ATTR muxTask(void *param);


class HW_Mux
{
  uint8_t ADDR[4];
  const uint8_t IO;

  void muxEnable(uint8_t channel);
  SemaphoreHandle_t timingSemaphore;
  SemaphoreHandle_t resourceMutex;
  TaskHandle_t muxTaskHandle;

  volatile uint16_t MUXREG;
  uint16_t tmp;

  friend void muxTask(void * param);
public:

  HW_Mux(const uint8_t* const addrPins, uint8_t ioPin);

  void ICACHE_RAM_ATTR service()
  {
    static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(timingSemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken)
    {
      portYIELD_FROM_ISR(); // this wakes Task immediately
    }
  }

  uint16_t getReg(void);
};
