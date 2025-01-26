////////////////////////////////////////////////////////////////////////////////////////////
//
// Hardware abstraction class and hardware-specific derived classes for a single ADC channel
//
#pragma once

#include <Arduino.h>
#include "MCP_ADC.h"
#include "ESP32AnalogRead.h"
#include <memory>
#include <list>
#include <freertos/semphr.h>

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
  SemaphoreHandle_t mutex;

  uint16_t adcMin;
  uint16_t adcMax;

  static inline const TickType_t PATIENCE = 10;

  bool lock()
  {
    if (xSemaphoreTakeRecursive(mutex, PATIENCE) != pdTRUE)
    {
      return false;
    }

    return true;
  }

  void unlock()
  {
    xSemaphoreGiveRecursive(mutex);
  }

public:

  inline static const uint8_t INVALID_CHANNEL = 99;

  ADC_Object():
      ADC_Object(0, 4095)
  { ; }

  ADC_Object(uint16_t min, uint16_t max):
      adcMin(min),
      adcMax(max),
      mutex(xSemaphoreCreateRecursiveMutex())
  { ; }

  virtual void service(void) = 0;
  virtual uint16_t read(void) = 0;

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
  volatile uint16_t rawVal;

public:

  virtual void service(void) override
  {
    if (pdTRUE != lock())
    {
      Serial.println("MCH svc semtake failed");
      while (1);
    }

    if ( (pADC == nullptr) || (channel == INVALID_CHANNEL) )
    {
      rawVal = adcMin;
    }
    else
    {
      rawVal = pADC->analogRead(channel);
    }
    unlock();
  }

  MCP_Channel():
      channel(99),
      pADC(nullptr)
  { ; }

  MCP_Channel(MCP_ADC *pADC,
              uint8_t inChannel = INVALID_CHANNEL):
      pADC(std::shared_ptr<MCP_ADC>(pADC)),
      channel(inChannel)
  { ; }

  MCP_Channel(std::shared_ptr<MCP_ADC>pADC,
              uint8_t inChannel = INVALID_CHANNEL):
      pADC(pADC),
      channel(inChannel)
  { ; }

  void setADC(MCP_ADC *pADC) { this->pADC = std::shared_ptr<MCP_ADC>(pADC); }
  void setChannel(uint8_t inChannel)    { channel = inChannel; }

  std::shared_ptr<MCP_ADC> getADC(void) { return pADC; }
  uint8_t getChannel(void)              { return channel; }

  virtual uint16_t read(void) override
  {
    if (pdTRUE != lock())
    {
      Serial.println("MCH read semtake failed");
      while (1);
    }

    uint16_t ret = rawVal;
    unlock();
    return ret;
  }
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

  volatile uint16_t rawVal;

public:
  ESP32_ADC_Channel():
      ESP32_ADC_Channel(INVALID_CHANNEL)
  {
    ;
  }

  ESP32_ADC_Channel(uint8_t inPin):
      ADC_Object()
  {
    ADC = ESP32AnalogRead();
    rawVal = 0;
    this->attach(inPin);
  }

  virtual void service() override
  {
    if (pdTRUE != lock())
    {
      Serial.println("e32 svc semtake failed");
      while (1);
    }

    if (pin == INVALID_CHANNEL)
    {
      rawVal = adcMin;
    }
    else
    {
      rawVal = ADC.readRaw();
    }
    unlock();
  }

  virtual uint16_t read() override
  {
    if (pdTRUE != lock())
    {
      Serial.println("e32 read semtake failed");
      while (1);
    }

    uint16_t ret = rawVal;
    unlock();
    return ret;
  }

  void attach(uint8_t inChannel)
  {
    pin = inChannel;
    if (pin == INVALID_CHANNEL)
    {
      return;
    }
    pinMode(pin, INPUT_PULLDOWN);
    ADC.attach(pin);
  }
};


class SmoothedADC : public ADC_Object
{
protected:
  std::array<volatile uint16_t, MAX_BUFFER_SIZE> readings;
  std::shared_ptr<ADC_Object> pADC;

  uint8_t   buffSize;
  volatile uint8_t   writeIndex;
  volatile int64_t   runningSum;
  volatile uint16_t  sampleCount;

public:
  SmoothedADC(std::shared_ptr<ADC_Object>inADC,
              uint8_t buffSize = MAX_BUFFER_SIZE):
      buffSize(buffSize),
      pADC(inADC)
  {
    reset();
  }

  SmoothedADC(ESP32_ADC_Channel *inADC,
              uint8_t buffSize = MAX_BUFFER_SIZE):
      buffSize(buffSize)
  {
    pADC = std::shared_ptr<ESP32_ADC_Channel>(inADC);
    reset();
  }

  SmoothedADC(std::shared_ptr<ESP32_ADC_Channel>inADC,
              uint8_t buffSize = MAX_BUFFER_SIZE):
      buffSize(buffSize),
      pADC(inADC)
  {
    reset();
  }

  SmoothedADC(MCP_Channel *inADC,
              uint8_t buffSize = MAX_BUFFER_SIZE):
      buffSize(buffSize)
  {
    pADC = std::shared_ptr<MCP_Channel>(inADC);
    reset();
  }

  SmoothedADC(std::shared_ptr<MCP_Channel>inADC,
              uint8_t buffSize = MAX_BUFFER_SIZE):
      buffSize(buffSize),
      pADC(inADC)
  {
    reset();
  }

  void reset()
  {
    if (pdTRUE != lock())
    {
      Serial.println("smth reset semtake failed");
      while (1);
    }

    memset(&readings, 0, sizeof(uint16_t) * buffSize);
    runningSum  = 0;
    sampleCount = 0;
    writeIndex = 0;
    unlock();
  }

  virtual void service(void) override
  {
    pADC->service();
    uint16_t newestReading = pADC->read();

    if (pdTRUE != lock())
    {
      Serial.println("smth svc semtake failed");
      while (1);
    }

    runningSum += newestReading;
    if (sampleCount == buffSize)
    {
      runningSum -= readings[writeIndex];
      readings[writeIndex] = newestReading;
      ++writeIndex;
      if (writeIndex == buffSize)
      {
        writeIndex = 0;
      }
    }
    else
    {
      readings[sampleCount] = newestReading;
      ++sampleCount;
    }
    unlock();
  }

  virtual uint16_t read(void) override
  {
    if (sampleCount == 0)
    {
      return pADC->getMin();
    }

    if (pdTRUE != lock())
    {
      Serial.println("smth read semtake failed");
      while (1);
    }

    uint16_t ret = (uint16_t)(runningSum / (uint64_t)sampleCount);
    unlock();
    return ret;
  }

  void fillBuffer(void)
  {
    while(sampleCount != buffSize)
    {
      service();
    }
  }
};