#include "OutputChannel.h"
#include <RatFuncs.h>


OutputChannel::OutputChannel(uint8_t ch, dac_ptr pDac /*=nullptr*/):
  latchable<uint16_t>((uint16_t)0),
  calVals(CalTable(ch)),
  MCP(pDac)
{
  if (MCP != nullptr)
  {
    clockIn(0);
  }
}


// Sets up a note to be written to DAC (which it will write when clocked)
uint16_t OutputChannel::set(uint16_t note)
{
  if (note > 255)
  {
    return 0;
  }

  uint16_t nextVal = calVals.valFromNote((uint16_t)note);
  latchable<uint16_t>::set(nextVal);
  return note;
}


// Latches the raw value corresponding to its note and writes it to the DAC
uint16_t OutputChannel::clock()
{
  uint16_t setVal = latchable<uint16_t>::clock();
  if (MCP == nullptr)
  {
    dbprintf("OutputChannel %u DAC is a nullptr!\n", calVals.logicalChannel);
    while (1) {;}
  }

  MCP->setChannelValue( calVals.dacChannel,
                        setVal,
                        MCP4728_VREF_INTERNAL,
                        MCP4728_GAIN_2X);
  return setVal;
}

