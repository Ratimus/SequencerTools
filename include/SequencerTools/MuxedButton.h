#pragma once

#include <Arduino.h>

#include <SequencerTools/MagicButton.h>
#include <SequencerTools/CD4067.h>

class MuxedButton : public MagicButton
{
  MultiMux * pMux = NULL;
  int8_t     mux_index;

public:
  int8_t pin_index;

  MuxedButton():
    MagicButton(-1, true, true),
    pMux(nullptr),
    mux_index(-1),
    pin_index(-1)
  { ; }

  MuxedButton(MultiMux *mux,
              uint8_t pin_index,
              uint8_t mux_index = 0):
    MagicButton(-1, true, true),
    pMux(mux),
    mux_index(mux_index),
    pin_index(pin_index)
  {
    pMux->add_digital_pin(pin_index, mux_index);
  }

  // NOTE: THIS EXPECTS THE MULTIMUX TO ALREADY HAVE A CD4067 at [mux_idx]!!!
  void init(MultiMux *mux,
            uint8_t pin,
            uint8_t mux_idx = 0)
  {
    pMux      = mux;
    pin_index = pin;
    mux_index = mux_idx;
    pMux->add_digital_pin(pin_index, mux_index);
  }

  virtual bool IRAM_ATTR readPin(void) override
  {
    // Check if pin is not valid
    if (!pMux || (pin_index == -1))
    {
      return false;
    }

    return !pMux->get_val(pin_index, mux_index);
  }
};
