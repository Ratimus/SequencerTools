#pragma once

#include <Arduino.h>
#include <ClickEncoder.h>
#include <MuxedButton.h>
#include <CD4067.h>


class MuxedEncoder : public ClickEncoder
{
protected:

  std::shared_ptr<MultiMux> pMux;
  int8_t pinA;
  int8_t pinB;
  int8_t mux_index;

  virtual bool readB() override
  {
    assert (pMux);
    return (bool)pMux->get_val(pinB, mux_index);
  }

  virtual bool readA() override
  {
    assert (pMux);
    return (bool)pMux->get_val(pinA, mux_index);
  }

public:

  MuxedEncoder():
    ClickEncoder(-1, -1, -1, 4, true)
  {
    pMux      = std::shared_ptr<MultiMux>(nullptr);
    hwButton  = std::unique_ptr<MuxedButton>(nullptr);
    pinA      = -1;
    pinB      = -1;
    mux_index = -1;
  }

  MuxedEncoder(MultiMux *mux,
               const uint8_t * const pinNums,
               int8_t stepsPerNotch,
               int8_t mux_index = 0):
    ClickEncoder(-1, -1, -1, stepsPerNotch, true),
    pinA((int8_t)pinNums[0]),
    pinB((int8_t)pinNums[1]),
    mux_index(mux_index)
  {
    pMux     = std::shared_ptr<MultiMux>(mux);
    hwButton = std::make_unique<MuxedButton>(mux, pinNums[2], mux_index);
    init_vals();
  }

  void init_vals()
  {
    assert (hwButton);
    assert (pMux);
    MSB = (long)readA();
    LSB = (long)readB();
    hwButton->service();
  }

  void init_hw(MultiMux *mux,
               const uint8_t * const pinNums,
               int8_t stepsPerNotch,
               int8_t mux_index = 0)
  {
    pMux = std::shared_ptr<MultiMux>(mux);
    pinA = ((int8_t)pinNums[0]);
    pinB = ((int8_t)pinNums[1]);
    this->mux_index = mux_index;
    pMux->add_digital_pin(pinA, mux_index);
    pMux->add_digital_pin(pinB, mux_index);
    hwButton = std::make_unique<MuxedButton>(mux, pinNums[2], mux_index);
    init_vals();
  }
};
