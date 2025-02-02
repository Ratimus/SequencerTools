#pragma once

#include <Arduino.h>
#include <MultimodeControl.h>
#include <vector>
#include <freertos/semphr.h>

class ControllerBank
{
  uint8_t currentMode;
  uint8_t controlCount;
  uint8_t modeCount;

  std::vector<MultiModeCtrl> controls;
  std::vector<bool>          locks;
  std::vector<uint16_t>      vals;
  std::vector<uint8_t>       positionMapping;

  SemaphoreHandle_t mutex;
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

  ControllerBank(uint8_t controlCount,
                 uint8_t modeCount):
    currentMode(0),
    controlCount(controlCount),
    modeCount(modeCount),
    mutex(xSemaphoreCreateRecursiveMutex())
  {
    controls.reserve(controlCount);
  }

  // Constructor passing an array of ESP32 pin numbers to use as ADC channels
  //   controlCount: number of physical controls
  //   modeCount:    number of modes / pages / virtual controller scenes
  //   topOfRange:   highest control value that you want to return
  ControllerBank(const uint8_t *pins,
                 uint8_t controlCount,
                 uint8_t modeCount,
                 uint16_t topOfRange):
    currentMode(0),
    controlCount(controlCount),
    modeCount(modeCount),
    mutex(xSemaphoreCreateRecursiveMutex())
  {
    for (uint8_t n(0); n < controlCount; ++n)
    {
      controls.push_back(MultiModeCtrl(std::make_shared<ESP32_ADC_Channel>(pins[n]), modeCount, topOfRange));
      vals.push_back(0);
      locks.push_back(0);
    }
  }

  // Constructor passing a pointer to an MCP ADC
  //   channelCount: number of physical channels (note: starts at 0; may want to allow using only a subset of available channels)
  //   modeCount:    number of modes / pages / virtual controller scenes
  //   topOfRange:   highest control value that you want to return
  ControllerBank(MCP_ADC* pADC,
                 uint8_t channelCount,
                 uint8_t modeCount,
                 uint16_t topOfRange):
    mutex(xSemaphoreCreateRecursiveMutex())
  {
    for (uint8_t n(0); n < channelCount; ++n)
    {
      controls.push_back(MultiModeCtrl(std::make_shared<MCP_Channel>(pADC, n), modeCount, topOfRange));
      vals.push_back(0);
      locks.push_back(0);
    }
  }

  ControllerBank(std::shared_ptr<MCP_ADC>pADC,
                 uint8_t channelCount,
                 uint8_t modeCount,
                 uint16_t topOfRange):
    mutex(xSemaphoreCreateRecursiveMutex())
  {
    for (uint8_t n(0); n < channelCount; ++n)
    {
      controls.push_back(MultiModeCtrl(std::make_shared<MCP_Channel>(pADC, n), modeCount, topOfRange));
      vals.push_back(0);
      locks.push_back(0);
    }
  }


  ControllerBank(const uint8_t clock,
                 const uint8_t miso,
                 const uint8_t mosi,
                 const uint8_t cs,
                 uint8_t controlCount,
                 uint8_t resolution,
                 uint8_t modeCount,
                 uint16_t topOfRange):
    mutex(xSemaphoreCreateRecursiveMutex())
  {
    std::shared_ptr<MCP_ADC>pADC(nullptr);

    switch (controlCount)
    {
      case 1:
      {
        if (resolution == 10)
        {
          pADC = std::make_shared<MCP3001>();
        }
        else if (resolution == 12)
        {
          pADC = std::make_shared<MCP3201>();
        }
        break;
      }

      case 2:
      {
        if (resolution == 10)
        {
          pADC = std::make_shared<MCP3002>();
        }
        else if (resolution == 12)
        {
          pADC = std::make_shared<MCP3202>();

        }
        break;
      }

      case 4:
      {
        if (resolution == 10)
        {
          pADC = std::make_shared<MCP3004>();

        }
        else if (resolution == 12)
        {
          pADC = std::make_shared<MCP3204>();

        }
        break;
      }

      case 8:
      {
        if (resolution == 10)
        {
          pADC = std::make_shared<MCP3008>();

        }
        else if (resolution == 12)
        {
          pADC = std::make_shared<MCP3208>();

        }
        break;
      }

      default:
        break;
    }

    assert(pADC != nullptr);
    pADC->setGPIOpins(clock, miso, mosi, cs);
    this->modeCount    = modeCount;
    this->controlCount = controlCount;

    for (uint8_t n(0); n < controlCount; ++n)
    {
      controls.push_back(MultiModeCtrl(std::make_shared<MCP_Channel>(pADC, n), modeCount, topOfRange));
      vals.push_back(0);
      locks.push_back(0);
    }
  }

  void init(const uint8_t *pins,
            uint16_t topOfRange)
  {
    for (uint8_t n(0); n < controlCount; ++n)
    {
      pinMode(pins[n], INPUT);
      auto pESP_ADC = std::make_shared<ESP32_ADC_Channel>(pins[n]);
      controls.push_back(MultiModeCtrl(pESP_ADC, modeCount, topOfRange));
      vals.push_back(0);
      locks.push_back(0);
    }
  }

  // Pass an array containing the indices of the hardware elements you want in the order you want to access them,
  // e.g. if you have controls A, B, and C, corresponding to ADC channels 7, 0, and 2, you'd pass {7, 0, 3}. Then,
  // when you read Control[0], it will read channel 7, Control[1] -> channel 0, and Control[2] -> channel 3
  void setControlPositionMapping(uint8_t *mapping)
  {
    while (!positionMapping.empty())
    {
      positionMapping.pop_back();
    }

    for (uint8_t n(0); n < controlCount; ++n)
    {
      positionMapping.push_back(mapping[n]);
    }
  }

  uint8_t getPositionMappedIndex(uint8_t controlIdx)
  {
    if (positionMapping.empty())
    {
      return controlIdx;
    }

    return positionMapping[controlIdx];
  }

  std::shared_ptr<ControlObject> getPtr(uint8_t controlIdx)
  {
    return controls[getPositionMappedIndex(controlIdx)].getPtr();
  }

  void saveScene(void)
  {
    selectScene(currentMode);
  }

  void selectScene(uint8_t sceneIdx, bool reqUnlock = true)
  {
    lock();
    currentMode = sceneIdx;
    for (uint8_t n(0); n < controlCount; ++n)
    {
      controls[getPositionMappedIndex(n)].selectMode(currentMode, reqUnlock);
    }
    unlock();
  }

  void service()
  {
    lock();
    for (uint8_t n(0); n < controlCount; ++n)
    {
      getPtr(n)->service();
    }
    unlock();
  }

  // Call this once to update all the read() values. Saves a bunch of mutex calls.
  void readAll(uint16_t *getVals = nullptr, bool *getLocks = nullptr)
  {
    lock();
    for (uint8_t n = 0; n < controlCount; ++n)
    {
      vals[n] = getPtr(n)->read();
      if (getVals)
      {
        getVals[n] = vals[n];
      }

      locks[n] = (getPtr(n)->getLockState() != STATE_UNLOCKED);
      if (getLocks)
      {
        getLocks[n] = locks[n];
      }
    }
    unlock();
  }

  uint16_t read(uint8_t controlIdx)
  {
    lock();
    uint16_t ret = vals[controlIdx];
    unlock();
    return ret;
  }

  bool isLocked(uint8_t controlIdx)
  {
    lock();
    bool ret = locks[controlIdx];
    unlock();
    return ret;
  }
};
