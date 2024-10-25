#ifndef DAC_CAL_TABLE_DOT_H
#define DAC_CAL_TABLE_DOT_H

#include <Arduino.h>
#include <Adafruit_MCP4728.h>

const uint8_t CAL_TABLE_HIGH_OCTAVE(8);

struct CalTable
{
  CalTable(uint8_t ch_L);

  uint16_t table[CAL_TABLE_HIGH_OCTAVE + 1];
  uint8_t logicalChannel;
  MCP4728_channel_t dacChannel;

  uint16_t valFromNote(uint8_t note) const;
};

#endif