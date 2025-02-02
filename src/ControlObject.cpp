#include <ControlObject.h>

#define SEM_TIMEOUT ((TickType_t)10)


////////////////////////////////////////////////
// Sets LockVal to current (measured) real value regardless of LockState
void ControlObject::overWrite()
{
  LockState tmpState(getLockState());
  read();
  uint16_t targetVal = rawValToControlVal(currentRawVal);
  setLockVal(targetVal);
  if (tmpState != STATE_LOCKED)
  {
    reqUnlock();
  }
}

bool ControlObject::lock()
{
  return (pdTRUE == xSemaphoreTakeRecursive(mutex, SEM_TIMEOUT));
}

void ControlObject::unlock()
{
  xSemaphoreGiveRecursive(mutex);
}

LockState ControlObject::getLockState(void)
{
  if (!lock())
  {
    Serial.println("ctl getlockstate semtake failed");
    while (1);
  }
  LockState ret = lockState;
  unlock();
  return ret;
}

void ControlObject::setMin(uint16_t min) { pADC->setMin(min); }
void ControlObject::setMax(uint16_t max) { pADC->setMax(max); }

uint16_t ControlObject::getMin(void) { return pADC->getMin(); }
uint16_t ControlObject::getMax(void) { return pADC->getMax(); }

////////////////////////////////////////////////
// Lock the control at its current value if it isn't already locked
void ControlObject::lockControl()
{
  if (!lock())
  {
    Serial.println("ctl lockControl semtake failed");
    while (1);
  }

  if (lockState != STATE_LOCKED)
  {
    lockState = STATE_LOCKED;
  }

  // Serial.printf("%p locked @ %u\n", this, lockCtrlVal);
  unlock();
}

////////////////////////////////////////////////
// Activates the control; it can now be unlocked
LockState ControlObject::reqUnlock()
{
  LockState ret;
  if (!lock())
  {
    Serial.println("ctl unlock semtake failed");
    while (1);
  }
  if (lockState == STATE_LOCKED)
  {
    lockState = STATE_UNLOCK_REQUESTED;
    pADC->reset();
    read();
  }
  ret = lockState;
  unlock();

  return ret;
}

////////////////////////////////////////////////
// Ignore current reading, overwrite the lockControl value with jamVal
void ControlObject::setLockVal(int16_t jamVal)
{
  if (!lock())
  {
    Serial.println("ctl setval semtake failed");
    while (1);
  }
  uint16_t tmpVal = lockCtrlVal;
  LockState tmpState = lockState;
  lockState = STATE_LOCKED;
  lockCtrlVal = jamVal;
  if (tmpState != STATE_LOCKED)
  {
    lockState = STATE_UNLOCK_REQUESTED;
  }
  unlock();
}

////////////////////////////////////////////////
// Get the control value corresponding to a given ADC value [val]
uint16_t ControlObject::rawValToControlVal(uint16_t rawVal)
{
  return (uint16_t)map(rawVal, pADC->getMin(), pADC->getMax() + 1, 0, numCtrlVals);
}

////////////////////////////////////////////////
// Figure out what ADC reading you'd need to match the given control value [tgtVal]
uint16_t ControlObject::controlValToRawVal(uint16_t tgtVal)
{
  return (uint16_t)map(tgtVal, 0, numCtrlVals, pADC->getMin(), pADC->getMax() + 1);
}

////////////////////////////////////////////////
// Returns current ADC reading if unlocked, else returns locked value
uint16_t ControlObject::read(void)
{
  service();
  if (!lock())
  {
    Serial.println("ctl read semtake failed");
    while (1);
  }

  if (lockState == STATE_LOCKED)
  {
    unlock();
    return lockCtrlVal;
  }

  // What control value would our current raw value give?
  uint16_t currentControlVal = rawValToControlVal(currentRawVal);
  if (currentControlVal == lockCtrlVal)
  {
    if (lockState == STATE_UNLOCK_REQUESTED)
    {
      lockState = STATE_UNLOCKED;
      // Serial.printf("%p unlocked @ %u\n", this, lockCtrlVal);
    }
  }

  if (lockState == STATE_UNLOCKED)
  {
    if (currentControlVal > lockCtrlVal)
    {
      uint16_t tgtRawValue = controlValToRawVal(currentControlVal);
      // Make sure you're part way into the higher value before switching
      if ((((float)currentRawVal - (float)tgtRawValue) / (float)tgtRawValue) > DEFAULT_THRESHOLD)
      {
        lockCtrlVal = currentControlVal;
      }
    }
    else if (currentControlVal < lockCtrlVal)
    {
      uint16_t tgtRawValue = controlValToRawVal(lockCtrlVal);
      // Make sure you're part way into the lower value before switching
      if ((((float)tgtRawValue - (float)currentRawVal) / (float)tgtRawValue) > DEFAULT_THRESHOLD)
      {
        lockCtrlVal = currentControlVal;
      }
    }
  }

  unlock();
  return lockCtrlVal;
}

void ControlObject::service(void)
{
  if (!lock())
  {
    Serial.println("ctl svc semtake failed");
    while (1);
  }
  pADC->service();
  currentRawVal = pADC->read();
  unlock();
}

