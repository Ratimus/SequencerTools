#include "DAC_CalTable.h"

// DAC channels (maps indices to actual channels)
static const uint8_t NUM_DAC_CHANNELS(4);

// // Cal tables: for each HW channel of the DAC, these are the raw values you need for octaves 0 - 8
static MCP4728_channel_t DAC_CH[NUM_DAC_CHANNELS]
{
  MCP4728_CHANNEL_D,
  MCP4728_CHANNEL_B,
  MCP4728_CHANNEL_A,
  MCP4728_CHANNEL_C
};

static uint16_t calvals[4][9]
{
  {0, 394, 804, 1214, 1625, 2036, 2454, 2862, 3270}, // 410 410 411 411 418 408 408 - MCP4728_CHANNEL_D
  {0, 408, 818, 1230, 1638, 2049, 2470, 2882, 3285}, // 410 412 408 411 421 412 413 - MCP4728_CHANNEL_B
  {0, 393, 802, 1217, 1627, 2041, 2459, 2869, 3279}, // 409 415 410 414 418 410 410 - MCP4728_CHANNEL_A
  {0, 400, 811, 1224, 1631, 2041, 2459, 2872, 3280}, // 411 413 407 410 418 413 408 - MCP4728_CHANNEL_C
};


 // Translates note values to raw DAC outputs using calibration tables
uint16_t CalTable::valFromNote(uint8_t note) const
{
  // Get the octave from the absolute note number
  uint8_t octave(note / 12);
  if (octave > CAL_TABLE_HIGH_OCTAVE)
  {
    octave = CAL_TABLE_HIGH_OCTAVE;
  }

  // Use the calibration table to determine how much you'd need to add to the DAC value
  // in order to go up an octave from the current octave. Divide that by 12 to get the
  // value of a semitone within the current octave.
  uint16_t octUp;
  uint16_t octDn;
  if (octave < CAL_TABLE_HIGH_OCTAVE)
  {
    octUp = table[octave + 1];
    octDn = table[octave];
  }
  else
  {
    octUp = table[CAL_TABLE_HIGH_OCTAVE];
    octDn = table[CAL_TABLE_HIGH_OCTAVE - 1];
  }

  // Look Ma, no floats!!!
  uint16_t inc = (octUp - octDn) / 12;

  // Determine which semitone we want within the current octave, then multiply that
  // by the DAC value of a semitone. Add it to the DAC value for the octave and we've
  // got our output value.
  uint16_t semiTone = note - (octave * 12);
  uint16_t setVal   = table[octave] + (inc * semiTone);
  if (setVal > 4095)
  {
    setVal = 4095;
  }
  return setVal;
}

  CalTable::CalTable(uint8_t ch_L):
    logicalChannel(ch_L),
    dacChannel(DAC_CH[logicalChannel])
  {
    for (uint8_t n(0); n < CAL_TABLE_HIGH_OCTAVE + 1; ++n)
    {
      table[n] = calvals[ch_L][n];
    }
  }