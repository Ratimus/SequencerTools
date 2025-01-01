#pragma once

#include <Arduino.h>
#include <MultimodeControl.h>
#include <vector>


class ControllerBank
{
  uint8_t currentMode;
  uint8_t controlCount;
  uint8_t modeCount;

  std::vector<MultiModeCtrl> controls;
  std::vector<uint8_t> positionMapping;

public:

  // Constructor passing an array of ESP32 pin numbers to use as ADC channels
  //   controlCount: number of physical controls
  //   modeCount:    number of modes / pages / virtual controller scenes
  //   topOfRange:   highest control value that you want to return
  ControllerBank(uint8_t *pins,
                 uint8_t controlCount,
                 uint8_t modeCount,
                 uint8_t topOfRange)
  {
    for (uint8_t n(0); n < controlCount; ++n)
    {
      controls.push_back(MultiModeCtrl(std::make_shared<ESP32_ADC_Channel>(pins[n]), modeCount, topOfRange));
    }
  }

  // Constructor passing a pointer to an MCP ADC
  //   channelCount: number of physical channels (note: starts at 0; may want to allow using only a subset of available channels)
  //   modeCount:    number of modes / pages / virtual controller scenes
  //   topOfRange:   highest control value that you want to return
  ControllerBank(MCP_ADC* pADC, uint8_t channelCount, uint8_t modeCount, uint8_t topOfRange)
  {
    for (uint8_t n(0); n < channelCount; ++n)
    {
      controls.push_back(MultiModeCtrl(std::make_shared<MCP_Channel>(pADC, n), modeCount, topOfRange));
    }
  }

  ControllerBank(std::shared_ptr<MCP_ADC>pADC, uint8_t channelCount, uint8_t modeCount, uint8_t topOfRange)
  {
    for (uint8_t n(0); n < channelCount; ++n)
    {
      controls.push_back(MultiModeCtrl(std::make_shared<MCP_Channel>(pADC, n), modeCount, topOfRange));
    }
  }


  ControllerBank(const uint8_t clock,
                 const uint8_t miso,
                 const uint8_t mosi,
                 const uint8_t cs,
                 uint8_t controlCount,
                 uint8_t resolution,
                 uint8_t modeCount,
                 uint8_t topOfRange)
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
    ControllerBank(pADC, controlCount, modeCount, topOfRange);
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
    for (uint8_t n(0); n < controlCount; ++n)
    {
      controls[getPositionMappedIndex(n)].lock();
    }
  }

  void selectScene(uint8_t sceneIdx)
  {
    saveScene();
    currentMode = sceneIdx;
    for (uint8_t n(0); n < controlCount; ++n)
    {
      controls[getPositionMappedIndex(n)].selectMode(currentMode);
    }
  }

  void service()
  {
    for (auto &ctrl: controls)
    {
      ctrl.service();
    }
  }


  uint16_t read(uint8_t controlIdx)
  {
    return controls[getPositionMappedIndex(controlIdx)].read();
  }

  void reqUnlock(int8_t controlIdx = -1)
  {
    if (controlIdx >= 0)
    {
      controls[getPositionMappedIndex(controlIdx)].reqUnlock();
    }
    else
    {
      for (auto &ctrl: controls)
      {
        ctrl.reqUnlock();
      }
    }
  }

  bool isLocked(uint8_t controlIdx)
  {
    return (controls[getPositionMappedIndex(controlIdx)].getLockState() == STATE_UNLOCKED);
  }
};
//   void init(const uint8_t SPI_DATA_OUT, const uint8_t SPI_DATA_IN, const uint8_t SPI_CLK, const uint8_t ADC_CS);
//   void init(uint8_t *pins, uint8_t pinCount);


// ControllerBank::ControllerBank(ControllerBank & proto):
//   NUM_FADERS(proto.NUM_FADERS),
//   NUM_BANKS(proto.NUM_BANKS),
//   bankIdx(proto.bankIdx),
//   ONE_OVER_ADC_MAX(proto.ONE_OVER_ADC_MAX)
// {
//   std::copy(proto.sliderMap.begin(), proto.sliderMap.end(), std::back_insert_iterator(this->sliderMap));

//   for (auto fader: proto.faderBank)
//   {
//     std::copy(proto.faderBank.begin(), proto.faderBank.end(), std::back_insert_iterator(this->faderBank));
//   }
// }

// ControllerBank::ControllerBank(uint8_t numFaders,
//                                uint8_t numBanks,
//                                const uint8_t sliderMapping[]):
//   NUM_FADERS(numFaders),
//   NUM_BANKS(numBanks)
// {
//   faderBank.reserve(NUM_BANKS);
//   for (auto n = 0; n < NUM_FADERS; ++n)
//   {
//     sliderMap.push_back(sliderMapping[n]);
//   }
// }

// void ControllerBank::init(MCP3208 *pMCP3208)
// {
//   for (uint8_t ch(0); ch < NUM_FADERS; ++ch)
//   {
//     faderBank.push_back(std::make_shared<MultiModeCtrl>(new MCP_Channel(pMCP3208, ch, 12),
//                                                         NUM_BANKS));
//     faderBank[ch]->setDefaults();
//     faderBank[ch]->saveActiveCtrl(NUM_FADERS - 1 - ch);
//     dbprintf("Fader %u initialized\n", ch);
//   }
//   ONE_OVER_ADC_MAX = 1.0f / faderBank[0]->getMax();
// }