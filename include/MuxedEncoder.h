#include <ClickEncoder.h>
#include <MuxedButton.h>
#include <CD4067.h>


class MuxedEncoder : public ClickEncoder
{
protected:

  std::shared_ptr<MultiMux> pMux;
  uint8_t pinA;
  uint8_t pinB;
  uint8_t mux_index;

  virtual bool readB() override
  {
    return (bool)pMux->get_val(pinB, mux_index);
  }

  virtual bool readA() override
  {
    return (bool)pMux->get_val(pinA, mux_index);
  }

public:

  MuxedEncoder(MultiMux *mux,
               const uint8_t * const pinNums,
               uint8_t stepsPerNotch,
               uint8_t mux_index = 0):
    ClickEncoder(-1, -1, -1, stepsPerNotch, true),
    pinA(pinNums[0]),
    pinB(pinNums[1]),
    mux_index(mux_index)
  {
      hwButton = std::make_unique<MuxedButton>(mux, pinNums[2], mux_index);
      init();
  }

  void init()
  {
    MSB = (long)readA();
    LSB = (long)readB();
    hwButton->service();
  }
};
