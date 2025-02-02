#include "ClickEncoder.h"
#include <DirectIO.h>


class MuxedEncoder : public ClickEncoder
{
protected:

  static inline uint16_t _REGISTER = 0;
  static inline std::shared_ptr<HW_Mux> _SHARED_MUX = NULL;

  const  uint16_t _BITMASK[2];

  virtual bool readA() override;
  virtual bool readB() override;

public:

  MuxedEncoder(const uint8_t * const pinNums,
               uint8_t stepsPerNotch);

  static void setMux(HW_Mux *pMux);

  void init();

  // Call this before servicing individual instances of the class
  // TODO: since other classes (e.g. MuxedButton) may be on the same mux, pull the service()
  //  call out and put it somewhere else

  // You'll still need to call updateReg() before servicing any individual encoders, but you
  // only need to do it once for everything on that mux
  static void updateReg();

  void service() override;
};
