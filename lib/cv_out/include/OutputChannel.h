#pragma once

#include <Arduino.h>
#include <Adafruit_MCP4728.h>
#include "Latchable.h"
#include <memory>
#include "DAC_CalTable.h"

typedef std::shared_ptr<Adafruit_MCP4728> dac_ptr;

// This class abstracts a single output channel of a DAC, allowing you to
// pre-enable note values and update the DAC with the raw value corresponding
// to that note when clocked. Individual channels can have their own unique
// calibration tables to improve accuracy
class OutputChannel
{
  const CalTable calVals;
  dac_ptr MCP;

public:

  latchable<uint16_t> latch;  // TODO: I changed latchable and modified this so other stuff would compile
  OutputChannel(uint8_t ch, dac_ptr pDac = nullptr);
  void setDacPointer(dac_ptr pDac) { MCP = pDac; }
  uint16_t set_input(uint16_t note);
  uint16_t clock();
  uint16_t clock_in(uint16_t val);
  uint16_t operator=(uint16_t val) { return set_input(val); }
};

typedef std::shared_ptr<OutputChannel> channel_ptr;
