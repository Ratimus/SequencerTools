#pragma once

#include <Arduino.h>
#include <ADC_Object.h>
#include <ControlObject.h>
#include <freertos/semphr.h>

////////////////////////////////////////////////
//
// Manager class allowing multiple modes / scenes / control pages to share a
// single ADC channel
class MultiModeCtrl
{
private:
  SemaphoreHandle_t mutex;
  volatile uint8_t activeIndex;
  uint8_t numModes;

  std::vector<std::shared_ptr<ControlObject>> pVirtualCtrls;

  std::shared_ptr<ControlObject> getPtr(int8_t idx = -1)
  {
    if (idx == -1)
    {
      return pVirtualCtrls[activeIndex];
    }

    return pVirtualCtrls[idx];
  }

  friend class ControllerBank;

  bool lock()
  {
    return (pdTRUE == xSemaphoreTake(mutex, 1));
  }

  void unlock()
  {
    xSemaphoreGive(mutex);
  }

  void lockControl()
  {
    if (getPtr())
    {
      getPtr()->lockControl();
    }
  }
public:

  MultiModeCtrl(ADC_Object *inAdc,
                uint8_t  numModes,
                uint16_t topOfRange,
                uint16_t defaultVal = 0):
      numModes(numModes)
  {
    mutex = xSemaphoreCreateRecursiveMutex();
    lock();
    for (uint8_t mode = 0; mode < numModes; ++mode)
    {
      pVirtualCtrls.push_back(std::make_shared<ControlObject>(inAdc, topOfRange + 1, defaultVal));
    }

    activeIndex = 0;
    unlock();
  }

  MultiModeCtrl(std::shared_ptr<ADC_Object>inAdc,
                uint8_t  numModes,
                uint16_t topOfRange,
                uint16_t defaultVal = 0):
      numModes(numModes)
  {
    mutex = xSemaphoreCreateRecursiveMutex();
    lock();
    for (uint8_t mode = 0; mode < numModes; ++mode)
    {
      pVirtualCtrls.push_back(std::make_shared<ControlObject>(inAdc, topOfRange + 1, defaultVal));
    }

    activeIndex = 0;
    unlock();
    // pActiveCtrl = pVirtualCtrls.at(0);
  }

  uint16_t read();

  void setControlMin(uint16_t min, int8_t mode = -1)
  {
    if (!getPtr(mode))
    {
      return;
    }
    getPtr(mode)->setMin(min);
  }

  void setControlMax(uint16_t max, int8_t mode = -1)
  {
    if (!getPtr(mode))
    {
      return;
    }
    getPtr(mode)->setMax(max);
  }

  int16_t getMin(int8_t mode = -1)
  {
    if (!getPtr(mode))
    {
      return 0;
    }
    return getPtr(mode)->getMin();
  }

  int16_t getMax(int8_t mode = -1)
  {
    if (!getPtr(mode))
    {
      return 0;
    }
    return getPtr(mode)->getMax();
  }

  LockState getLockState(int8_t mode = -1)
  {
    if (!getPtr(mode))
    {
      return STATE_LOCKED;
    }
    return getPtr(mode)->getLockState();
  }

  uint16_t getRange(int8_t mode = -1)
  {
    if (!getPtr(mode))
    {
      return 0;
    }
    return (getPtr(mode)->getMax() - getPtr(mode)->getMin());
  }

  void service();
  uint8_t getNumModes();

  void selectMode(uint8_t mode, bool reqUnlock = true)
  {
    if (!lock())
    {
      Serial.println("MMC selmode semtake failed");
      while (1);
    }

    if (getPtr())
    {
      getPtr()->lockControl();
    }

    activeIndex = mode;
    if (getPtr())
    {
      getPtr()->reqUnlock();
    }

    unlock();
  }

  void setLockVal(uint16_t jamVal, int8_t mode = -1)
  {
  if (!lock())
  {
    Serial.println("MMC setval semtake failed");
    while (1);
  }
    if (getPtr())
    {
      getPtr(mode)->setLockVal(jamVal);
    }

    unlock();
  }

  void setDefaults();

  void copySettings(uint8_t dest,
                    int8_t source = -1);

  void copySettings(std::shared_ptr<ControlObject> pDest,
                    std::shared_ptr<ControlObject> pSource);
};