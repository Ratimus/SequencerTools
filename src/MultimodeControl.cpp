#include <MultimodeControl.h>

//////////////////////////////////////////////
// Returns the number of VirtualCtrls sharing a single HwCtrl
uint8_t MultiModeCtrl::getNumModes()
{
  return numModes;
}

////////////////////////////////////////////////
// Returns the value of the currently selected VirtualCtrl
uint16_t MultiModeCtrl::read()
{
  if (!lock())
  {
    Serial.println("MMC read semtake failed");
    if (mutex == NULL) Serial.println("null ptr");
    while (1);
  }
  uint16_t ret = getPtr()->read();
  unlock();
  return ret;
}

void MultiModeCtrl::service()
{
  if (!lock())
  {
    Serial.println("MMC svc semtake failed");
    if (mutex == NULL) Serial.println("null ptr");
    while (1);
  }
  getPtr()->service();
  unlock();
}

////////////////////////////////////////////////
// Sets the LockVal for the current active VirtualCtrl with its real
// (measured) value regardless of LockState
void MultiModeCtrl::setDefaults()
{
  if (!lock())
  {
    Serial.println("MMC set defaults semtake failed");
    if (mutex == NULL) Serial.println("null ptr");
    while (1);
  }
  getPtr()->overWrite();
  unlock();
}

////////////////////////////////////////////////
// Copies the LockVal, min_, and max_ from VirtualCtrl[source] into VirtualCtrl[dest]
void MultiModeCtrl::copySettings(uint8_t dest, int8_t source)
{
  printf("copying slot %u [val=%i] to slot %u\n", source, pVirtualCtrls.at(source)->read(), dest);
  if ((int8_t)dest == source)
  {
    return;
  }
  if (!lock())
  {
    Serial.println("MMC copy ints semtake failed");
    if (mutex == NULL) Serial.println("null ptr");
    while (1);
  }
  copySettings(getPtr(dest), getPtr(source));
  unlock();
}


void MultiModeCtrl::copySettings(std::shared_ptr<ControlObject> pDest,
                                 std::shared_ptr<ControlObject> pSource)
{
  if (pDest == pSource)
  {
    return;
  }

  if (!lock())
  {
    Serial.println("MMC copy ptrs semtake failed");
    if (mutex == NULL) Serial.println("null ptr");
    while (1);
  }
  LockState tmpState(pDest->getLockState());
  // TODO: compare number of control vals
  pDest->lockControl();
  pDest->setLockVal(pSource->read());
  pDest->setMin(pSource->getMin());
  pDest->setMax(pSource->getMax());
  if (tmpState != LockState::STATE_LOCKED)
  {
    pDest->reqUnlock();
  }
  unlock();
}