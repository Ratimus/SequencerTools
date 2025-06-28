#pragma once

#include <Arduino.h>
#include <memory>
#include <vector>

#include <SequencerTools/ADC_Object.h>

////////////////////////////////////////////////
// UNLOCKED:         control value is whatever the current reading is
// UNLOCK_REQUESTED: control will unlock if/when current reading matches lockControl value,
//                   else it returns lockControl value
// LOCKED:           ignore vurrent reading and return lockControl value
enum LockState
{
  STATE_UNLOCKED = 0,
  STATE_UNLOCK_REQUESTED,
  STATE_LOCKED
};

// Unlock control when within this percent difference from the lockControl value
static const double DEFAULT_THRESHOLD(0.01);

class ControlObject
{
protected:
  std::shared_ptr<SmoothedADC> pADC;
  uint16_t numCtrlVals;

  volatile LockState lockState;
  volatile uint16_t lockCtrlVal;
  volatile uint16_t currentRawVal;

public:
  ControlObject(ADC_Object *inADC,
                uint16_t numVals,
                uint16_t defaultControlVal = 0) : numCtrlVals(numVals),
                                                  lockState(STATE_LOCKED),
                                                  lockCtrlVal(defaultControlVal), currentRawVal(0)
  {
    pADC = std::make_shared<SmoothedADC>(std::shared_ptr<ADC_Object>(inADC), 100);
  }

  ControlObject(std::shared_ptr<ADC_Object> inADC,
                uint16_t numVals,
                uint16_t defaultControlVal = 0) : numCtrlVals(numVals),
                                                  lockState(STATE_LOCKED),
                                                  lockCtrlVal(defaultControlVal),
                                                  currentRawVal(0)
  {
    pADC = std::make_shared<SmoothedADC>(inADC, 100);
  }

  uint16_t getMin(void);
  uint16_t getMax(void);
  LockState getLockState(void);
  void setMin(uint16_t min);
  void setMax(uint16_t max);
  void lockControl();
  LockState reqUnlock();
  void setLockVal(int16_t jamVal);

  uint16_t rawValToControlVal(uint16_t rawVal);
  uint16_t controlValToRawVal(uint16_t tgtVal);

  uint16_t read(void);
  void service(void);
  void overWrite(void);
  ////////////////////////////////////////////////
  // Double equal all the way across the sky
  friend bool operator==(const ControlObject &CO1, const ControlObject &CO2)
  {
    return (&CO1 == &CO2);
  }

  ////////////////////////////////////////////////
  // Some objects are more equal than others
  friend bool operator!=(const ControlObject &CO1, const ControlObject &CO2)
  {
    return !(CO1 == CO2);
  }
};
