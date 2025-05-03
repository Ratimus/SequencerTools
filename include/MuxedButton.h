#include <MagicButton.h>
#include <CD4067.h>


class MuxedButton : public MagicButton
{
  std::shared_ptr<MultiMux> pMux = NULL;
  uint8_t pin_index;
  uint8_t mux_index;

public:

  MuxedButton():
    MagicButton(-1, true, true),
    pMux(std::shared_ptr<MultiMux>(0))
  { ; }

  MuxedButton(MultiMux *mux, uint8_t pin_index, uint8_t mux_index = 0):
    MagicButton(-1, true, true),
    pMux(std::shared_ptr<MultiMux>(pMux))
  {
    pMux->add_digital_pin(pin_index, mux_index);
  }

  // NOTE: THIS EXPECTS THE MULTIMUX TO ALREADY HAVE A CD4067 at [mux_idx]!!!
  void init(MultiMux *mux, uint8_t pin, uint8_t mux_idx = 0)
  {
    pMux = std::shared_ptr<MultiMux>(mux);
    pin_index = pin;
    mux_index = mux_idx;
    pMux->add_digital_pin(pin_index, mux_index);
  }

  virtual bool readPin(void) override
  {
    if (!pMux)
    {
      return false;
    }

    return (bool)pMux->get_val(pin_index, mux_index);
  }
};