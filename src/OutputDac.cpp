#include "OutputDac.h"
#include <Adafruit_MCP4728.h>

MultiChannelDac::MultiChannelDac(uint8_t numCh):
  NUM_DAC_CHANNELS(numCh),
  ready(false)
{
  DAC.reserve(NUM_DAC_CHANNELS);
}

void MultiChannelDac::setChannelNote(uint8_t channel, uint8_t note)
{
  if (!ready || channel >= NUM_DAC_CHANNELS)
  {
    return;
  }

  *DAC[channel] = (uint16_t)note;
  DAC[channel]->clock();
}

void MultiChannelDac::init()
{
  if (ready) return;

  std::shared_ptr<Adafruit_MCP4728> MCP4728 = std::make_shared<Adafruit_MCP4728>();

  // Set up external
  dbprintln("MCP4728 test...");
  if (!MCP4728->begin(0x64))
  {
    dbprintln("Failed to find MCP4728 chip");
    while (1)
    {
      delay(1);
    }
  }
  dbprintln("MCP4728 chip initialized");

  for (uint8_t ch(0); ch < NUM_DAC_CHANNELS; ++ch)
  {
    channel_ptr newChannel;
    newChannel.reset(new OutputChannel(ch, MCP4728));
    DAC.push_back(newChannel);
  }

  ready = true;
}

uint16_t MultiChannelDac::getChannelVal(uint8_t ch)
{
  return DAC[ch]->out;
}
