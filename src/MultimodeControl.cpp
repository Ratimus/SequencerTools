#include "MultimodeControl.h"

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
  cli();
  uint16_t ret = getPtr()->read();
  sei();
  return ret;
}

void MultiModeCtrl::service()
{
  getPtr()->service();
}

////////////////////////////////////////////////
// Sets the LockVal for the current active VirtualCtrl with its real
// (measured) value regardless of LockState
void MultiModeCtrl::setDefaults()
{
  cli();
  getPtr()->overWrite();
  sei();
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
  cli();
  copySettings(getPtr(dest), getPtr(source));
  sei();
}


void MultiModeCtrl::copySettings(std::shared_ptr<ControlObject> pDest,
                                 std::shared_ptr<ControlObject> pSource)
{
  if (pDest == pSource)
  {
    return;
  }

  cli();
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
  sei();
}
