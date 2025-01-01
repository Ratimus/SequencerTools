#pragma once

#include <Arduino.h>
#include <ADC_Object.h>
#include <ControlObject.h>

////////////////////////////////////////////////
//
// Manager class allowing multiple modes / scenes / control pages to share a
// single ADC channel
class MultiModeCtrl
{
  uint8_t numModes;
  std::shared_ptr<ControlObject> pActiveCtrl;
  std::vector<std::shared_ptr<ControlObject>> pVirtualCtrls;

  std::shared_ptr<ControlObject> getPtr(int8_t idx = -1)
  {
    if (idx == -1)
    {
      return pActiveCtrl;
    }

    return pVirtualCtrls.at(idx);
  }

  friend class ControllerBank;

public:

  MultiModeCtrl(ADC_Object *inAdc,
                uint8_t  numModes,
                uint16_t topOfRange,
                uint16_t defaultVal = 0):
      pActiveCtrl(nullptr),
      numModes(numModes)
  {
    for (uint8_t mode = 0; mode < numModes; ++mode)
    {
      pVirtualCtrls.push_back(std::make_shared<ControlObject>(inAdc, topOfRange + 1, defaultVal));
    }

    pActiveCtrl = pVirtualCtrls.at(0);
  }

    MultiModeCtrl(std::shared_ptr<ADC_Object>inAdc,
                uint8_t  numModes,
                uint16_t topOfRange,
                uint16_t defaultVal = 0):
      pActiveCtrl(nullptr),
      numModes(numModes)
  {
    for (uint8_t mode = 0; mode < numModes; ++mode)
    {
      pVirtualCtrls.push_back(std::make_shared<ControlObject>(inAdc, topOfRange + 1, defaultVal));
    }

    pActiveCtrl = pVirtualCtrls.at(0);
  }

  uint16_t read();

  void setControlMin(uint16_t min, int8_t mode = -1)
  {
    getPtr(mode)->setMin(min);
  }

  void setControlMax(uint16_t max, int8_t mode = -1)
  {
    getPtr(mode)->setMax(max);
  }

  int16_t getMin(int8_t mode = -1)
  {
    return getPtr(mode)->getMin();
  }

  int16_t getMax(int8_t mode = -1)
  {
    return getPtr(mode)->getMax();
  }

  LockState getLockState(int8_t mode = -1)
  {
    return getPtr(mode)->getLockState();
  }

  uint16_t getRange(int8_t mode = -1)
  {
    return (getPtr(mode)->getMax() - getPtr(mode)->getMin());
  }

  void service();
  uint8_t getNumModes();

  void selectMode(uint8_t mode)
  {
    pActiveCtrl->lock();
    pActiveCtrl = pVirtualCtrls.at(mode);
  }

  void reqUnlock()
  {
    pActiveCtrl->reqUnlock();
  }

  void lock()
  {
    pActiveCtrl->lock();
  }

  void setLockVal(uint16_t jamVal, int8_t mode = -1)
  {
    getPtr(mode)->setLockVal(jamVal);
  }

  void setDefaults();

  void copySettings(uint8_t dest,
                    int8_t source = -1);

  void copySettings(std::shared_ptr<ControlObject> pDest,
                    std::shared_ptr<ControlObject> pSource);
};