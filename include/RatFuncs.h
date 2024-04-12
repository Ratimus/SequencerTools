// ------------------------------------------------------------------------
// RatFuncs.h
//
// Oct. 2022
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#ifndef RatFuncs_h
#define RatFuncs_h

#include <Arduino.h>

// #ifndef CASSIDEBUG
// #define CASSIDEBUG
// #endif


#ifdef CASSIDEBUG
  #define dbprint Serial.print
  #define dbprintf Serial.printf
  #define dbprintln Serial.println
#else
  #define dbprint(...)
  #define dbprintf(...)
  #define dbprintln(...)
#endif

//////////////////////////////////////////////////////////////////
//
//               SERIAL DEBUGGING UTILITIES
//
//////////////////////////////////////////////////////////////////
void printBuff4x4SER(uint16_t pattBuf);
void dumpBufferSER(uint16_t reg, bool eightBits);
void printBits(uint8_t  val);
void printBits(uint16_t val);

//////////////////////////////////////////////////////////////////
//
//                  HW INTERFACE STUFF
//
//////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
//
//               COMPUTATIONAL HELPERS
//
//////////////////////////////////////////////////////////////////
int16_t wrapConstrain(int16_t N, int16_t dN, int16_t nMin, int16_t nMax);
int16_t wrapConstrain(int16_t N, int16_t nMin, int16_t nMax);
void wrapConstrain(int8_t * pN, int16_t nMin, int16_t nMax);
void wrapConstrain(int16_t * pN, int16_t nMin, int16_t nMax);

template <typename T> T getSign(T num)
{
  if (num == 0) return 0;
  return num/abs(num);
}


const byte MASK0(1 << 0);
const byte MASK1(MASK0 << 1);
const byte MASK2(MASK1 << 1);
const byte MASK3(MASK2 << 1);
const byte MASK4(MASK3 << 1);
const byte MASK5(MASK4 << 1);
const byte MASK6(MASK5 << 1);
const byte MASK7(MASK6 << 1);

const byte GATE_MASKS[8] = {MASK0, MASK1, MASK2, MASK3, MASK4, MASK5, MASK6, MASK7};


#endif
