// ------------------------------------------------------------------------
// SharedCtrl.h
//
// January 2024
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#ifndef SHARED_CONTROL_H
#define SHARED_CONTROL_H

#include <Arduino.h>
#include "MCP_ADC.h"

// Maximum number of samples to average (higher values smooth noise)
static const uint8_t MAX_BUFFER_SIZE(128);

// Unlock knob when within this percent difference from the lock value
static const double  DEFAULT_THRESHOLD(0.05);

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

// Wraps an ADC channel for use with control classes
class HardwareCtrl
{
protected:
  MCP3208*  pADC;
  const    uint8_t  ch;
  const    int16_t  adcMax;

  uint8_t  buffSize;
  volatile int16_t  buff[MAX_BUFFER_SIZE];
  volatile uint8_t  sampleIdx;
  int64_t  sum_;
  volatile bool bufferReady;

public:

  // Constructor
  explicit HardwareCtrl(
    MCP3208* inAdc,
    uint8_t inCh,
    uint8_t numSamps = 1);

  void service();
  bool isReady();
  virtual int16_t read();
  virtual int16_t maxValue();
};



////////////////////////////////////////////////
// Defines a control that can be locked and unlocked
// You probably won't instantiate one of these directly. Rather,
// you'll create a VirtualControl (which extends this class).
class LockingCtrl
{
protected:
  HardwareCtrl*      pHwCtrl_;
  volatile LockState state_;
  int16_t min_;
  int16_t max_;
  int16_t lockVal_;
  uint16_t threshInt_;
  LockState setLockState_(LockState state);

public:

  void service() { pHwCtrl_->service(); }

  ////////////////////////////////////////////////
  // Double equal all the way across the skyh
  friend bool operator== (const LockingCtrl& L1, const LockingCtrl& L2)
  {
    // Serial.printf("L1: %p;  L2: %p\n", &L1, &L2);
    return (&L1 == &L2);
  }

  ////////////////////////////////////////////////
  // Some objects are more equal than others
  friend bool operator!= (const LockingCtrl& L1, const LockingCtrl& L2)
  {
    // Serial.printf("L1: %p;  L2: %p\n", &L1, &L2);
    return (&L1 != &L2);
  }

  ////////////////////////////////////////////////
  // Constructor
  LockingCtrl(){;}
  LockingCtrl(
    MCP3208 * pADC,
    uint8_t adcChannel,
    int16_t inVal,
    bool createLocked = true);

  ~LockingCtrl();

  int16_t getMin();
  int16_t getMax();
  int16_t getLockVal();
  LockState getLockState();
  void overWrite();
  bool isReady();

  virtual void lock();
  virtual int16_t read();
  virtual LockState reqUnlock();
  virtual void setLockVal(int16_t jamVal);
  virtual int16_t peekMeasuredVal();

};

////////////////////////////////////////////////////////////////////////
// VIRTUAL CTRL
////////////////////////////////////////////////////////////////////////
// Inherits from LockingControl; returns a limited number of options rather
// than a raw ADC value and uses hysteresis to prevent erratic mode-switching
class VirtualCtrl : public LockingCtrl
{
  int16_t valToSlice(int16_t val);
  int16_t sliceToVal(int16_t tgtSlice);

public:

  // Constructor
VirtualCtrl(): LockingCtrl() {;}
VirtualCtrl(
  MCP3208 * pADC,
  uint8_t adcChannel,
  int16_t inSlice,
  int16_t min,
  int16_t max = 0,
  bool createLocked = true);

  int16_t peekMeasuredVal();
  int16_t read();
  void setMaxAndMin(int16_t min, int16_t max = 0);
};


////////////////////////////////////////////////
//
// Manager class to serve as a single point of interaction for an array of virtual controls
// in which only one virtual control is active at a time
class MultiModeCtrl
{
  uint8_t numModes_;
  VirtualCtrl * pActiveCtrl;

public:
  VirtualCtrl * pVirtualCtrls[8]; // Array of ptrs (NOT a ptr to an array)

  MultiModeCtrl(uint8_t numCtrls, MCP3208 * pADC, uint8_t adcChannel, uint8_t numVals);
  uint16_t getLockVal() { return pActiveCtrl->getLockVal(); }
  uint8_t getNumModes();
  int16_t read();
  int16_t getMin() { return pActiveCtrl->getMin(); }
  int16_t getMax() { return pActiveCtrl->getMax(); }
  LockState getLockState() { return pActiveCtrl->getLockState(); }
  void selectActiveBank(uint8_t bank);
  void lock();

  void service() { pActiveCtrl->service(); }
  void setLockVal(int16_t jamVal);

  void setDefaults();
  void setRange(uint8_t mode, int16_t max, int16_t min = 0);
  void setRange(VirtualCtrl * pDest, int16_t max, int16_t min);
  void setRange(uint8_t octaves);

  void copySettings(uint8_t dest, int8_t source);
  void copySettings(VirtualCtrl * pDest, VirtualCtrl * pSource);
  void saveActiveCtrl(uint8_t dest);


  ~MultiModeCtrl()
  {
    for (auto idx(0); idx < numModes_; ++idx)
    {
      delete pVirtualCtrls[idx];
      pVirtualCtrls[idx] = 0;
    }
    pActiveCtrl = 0;
  }
};

#endif
