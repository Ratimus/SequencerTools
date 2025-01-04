#pragma once

#include <Arduino.h>
#include <memory>
#include <ADC_Object.h>
#include <vector>

////////////////////////////////////////////////
// UNLOCKED:         control value is whatever the current reading is
// UNLOCK_REQUESTED: control will unlock if/when current reading matches lock value,
//                   else it returns lock value
// LOCKED:           ignore vurrent reading and return lock value
enum LockState
{
  STATE_UNLOCKED = 0,
  STATE_UNLOCK_REQUESTED,
  STATE_LOCKED
};

// Unlock control when within this percent difference from the lock value
static const double  DEFAULT_THRESHOLD(0.05);

class ControlObject
{
protected:
  LockState lockState;
  uint16_t  lockCtrlVal;

  uint16_t  currentRawVal;
  uint16_t  numCtrlVals;

  std::shared_ptr<ADC_Object> pADC;

public:

  ControlObject(ADC_Object *inADC,
                uint16_t numVals,
                uint16_t defaultControlVal = 0):
      lockState(STATE_LOCKED),
      numCtrlVals(numVals),
      lockCtrlVal(defaultControlVal)
  {
    pADC = std::make_shared<SmoothedADC>(std::shared_ptr<ADC_Object>(inADC), 100);
  }

  ControlObject(std::shared_ptr<ADC_Object>inADC,
                uint16_t numVals,
                uint16_t defaultControlVal = 0):
      lockState(STATE_LOCKED),
      numCtrlVals(numVals),
      lockCtrlVal(defaultControlVal)
  {
    pADC = std::make_shared<SmoothedADC>(inADC, 100);
  }

  uint16_t  getMin(void);
  uint16_t  getMax(void);
  LockState getLockState(void);
  void      setMin(uint16_t min);
  void      setMax(uint16_t max);
  void      lock();
  LockState reqUnlock();
  void      setLockVal(int16_t jamVal);

  uint16_t  rawValToControlVal(uint16_t rawVal);
  uint16_t  controlValToRawVal(uint16_t tgtVal);

  uint16_t  read(void);
  void      service(void);
  void      overWrite(void);
  ////////////////////////////////////////////////
  // Double equal all the way across the sky
  friend bool operator == (const ControlObject& CO1, const ControlObject& CO2)
  {
    return (&CO1 == &CO2);
  }

  ////////////////////////////////////////////////
  // Some objects are more equal than others
  friend bool operator != (const ControlObject& CO1, const ControlObject& CO2)
  {
    return !(CO1 == CO2);
  }
};
