#ifndef OUTPUT_CHANNEL_H
#define OUTPUT_CHANNEL_H

#include <Arduino.h>
#include <Adafruit_MCP4728.h>
#include <Latchable.h>
#include <memory>


const uint8_t CAL_TABLE_HIGH_OCTAVE(8);

struct CalTable
{
  uint16_t table[CAL_TABLE_HIGH_OCTAVE + 1];
  uint8_t logicalChannel;
  MCP4728_channel_t dacChannel;
};

typedef std::shared_ptr<Adafruit_MCP4728> dac_ptr;


// This class abstracts a single output channel of a DAC, allowing you to
// pre-enable note values and update the DAC with the raw value corresponding
// to that note when clocked. Individual channels can have their own unique
// calibration tables to imporove accuracy
class OutputChannel : public latchable<uint16_t>
{
  dac_ptr MCP;
  const CalTable calVals;
  uint16_t valFromNote(uint8_t note) const;

public:

  OutputChannel(uint8_t ch, dac_ptr pDac = nullptr);
  void setDacPointer(dac_ptr pDac) { MCP = pDac; }
  virtual void set(uint16_t note) override;
  virtual uint16_t clock() override;
};

typedef std::shared_ptr<OutputChannel> channel_ptr;


#endif