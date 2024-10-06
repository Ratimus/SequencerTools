#ifndef MULTICHANNEL_DAC
#define MULTICHANNEL_DAC

#include <Arduino.h>
#include <Adafruit_MCP4728.h>
#include <RatFuncs.h>
#include <vector>
#include "OutputChannel.h"
#include <memory>


class MultiChannelDac
{
  std::vector<channel_ptr> DAC;
  const uint8_t NUM_DAC_CHANNELS;
  bool ready;

public:

  MultiChannelDac(uint8_t numCh);

  void setChannelNote(uint8_t channel, uint8_t note);

  void init();

  uint16_t getChannelVal(uint8_t ch);
};

#endif


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
