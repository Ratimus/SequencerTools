#include "OutputChannel.h"
#include <RatFuncs.h>

// DAC channels (maps indices to actual channels)
const uint8_t NUM_DAC_CHANNELS(4);

// // Cal tables: for each HW channel of the DAC, these are the raw values you need for octaves 0 - 8
static const MCP4728_channel_t DAC_CH[] {MCP4728_CHANNEL_D, MCP4728_CHANNEL_B, MCP4728_CHANNEL_A, MCP4728_CHANNEL_C};

uint16_t calvals[4][9]
{
  {0, 394, 804, 1214, 1625, 2036, 2454, 2862, 3270}, // 410 410 411 411 418 408 408 - MCP4728_CHANNEL_D
  {0, 408, 818, 1230, 1638, 2049, 2470, 2882, 3285}, // 410 412 408 411 421 412 413 - MCP4728_CHANNEL_B
  {0, 393, 802, 1217, 1627, 2041, 2459, 2869, 3279}, // 409 415 410 414 418 410 410 - MCP4728_CHANNEL_A
  {0, 400, 811, 1224, 1631, 2041, 2459, 2872, 3280}, // 411 413 407 410 418 413 408 - MCP4728_CHANNEL_C
};

CalTable calTables[4]
{
  CalTable(calvals[0], 0, MCP4728_CHANNEL_D),
  CalTable(calvals[1], 1, MCP4728_CHANNEL_B),
  CalTable(calvals[2], 2, MCP4728_CHANNEL_A),
  CalTable(calvals[3], 3, MCP4728_CHANNEL_C)
};


OutputChannel::OutputChannel(uint8_t ch, dac_ptr pDac /*=nullptr*/):
  latchable<uint16_t>((uint16_t)0),
  calVals(calTables[ch]),
  MCP(pDac)
{
  if (MCP != nullptr)
  {
    clockIn(0);
  }
}


// Sets up a note to be written to DAC (which it will write when clocked)
uint16_t  OutputChannel::set(uint16_t note)
{
  uint16_t nextVal(valFromNote((uint16_t)note));
  latchable<uint16_t>::set(nextVal);
  return note;
}


// Latches the raw value corresponding to its note and writes it to the DAC
uint16_t OutputChannel::clock()
{
  uint16_t setVal(latchable<uint16_t>::clock());
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


// Translates note values to raw DAC outputs using calibration tables
uint16_t OutputChannel::valFromNote(uint8_t note) const
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
    octUp = calVals.table[octave + 1];
    octDn = calVals.table[octave];
  }
  else
  {
    octUp = calVals.table[CAL_TABLE_HIGH_OCTAVE];
    octDn = calVals.table[CAL_TABLE_HIGH_OCTAVE - 1];
  }

  // Look Ma, no floats!!!
  uint16_t inc((octUp - octDn) / 12);

  // Determine which semitone we want within the current octave, then multiply that
  // by the DAC value of a semitone. Add it to the DAC value for the octave and we've
  // got our output value.
  uint16_t semiTone(note - (octave * 12));
  uint16_t setVal(calVals.table[octave] + inc * semiTone);
  if (setVal > 4095)
  {
    setVal = 4095;
  }
  return setVal;
}


//=====================================================================================
// TODO: CALIBRATION STUFF
  // SET LED MAPPING:
  // write a 1 to bit 7
  // move encoder until bit 0 is lit
  // click to move on
  // bit 0 stays lit, bit 7 is lit
  // move encoder until bit 1 is lit
  // etc.
  // . . . do this for all three LED registers

  // SET DAC OUTPUT CHANNEL MAPPING:
  // for output in UserOutputs:
  //   outCh = 0;
  //   Plug something in to output
  //   Click the encoder until the pitch goes up
  //
  //   if encoder right
  //     ++outCh;
  //   else if encoder left
  //     -- outCh;
  //
  //   if write
  //     DAC_CH[output] = outCh;

  // SET FADER MAPPING
  // set all faders to zero
  // move fader 0 to max until it lights up,
  // move fader 1 to max,
  // etc.

// // Allows you to fine-tune the output of each individual DAC channel using all eight faders.
// // Save the values you get and use them to populate the calibration data in TMOC_HW.h
// void calibrate()
// {
//   uint8_t selch(0);
//   uint8_t selreg(1);
//   leds.tempWrite(1, 0);
//   leds.tempWrite(1, 1);

//   uint16_t outval(0);
//   faderBank[selch]->selectActiveBank(0);
//   dbprintf("ch: %u\n", selch);
//   while (true)
//   {
//     if (writeHigh.readAndFree())
//     {
//       writeRawValToDac(selch, 0);
//       ++selch;

//       if (selch == 4)
//       {
//         selch = 0;
//       }

//       selreg = 0 | (1 << selch);
//       faderBank[selch]->selectActiveBank(0);
//       leds.tempWrite(selreg, 0);
//       leds.tempWrite(selreg, 1);
//       dbprintf("ch: %u\n", selch);
//     }

//     outval = 0;
//     for (uint8_t bn(0); bn < 8; ++bn)
//     {
//       outval += faderBank[sliderMap[bn]]->read() >> (1 + bn);
//     }
//     if (outval > 4095)
//     {
//       outval = 4095;
//     }
//     output.setChannelVal(selch, outval);
//     if (writeLow.readAndFree())
//     {
//       dbprintf("%u\n", outval);
//     }
//   }
// }
