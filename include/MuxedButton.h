#pragma once

#include <Arduino.h>

#include <MagicButton.h>
#include <CD4067.h>

class MuxedButton : public MagicButton
{
  std::shared_ptr<MultiMux> pMux = NULL;
  int8_t                    mux_index;

public:
  int8_t pin_index;

  MuxedButton()
      : MagicButton(-1, true, true), pMux(std::shared_ptr<MultiMux>(0)),
        mux_index(-1), pin_index(-1)
  {
    ;
  }

  MuxedButton(MultiMux *mux, uint8_t pin_index, uint8_t mux_index = 0)
      : MagicButton(-1, true, true), pMux(std::shared_ptr<MultiMux>(mux)),
        mux_index(mux_index), pin_index(pin_index)
  {
    pMux->add_digital_pin(pin_index, mux_index);
  }

  // NOTE: THIS EXPECTS THE MULTIMUX TO ALREADY HAVE A CD4067 at [mux_idx]!!!
  void init(MultiMux *mux, uint8_t pin, uint8_t mux_idx = 0)
  {
    pMux      = std::shared_ptr<MultiMux>(mux);
    pin_index = pin;
    mux_index = mux_idx;
    pMux->add_digital_pin(pin_index, mux_index);
  }

  virtual bool readPin(void) override
  {
    // Check if pin is not valid
    if (!pMux || (pin_index == -1))
    {
      return false;
    }

    return (bool)pMux->get_val(pin_index, mux_index);
  }
};
