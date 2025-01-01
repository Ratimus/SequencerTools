////////////////////////////////////////////////////////////////////////////////////////////
//
// Hardware abstraction class and hardware-specific derived classes for a single ADC channel
//
#pragma once

#include <Arduino.h>
#include "MCP_ADC.h"
#include "ESP32AnalogRead.h"
#include <memory>

const uint8_t MAX_BUFFER_SIZE(128);

////////////////////////////////////////////////////////////////////////////////////////////
// Abstract class for a single ADC channel.
//
//  min:         lowest measurable value the HW will report
//  max:         highest measurable value the HW will report
//
//  getRawValue: returns measured value from ADC channel
//  read:        returns a value between 0 and 4095
class ADC_Object
{
protected:

  uint16_t adcMin;
  uint16_t adcMax;
  uint8_t  buffSize;

  volatile uint16_t val;

public:

  inline static const uint8_t INVALID_CHANNEL = 99;

  ADC_Object():
      adcMin(0),
      adcMax(4095)
  { ; }

  ADC_Object(uint16_t min, uint16_t max):
    adcMin(min),
    adcMax(max)
  { ; }

  virtual void service(void) = 0;

  virtual uint16_t read(void)
  {
    return val;
  }

  void setMin(uint16_t min) { adcMin = min; }
  void setMax(uint16_t max) { adcMax = max; }

  uint16_t getMin(void) { return adcMin; }
  uint16_t getMax(void) { return adcMax; }
};


////////////////////////////////////////////////////////////////////////////////////////////
// ADC Channel corresponding to an MCP ADC
//
//  *inADC:    pointer to an instance of an MCP_ADC, e.g. MCP3008, MCP3204, etc.
//  inChannel: which ADC channel to read from
class MCP_Channel : public ADC_Object
{
private:

  uint8_t channel;
  std::shared_ptr<MCP_ADC> pADC;

public:

  virtual void service(void) override
  {
    if ( (pADC == nullptr) || (channel == INVALID_CHANNEL) )
    {
      val = adcMin;
    }
    else
    {
      val = pADC->analogRead(channel);
    }
  }

  MCP_Channel():
      channel(99),
      pADC(nullptr)
  { ; }

  MCP_Channel(MCP_ADC *pADC, uint8_t inChannel = INVALID_CHANNEL):
      pADC(pADC),
      channel(inChannel)
  { ; }

  void setADC(MCP_ADC *pADC) { this->pADC = std::shared_ptr<MCP_ADC>(pADC); }
  void setChannel(uint8_t inChannel)    { channel = inChannel; }

  std::shared_ptr<MCP_ADC> getADC(void) { return pADC; }
  uint8_t getChannel(void)              { return channel; }
};



////////////////////////////////////////////////////////////////////////////////////////////
// ADC Channel corresponding to an ADC-enabled input on the ESP32
//
//  inPin: which ESP32 pin to read
class ESP32_ADC_Channel : public ADC_Object
{
private:
  ESP32AnalogRead ADC;
  uint8_t pin;

public:

  virtual void service() override
  {
    if (pin == INVALID_CHANNEL)
    {
      val = adcMin;
    }
    else
    {
      val = ADC.readRaw();
    }
  }

  ESP32_ADC_Channel():
      pin(INVALID_CHANNEL)
  {
    ADC = ESP32AnalogRead();
  }

  ESP32_ADC_Channel(uint8_t inPin):
      pin(inPin)
  {
    ADC = ESP32AnalogRead(pin);
  }

  void setChannel(uint8_t inChannel)
  {
    pin = inChannel;
    pinMode(pin, INPUT);
    ADC.attach(pin);
  }
};


class SmoothedADC : public ADC_Object
{
protected:
  std::vector<uint16_t> readings;
  std::shared_ptr<ADC_Object> pADC;
  MCP_ADC *pMCP;
  ESP32_ADC_Channel *pESP;
  MCP_Channel *pCHNL;

  uint8_t   buffSize;
  uint16_t  sampleAvg;
  uint64_t  runningSum;
  uint16_t  sampleCount;

public:
  SmoothedADC(ESP32_ADC_Channel *inADC, uint8_t sampleCount = MAX_BUFFER_SIZE):
      buffSize(sampleCount)
  {
    pADC = std::shared_ptr<ESP32_ADC_Channel>(inADC);
  }

  SmoothedADC(MCP_ADC *pMCP, uint8_t inChannel = INVALID_CHANNEL, uint8_t sampleCount = MAX_BUFFER_SIZE):
      pMCP(pMCP),
      buffSize(sampleCount)
  {
    pADC = std::make_shared<MCP_Channel>(pMCP, inChannel);
  }

  SmoothedADC(MCP_Channel *inADC, uint8_t sampleCount = MAX_BUFFER_SIZE):
      buffSize(sampleCount)
  {
    pADC = std::shared_ptr<MCP_Channel>(inADC);
  }

  void reset()
  {
    while (!readings.empty())
    {
      readings.pop_back();
    }
    readings.reserve(buffSize);
    runningSum  = 0;
    sampleCount = 0;
  }

  virtual void service(void) override
  {
    pADC->service();
    if (sampleCount == buffSize)
    {
      runningSum -= *readings.cend();
      readings.pop_back();
    }
    else
    {
      ++sampleCount;
    }
    uint16_t reading = pADC->read();
    runningSum += reading;
    readings.insert(readings.begin(), reading);
  }

  virtual uint16_t read(void) override
  {
    if (sampleCount == 0)
    {
      return pADC->getMin();
    }

    return runningSum / sampleCount;
  }

  void fillBuffer(void)
  {
    while(sampleCount != buffSize)
    {
      service();
    }
  }
};