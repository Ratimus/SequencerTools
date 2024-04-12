// ------------------------------------------------------------------------
// SharedCtrl.cpp
//
// January 2024
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#include <Arduino.h>
#include "MCP_ADC.h"
#include "SharedCtrl.h"
#include "RatFuncs.h"

////////////////////////////////////////////////
// Constructor
HardwareCtrl :: HardwareCtrl(
  MCP3208* inAdc,
  uint8_t inCh,
  uint8_t numSamps):
    pADC       (inAdc),
    ch         (inCh),
    adcMax     (pADC->maxValue()),
    buffSize   (constrain(numSamps, 1, MAX_BUFFER_SIZE-1)),
    sampleIdx  (0),
    sum_       (0),
    bufferReady(false)
{
  cli();
  for (uint8_t ii = 0; ii < MAX_BUFFER_SIZE; ++ii)
  {
    // Fill buffer so we already have a good average to start with
    buff[ii] = pADC->analogRead(ch);
    sum_ += buff[ii];
  }
  // bufferReady = true;
  sampleIdx = buffSize;
  sei();
}


////////////////////////////////////////////////
// Call this in an ISR at like 1ms or something
void HardwareCtrl :: service()
{
  if (pADC == NULL)
  {
    return;
  }

  cli();
  // Add one sample to the buffer
  buff[sampleIdx] = pADC->analogRead(ch);
  ++sampleIdx;
  if (sampleIdx >= buffSize)
  {
    sampleIdx   = 0;
    bufferReady = true;
  }
  sei();
}


////////////////////////////////////////////////
// Report 'ready' if buffer is full
bool HardwareCtrl :: isReady()
{
  bool retVal;
  cli();
  retVal = bufferReady;
  sei();
  return retVal;
}


////////////////////////////////////////////////
// Get the (smoothed) raw ADC value
int16_t HardwareCtrl :: read()
{
  uint8_t samps;
  int16_t retVal;
  sum_ = 0;

  cli();
  // Buffer isn't full; just return the first sample
  if (!bufferReady)
  {
    samps  = 0;
    retVal = buff[0];
  }
  else
  {
    samps = buffSize;
  }
  sei();
  if (samps == 0)
  {
    return retVal;
  }

  // Take the mean of all samples in the buffer
  for (uint8_t ii = 0; ii < buffSize; ++ii)
  {
    cli();
    sum_ += buff[ii];
    sei();
  }

  return (sum_ / (int16_t)buffSize);
}


////////////////////////////////////////////////
// Get the highest value the ADC can return
int16_t HardwareCtrl :: maxValue()
{
  return adcMax;
}


////////////////////////////////////////////////
// Defines a control that can be locked and unlocked
// You probably won't instantiate one of these directly. Rather,
// you'll create a VirtualControl (which extends this class).
////////////////////////////////////////////////
// Constructor
LockingCtrl :: LockingCtrl(
  MCP3208 * pADC,
  uint8_t adcChannel,
  int16_t inVal,
  bool    createLocked):
    min_    (0),
    max_    (pADC->maxValue()),
    lockVal_ (inVal)
{
  pHwCtrl_ = new HardwareCtrl(pADC, adcChannel, MAX_BUFFER_SIZE);
  // dbprintf("LockingCtrl %p :: pHwCtrl = %p\n", this, pHwCtrl_);
  while (!pHwCtrl_->isReady())
  {
    pHwCtrl_->service();
  }
  // dbprintf("  HwCtrl %p initialized!\n\n", pHwCtrl_);
  threshInt_ = static_cast<uint16_t>( (uint16_t)(DEFAULT_THRESHOLD * max_ + 0.5) );
  state_ = STATE_LOCKED;
  if (!createLocked)
  {
    reqUnlock();
  }
}

LockingCtrl :: ~LockingCtrl()
{
  delete pHwCtrl_;
  pHwCtrl_ = 0;
}


////////////////////////////////////////////////
// Returns lower end of range
int16_t LockingCtrl :: getMin()
{
  return min_;
}


////////////////////////////////////////////////
// Returns upper end of range
int16_t LockingCtrl :: getMax()
{
  return max_;
}


////////////////////////////////////////////////
// Get the lock value regardles of lock state
int16_t LockingCtrl :: getLockVal()
{
  return lockVal_;
}


////////////////////////////////////////////////
// LockState getter
LockState LockingCtrl :: getLockState()
{
  LockState tmpState;
  cli();
  tmpState = state_;
  sei();
  return tmpState;
}


////////////////////////////////////////////////
// LockState setter
LockState LockingCtrl :: setLockState_(LockState state)
{
  LockState tmpState(getLockState());
  if (state != tmpState)
  {
    cli();
    state_ = state;
    sei();
  }

  return tmpState;
}


////////////////////////////////////////////////
// Returns current ADC reading if unlocked, else returns locked value
int16_t LockingCtrl :: read()
{
  LockState tmpState(getLockState());

  // Return lockVal if locked
  if (tmpState == STATE_LOCKED || !pHwCtrl_->isReady())
  {
    return lockVal_;
  }

  int16_t tmpVal(pHwCtrl_->read());
  if (tmpState == STATE_UNLOCKED)
  {
    return tmpVal;
  }

  // STATE_UNLOCK_REQUESTED
  if (abs(lockVal_ - tmpVal) < threshInt_)
  {
    setLockState_(STATE_UNLOCKED);
    return tmpVal;
  }

  return lockVal_;
}


////////////////////////////////////////////////
// Returns the raw ADC value regardless of LockState
int16_t LockingCtrl :: peekMeasuredVal()
{
  return pHwCtrl_->read();
}


////////////////////////////////////////////////
// Sets LockVal to current (measured) real value regardless of LockState
void LockingCtrl :: overWrite()
{
  LockState tmpState(getLockState());
  setLockVal(peekMeasuredVal());
  if (tmpState != STATE_LOCKED)
  {
    reqUnlock();
  }
}


////////////////////////////////////////////////
// Ignore current reading, overwrite the lock value with jamVal
void LockingCtrl :: setLockVal(int16_t jamVal)
{
  lockVal_ = jamVal;
}


////////////////////////////////////////////////
// Lock the control at its current value if it isn't already locked
void LockingCtrl :: lock()
{
  setLockVal(read());
  setLockState_(LockState :: STATE_LOCKED);
}


////////////////////////////////////////////////
// Activates the control; it can now be unlocked
LockState LockingCtrl :: reqUnlock()
{
  if (getLockState() == STATE_LOCKED)
  {
    // dbprintf("LockingCtrl    %p UNLOCK_REQUESTED\n", this);
    setLockState_(STATE_UNLOCK_REQUESTED);
  }

  read();
  return getLockState();
}


////////////////////////////////////////////////
// Returns true if ADC sample buffer is full
bool LockingCtrl :: isReady()
{
  return pHwCtrl_->isReady();
}


////////////////////////////////////////////////////////////////////////
// VIRTUAL CTRL
////////////////////////////////////////////////////////////////////////
// Inherits from LockingControl; returns a limited number of options rather
// than a raw ADC value and uses hysteresis to prevent erratic mode-switching
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////
// Constructor
VirtualCtrl :: VirtualCtrl(
  MCP3208 * pADC,
  uint8_t adcChannel,
  int16_t inSlice,
  int16_t max,
  int16_t min,
  bool createLocked):
    LockingCtrl(pADC, adcChannel, inSlice, createLocked)
{
  while (!pHwCtrl_->isReady())
  {
    pHwCtrl_->service();
  }
  threshInt_ = static_cast<uint16_t>( (uint16_t)(DEFAULT_THRESHOLD * max_ + 0.5) );
  state_ = STATE_LOCKED;
  min_ = min;
  max_ = max;
  if (!createLocked)
  {
    reqUnlock();
  }
}


////////////////////////////////////////////////
// Change the range of values the control can return
void VirtualCtrl :: setMaxAndMin(int16_t max, int16_t min)
{
  // dbprintf("VirtualControl %p range WAS [%d - %d], IS [%d - %d]\n", this, min_, max_, min, max);
  if (max < lockVal_ || min > lockVal_)
  {
    setLockVal(map(lockVal_, min_, max_ + 1, min, max + 1));
  }
  min_ = min;
  max_ = max;
}


////////////////////////////////////////////////
// Returns current (measured) value regardless of LockState
int16_t VirtualCtrl :: peekMeasuredVal()
{
  return valToSlice(pHwCtrl_->read());
}


////////////////////////////////////////////////
// Get the value of the control:
//  If an unlock is requested and its measured val == LockVal, unlock it
//  If it's locked, return LockVal
//  If it's unlocked, return measured val
int16_t VirtualCtrl :: read()
{
  LockState tmpState(getLockState());

  // Return lockVal if locked
  if (tmpState == STATE_LOCKED || !pHwCtrl_->isReady())
  {
    return lockVal_;
  }

  int16_t rawHwVal(pHwCtrl_->read());
  int16_t rawHwSlice(peekMeasuredVal());
  if (tmpState == STATE_UNLOCKED)
  {
    return rawHwSlice;
  }

  // STATE_UNLOCK_REQUESTED
  int16_t targetVal(sliceToVal(lockVal_));
  // if (abs(targetVal - rawHwVal) < threshInt_)
  if (rawHwSlice == lockVal_)
  {
    setLockState_(STATE_UNLOCKED);
    // dbprintf("VirtualControl %p UNLOCKED @ %d [%d, tgtVal=%d]\n", this, rawHwSlice, rawHwVal, targetVal);
    return rawHwSlice;
  }

  return lockVal_;
}


////////////////////////////////////////////////
// Figure out what ADC reading you'd need to match the given control value [tgtSlice]
int16_t VirtualCtrl :: sliceToVal(int16_t tgtSlice)
{
  return map(tgtSlice, min_, max_ + 1, 0, pHwCtrl_->maxValue() + 1);
}


////////////////////////////////////////////////
// Get the control value corresponding to a given ADC value [val]
int16_t VirtualCtrl :: valToSlice(int16_t val)
{
  return map(val, 0, pHwCtrl_->maxValue() + 1, min_, max_ + 1);
}


////////////////////////////////////////////////
// Manager class to serve as a single point of interaction for an array of virtual controls
// in which only one virtual control is active at a time
MultiModeCtrl :: MultiModeCtrl(
  uint8_t numCtrls,
  MCP3208 * pADC,
  uint8_t adcChannel,
  uint8_t numVals):
    numModes_(numCtrls)
{
  for (auto idx(0); idx < numModes_; ++idx)
  {
    VirtualCtrl * pvc = new VirtualCtrl(pADC,
                                        adcChannel,
                                        numVals / 2,
                                        numVals);
    pVirtualCtrls[idx] = pvc;
  }
  pActiveCtrl = new VirtualCtrl(pADC,
                                adcChannel,
                                numVals / 2,
                                numVals,
                                0,
                                false);
}


////////////////////////////////////////////////
// Returns the number of VirtualCtrls sharing a single HwCtrl
uint8_t MultiModeCtrl :: getNumModes()
{
  return numModes_;
}


////////////////////////////////////////////////
// Returns the value of the currently selected VirtualCtrl
int16_t MultiModeCtrl :: read()
{
  return pActiveCtrl->read();
}


////////////////////////////////////////////////
// Locks the current VirtualCtrl and activates sel
void MultiModeCtrl :: selectActiveBank(uint8_t bank)
{
  copySettings(pActiveCtrl, pVirtualCtrls[bank]);
}


////////////////////////////////////////////////
// Lock the active VirtualCtrl
void MultiModeCtrl :: lock()
{
  pActiveCtrl->lock();
}


////////////////////////////////////////////////
// Sets the LockVal for the current active VirtualCtrl
void MultiModeCtrl :: setLockVal(int16_t jamVal)
{
  pActiveCtrl->setLockVal(jamVal);
}


////////////////////////////////////////////////
// Sets the LockVal for the current active VirtualCtrl with its real
// (measured) value regardless of LockState
void MultiModeCtrl :: setDefaults()
{
  pActiveCtrl->overWrite();
}


////////////////////////////////////////////////
// Set the min_ and max_ for the selected VirtualCtrl[sel]
void MultiModeCtrl :: setRange(uint8_t sel, int16_t max, int16_t min)
{
  setRange(pVirtualCtrls[sel], max, min);
}


void MultiModeCtrl :: setRange(VirtualCtrl * pDest, int16_t max, int16_t min)
{
  LockState tmpState(pDest->getLockState());
  pDest->lock();
  pDest->setMaxAndMin(max, min);
  if (tmpState != LockState :: STATE_LOCKED)
  {
    pDest->reqUnlock();
  }
}


void MultiModeCtrl :: setRange(uint8_t octaves)
{
  setRange(pActiveCtrl, octaves * 12, 0);
}

////////////////////////////////////////////////
// Copies the LockVal, min_, and max_ from VirtualCtrl[source] into VirtualCtrl[dest]
void MultiModeCtrl :: copySettings(uint8_t dest, int8_t source)
{
  printf("copying slot %u [val=%i] to slot %u\n", source, pVirtualCtrls[source]->read(), dest);
  copySettings(pVirtualCtrls[dest], pVirtualCtrls[source]);
}


void MultiModeCtrl :: copySettings(VirtualCtrl * pDest, VirtualCtrl * pSource)
{
  if (pDest == pSource)
  {
    return;
  }

  LockState tmpState(pDest->getLockState());
  pDest->lock();
  pDest->setLockVal(pSource->read());
  setRange(pDest, pSource->getMax(), pSource->getMin());
  if (tmpState != LockState :: STATE_LOCKED)
  {
    pDest->reqUnlock();
  }
}

void MultiModeCtrl :: saveActiveCtrl(uint8_t dest)
{
  copySettings(pVirtualCtrls[dest], pActiveCtrl);
}

