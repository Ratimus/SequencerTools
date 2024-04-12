// ------------------------------------------------------------------------
// OutputRegister.h
//
// Extends the concept defined in "Latchable.h" to a physical hardware
// shift register (e.g. 74HC595)
//
// December 2023
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#ifndef OUTPUT_REGISTER_DOT_AITCH
#define OUTPUT_REGISTER_DOT_AITCH
#include <Arduino.h>
#include <Latchable.h>
#include <bitHelpers.h>
#include <FastShiftOut.h>

// enable
// set
// clock
// clockIn
// clear
// reset
// pending
// preEnable

/*
  // setting all pins at the same time to either HIGH or LOW
  sr.setAllHigh(); // set all pins HIGH
  sr.setAllLow();  // set all pins LOW
  sr.set(i, HIGH); // set single pin HIGH

  // set all pins at once
  uint8_t pinValues[] = { B10101010 };
  sr.setAll(pinValues);

  // read pin (zero based, i.e. 6th pin)
  uint8_t stateOfPin5 = sr.get(5);
  sr.set(6, stateOfPin5);

  // set pins without immediate update
  sr.setNoUpdate(0, HIGH);
  sr.setNoUpdate(1, LOW);
  sr.updateRegisters(); // update the pins to the set values
  */

template <typename T>
  class OutputRegister : public latchable<T>
{
public:
  OutputRegister(
    uint8_t clkPin,
    uint8_t dataPin,
    uint8_t csPin,
    const uint8_t mapping[]):
      latchable<T>(T(0)),
      CLK(clkPin),
      DAT(dataPin),
      LCH(csPin),
      MAP(mapping),
      BYTE_COUNT(sizeof(T)),
      NUM_BITS(sizeof(T) * 8)
  {
    SR = new FastShiftOut(DAT, CLK, LSBFIRST);
  }

  ~OutputRegister()
  {
    delete SR;
    SR  = NULL;
    MAP = NULL;
  }

  T clock() override
  {
    latchable<T> :: clock();
    writeOutputRegister();
    return Q();
  }

  // Bypasses clock and increment; immediately writes {byteVal} to register {byteNum}
  void tempWrite(uint8_t byteVal, uint8_t byteNum = 0)
  {
    setReg(byteVal, byteNum);
    latchable<T> :: clock();
    writeOutputRegister();
  }

  // Implementation of bitWrite to selected register {byteNum}; requires clock to take effect
  void writeBit(uint8_t bitnum, bool val, uint8_t bytenum = 0)
  {
    uint8_t temp(getReg(bytenum));
    bitWrite(temp, bitnum, val);
    setReg(temp, bytenum);
  }

  // Returns register {byteNum}
  uint8_t getReg(uint8_t bytenum = 0)
  {
    return (D() >> (8 * bytenum)) & 0xFF;
  }

  // Overwrites register {byteNum} with {val}
  void setReg(uint8_t val, uint8_t bytenum = 0)
  {
    T setVal(T(val) << (8 * bytenum));
    T mask(T(0xFF)  << (8 * bytenum));
    T temp(D() & ~mask);
    temp |= setVal;
    latchable<T> :: set(temp);
  }

  // Returns Output value
  T Q()
  {
    return latchable<T> :: Q;
  }

  // Returns recent Input value
  T D()
  {
    return latchable<T> :: D;
  }

  void allOff()
  {
    digitalWrite(LCH, LOW);
    for (uint8_t bn(0); bn < BYTE_COUNT; ++bn)
    {
      SR->write(0);
    }
    digitalWrite(LCH, HIGH);
  }

protected:
  const uint8_t  CLK;
  const uint8_t  DAT;
  const uint8_t  LCH;
  const uint8_t *MAP;
  const uint8_t  NUM_BITS;
  const size_t   BYTE_COUNT;
  FastShiftOut  *SR;

  void writeOutputRegister()
  {
    memset(&REGISTER, 0, BYTE_COUNT);
    for (auto bitnum(0); bitnum < NUM_BITS; ++bitnum)
    {
      bitWrite(REGISTER, MAP[bitnum], bitRead(Q(), bitnum));
    }

    digitalWrite(LCH, LOW);
    for (uint8_t bytenum(0); bytenum < BYTE_COUNT; ++bytenum)
    {
      SR->write((uint8_t)(REGISTER >> (8 * bytenum)));
    }
    digitalWrite(LCH, HIGH);
  }

  T REGISTER;
};

#endif
