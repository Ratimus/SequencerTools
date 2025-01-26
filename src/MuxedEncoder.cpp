#include <MuxedEncoder.h>


MuxedEncoder::MuxedEncoder(const uint8_t * const pinNums,
                           uint8_t stepsPerNotch):
  ClickEncoder(-1, -1, -1, stepsPerNotch, true),
  _BITMASK{uint16_t((uint16_t)1 << pinNums[0]), uint16_t((uint16_t)1 << pinNums[1])}
{
  hwButton = std::make_shared<MuxedButton>(pinNums[2]);
}


bool MuxedEncoder::readB()
{
  return (_REGISTER & _BITMASK[0]);
}


bool MuxedEncoder::readA()
{
  return (_REGISTER & _BITMASK[1]);
}


void MuxedEncoder::init()
{
  MSB = (long)readA();
  LSB = (long)readB();
  hwButton->service();
}


void MuxedEncoder::updateReg()
{
  _SHARED_MUX->service();
}


void MuxedEncoder::service()
{
  _REGISTER = _SHARED_MUX->getReg();
  ClickEncoder::service();
}


void MuxedEncoder::setMux(HW_Mux *pMux)
{
  _SHARED_MUX = std::shared_ptr<HW_Mux>(pMux);
}
