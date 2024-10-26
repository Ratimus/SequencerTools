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
#include <memory>
#include <vector>
#include <RatFuncs.h>


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
  std::shared_ptr<MCP3208> pADC;
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
    std::shared_ptr<MCP3208> inAdc,
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
  std::shared_ptr<HardwareCtrl> pHwCtrl_;
  volatile LockState state_;
  int16_t min_;
  int16_t max_;
  int16_t lockVal_;
  uint16_t threshInt_;
  LockState setLockState_(LockState state);

public:

  void service() { pHwCtrl_->service(); }

  ////////////////////////////////////////////////
  // Double equal all the way across the sky
  friend bool operator == (const LockingCtrl& L1, const LockingCtrl& L2)
  {
    return (&L1 == &L2);
  }

  ////////////////////////////////////////////////
  // Some objects are more equal than others
  friend bool operator != (const LockingCtrl& L1, const LockingCtrl& L2)
  {
    return (&L1 != &L2);
  }

  ////////////////////////////////////////////////
  // Constructor
  LockingCtrl(){;}
  LockingCtrl(
    std::shared_ptr<MCP3208> inAdc,
    uint8_t adcChannel,
    int16_t inVal,
    bool createLocked = true);

  int16_t           getMin();
  int16_t           getMax();
  int16_t           getLockVal();
  LockState         getLockState();
  void              overWrite();
  bool              isReady();

  virtual void      lock();
  virtual int16_t   read();
  virtual LockState reqUnlock();
  virtual void      setLockVal(int16_t jamVal);
  virtual int16_t   peekMeasuredVal();

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
    std::shared_ptr<MCP3208> inAdc,
    uint8_t adcChannel,
    int16_t inSlice,
    int16_t max,
    int16_t min = 0,
    bool    createLocked = true);

    int16_t peekMeasuredVal();
    int16_t read();
    void    setMaxAndMin(int16_t max,
                         int16_t min = 0);
};


////////////////////////////////////////////////
//
// Manager class to serve as a single point of interaction for an array of virtual controls
// in which only one virtual control is active at a time
class MultiModeCtrl
{
  uint8_t numModes_;
  std::shared_ptr<VirtualCtrl> pActiveCtrl;

public:
  std::vector<std::shared_ptr<VirtualCtrl>> pVirtualCtrls; // Array of ptrs (NOT a ptr to an array)

  MultiModeCtrl(uint8_t numCtrls,
                std::shared_ptr<MCP3208> inAdc,
                uint8_t adcChannel,
                uint8_t numVals);

  uint16_t  getLockVal()    { return pActiveCtrl->getLockVal(); }
  int16_t   getMin()        { return pActiveCtrl->getMin(); }
  int16_t   getMax()        { return pActiveCtrl->getMax(); }
  LockState getLockState()  { return pActiveCtrl->getLockState(); }
  uint8_t   getRange()      { return getMax() - getMin(); }

  void      service()       { pActiveCtrl->service(); }

  uint8_t   getNumModes();
  int16_t   read();

  void      selectActiveBank(uint8_t bank);
  void      lock();

  void      setLockVal(int16_t jamVal);

  void      setDefaults();

  void      setRange(uint8_t octaves);
  void      setRange(uint8_t mode,
                     int16_t max,
                     int16_t min = 0);
  void      setRange(std::shared_ptr<VirtualCtrl> pDest,
                     int16_t max,
                     int16_t min);

  void      copySettings(uint8_t dest,
                         int8_t source);
  void      copySettings(std::shared_ptr<VirtualCtrl> pDest,
                         std::shared_ptr<VirtualCtrl> pSource);
  void      saveActiveCtrl(uint8_t dest);

  ~MultiModeCtrl()
  {
    pActiveCtrl = 0;
  }
};


////////////////////////////////////////////////
//
// *Slaps roof* You can fit so many MultiModeControls in this baby.
//
class ControllerBank
{
  uint8_t bankIdx;
  std::vector<std::shared_ptr<MultiModeCtrl>> faderBank;
  float ONE_OVER_ADC_MAX;
  uint8_t NUM_BANKS;
  uint8_t NUM_FADERS;
  std::shared_ptr<MCP3208> pADC;
  std::vector<uint8_t> sliderMap;
  static const uint8_t MAX_RANGE = 3;

  // Sets upper and lower bounds for faders based on desired octave range
  void setRange(uint8_t octaves);
  void init();

public:
  ControllerBank();
  ControllerBank(ControllerBank & proto);
  ControllerBank(uint8_t numFaders, uint8_t numBanks, const uint8_t sliderMapping[]);

  void init(const uint8_t SPI_DATA_OUT, const uint8_t SPI_DATA_IN, const uint8_t SPI_CLK, const uint8_t ADC_CS);
  void saveBank(uint8_t idx);
  void selectBank(uint8_t idx);
  void moreRange();
  void lessRange();
  uint8_t getRange();
  void service();
  uint16_t read(uint8_t ch);
  bool isLocked(uint8_t ch);

  uint8_t getLockByte();
};


#endif
