// ------------------------------------------------------------------------
// GateIn.h
//
// Aug. 2024
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------

#ifndef GATE_IN_H
#define GATE_IN_H
#include "Arduino.h"
#include "DirectIO.h"
#include <freertos/semphr.h>


// Abstract Base Class for reading and storing the instantaneous states and keeping
// track of rising and falling edges of a bank of digital inputs
class GateInABC
{
protected:

  // These are going to be our mapped values, i.e. Gate 0 <--> BIT0, Gate 1 <--> BIT1, etc.
  volatile uint32_t _gates;
  volatile uint32_t _gatesDiff;
  volatile uint32_t _rising;
  volatile uint32_t _falling;

  // You can't have more than this many gates in a single object of this class.
  // If you want to make this smaller, you can also change the volatiles to save some memory,
  // like change them to uint_8 if you only need 8 or fewer values. You could also make them
  // uint64_t if you want more than 32.
  static const uint8_t MAX_GATES = 32;
  static const TickType_t PATIENCE = 10;
  const uint8_t NUM_GATES;

  virtual uint32_t readPins() = 0;

  SemaphoreHandle_t mutex;

  // This is an Abstract Base Class, meant only to be inherited from, so limit access to its
  // constructor
  GateInABC(const uint8_t numGates):
    NUM_GATES(numGates)
  {
    assert(numGates < 33);
    mutex = xSemaphoreCreateRecursiveMutex();
    reset();
  }

public:

  // Return everything to defaults
  void reset()
  {
    assert(xSemaphoreTakeRecursive(mutex, PATIENCE));
    _gates      = 0;
    _gatesDiff  = 0;
    _rising     = 0;
    _falling    = 0;
    xSemaphoreGiveRecursive(mutex);
  }

  // Call this in an ISR or in a loop.
  // You should service all your input gates at an interval that is less than the
  // shortest pulse you hope to register, e.g. if you want to catch a 10 millisecond
  // trigger, you'll need to call this faster than that
  virtual void service()
  {
    assert(pdTRUE == xSemaphoreTake(mutex, PATIENCE));
    uint32_t prev(_gates);

    _gates      = readPins();
    _gatesDiff  = _gates ^ prev;
    _rising    |= (_gatesDiff & _gates);
    _falling   |= (_gatesDiff & ~_gates);
    xSemaphoreGive(mutex);
  }

  // If you get a rising edge on any given input, it will be stored until you read it.
  bool readRiseFlag(uint8_t gate)
  {
    assert(xSemaphoreTake(mutex, PATIENCE));
    bool ret(bitRead(_rising, gate));
    bitWrite(_rising, gate, 0);
    xSemaphoreGive(mutex);
    return ret;
  }

  // If you get a falling edge on any given input, it will be stored until you read it.
  bool readFallFlag(uint8_t gate)
  {
    assert(xSemaphoreTake(mutex, PATIENCE));
    bool ret(bitRead(_falling, gate));
    bitWrite(_falling, gate, 0);
    xSemaphoreGive(mutex);
    return ret;
  }


  virtual bool peekGate(uint8_t gate) = 0;
  virtual bool peekDiff(uint8_t gate) = 0;
};


// Arduino-specific implementation of GateIn class
class GateInArduino : public GateInABC
{
protected:
  uint8_t INPUT_MAP[MAX_GATES];
  bool _pullup;

  virtual uint32_t readPins() override
  {
    uint32_t ret(0);

    for (uint8_t gate(0); gate < NUM_GATES; ++gate)
    {
#ifdef DIRECT_IO_AITCH
      // Li'l bit faster
      bool val(directRead(INPUT_MAP[gate]) ^ _pullup);
#else
      // Tried and true
      bool val(digitalRead(INPUT_MAP[gate]) ^ _pullup);
#endif
      bitWrite(ret, gate, val);
    }
    return ret;
  }

public:
  GateInArduino(const uint8_t numGates, const uint8_t pins[], bool pullup = false) :
    GateInABC(numGates),
    _pullup(pullup)
  {
    for (auto gateNum(0); gateNum < NUM_GATES; ++gateNum)
    {
      uint8_t pinNum(pins[gateNum]);
      INPUT_MAP[gateNum] = pinNum;
      pinMode(pinNum, INPUT);
      if (_pullup)
      {
        digitalWrite(pinNum, HIGH);
      }
    }
  }

  void setActiveLow(bool activeLow = true)
  {
    _pullup = activeLow;
  }


  virtual bool peekGate(uint8_t gate) override
  {
    assert(xSemaphoreTake(mutex, PATIENCE));
    bool ret(_gates & (1 << gate));
    xSemaphoreGive(mutex);
    return ret;
  }

  virtual bool peekDiff(uint8_t gate) override
  {
    assert(xSemaphoreTake(mutex, PATIENCE));
    bool ret(_gatesDiff & (1 << gate));
    xSemaphoreGive(mutex);
    return ret;
  }

  virtual bool anyDiff()
  {
    assert(xSemaphoreTake(mutex, PATIENCE));
    bool ret = (_gatesDiff != 0);
    xSemaphoreGive(mutex);
    return ret;
  }
};


#endif