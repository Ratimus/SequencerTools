#include "CD4067.h"
#include <DirectIO.h>


void IRAM_ATTR muxTask(void *param)
{
  HW_Mux *mux = static_cast<HW_Mux *>(param);
  while (1)
  {
    if (pdTRUE != xSemaphoreTake(mux->timingSemaphore, portMAX_DELAY))
    {
      continue;
    }

    if (pdTRUE != xSemaphoreTake(mux->resourceMutex, 0))
    {
      continue;
    }

    mux->tmp = mux->MUXREG;
    mux->MUXREG = 0;

    for (auto n: GRAY_CODE)
    {
      mux->muxEnable(n);
      if (!directRead(mux->IO))
      {
        bitWrite(mux->MUXREG, n, 1);
      }
    }

    xSemaphoreGive(mux->resourceMutex);
  }
}


HW_Mux::HW_Mux(const uint8_t* const addrPins, uint8_t ioPin):
    IO(ioPin),
    MUXREG(0),
    tmp(0)
{
  for (auto n(0); n < 4; ++n)   // C'mon C++... if Python's got enumerate(), why can't we???
  {
    ADDR[n] = addrPins[n];
    pinMode(ADDR[n], OUTPUT);
  }
  pinMode(IO, INPUT_PULLUP);

  muxTaskHandle   = TaskHandle_t(NULL);
  resourceMutex   = xSemaphoreCreateMutex();
  timingSemaphore = xSemaphoreCreateBinary();

  xTaskCreatePinnedToCore
  (
    muxTask,
    "MUX Task",
    4096,
    this,
    10,
    &muxTaskHandle,
    0
  );
}


void HW_Mux::muxEnable(uint8_t channel)
{
  static uint8_t CURRENT_CHANNEL = 0b00001111;
  uint8_t diff = CURRENT_CHANNEL ^ channel;
  for (auto n(0); n < 4; ++n)
  {
    uint8_t mask = (0x01 << n);
    if (diff ^ mask)
    {
      continue;
    }

    if (channel & mask)
    {
      directWriteHigh(ADDR[n]);
    }
    else
    {
      directWriteLow(ADDR[n]);
    }
  }

  CURRENT_CHANNEL = channel;
  delayMicroseconds(5);
}


uint16_t HW_Mux::getReg(void)
{
  uint16_t ret = tmp;
  if (pdTRUE == xSemaphoreTake(resourceMutex, 0))
  {
    ret = MUXREG;
    xSemaphoreGive(resourceMutex);
  }
  return ret;
}

/*
ONE WAY TO DO VIRTUAL PINS:

class VirtualPin
{
  const uint8_t ID;
  virtual bool read() = 0;
  friend class PinLibrary;

public:

  CTOR();
};

class PinLibrary
{
  VirtualPins pins[];

public:

  uint8_t addPin(args)
  {
    std::shared_ptr<VirtualPin> newPin = std::make_shared<VirtualPin>(args, nextID)
    pins.push_back(newPin);
    return newPin->ID;
  }

  bool read(uint8_t pinNum)
  {
    return pins.at(pinNum)->read();
  }
};
/////////////////////////////////////////////////////
quicker, dirtier way?:

class MuxedButton : public MagicButton
{
  static uint16_t MUX_REG;
  static CD4067 MuxObject;
  static uint8_t lastRead;

public:

  static void service(uint8_t serviceTime)
  {
    if (serviceTime == lastRead)
    {
      return;
    }

    lastRead = serviceTime;
    MUX_REG = MuxObject.readMux();
  }
... ok, is there a mutex or lock in FreeRTOS that allows multiple concurrent reads but locks everything for writes?
problem:
  I have a register that I need to make ReadOnly.
  I only want to service it once per cycle, and I only want to have to lock, read, and unlock it once, allowing any
    objects that depend on it to read from it freely without having to re-copy the entire register every time.
  I probably don't want e.g. 8 mutices.

  Do this:

  ISR()
  {
    mux.service();
    // don't need thread-safe copy of mux_reg since we're using it in the same scope that updated it
    timestamp = millis();
    for (buttons in muxedButtonArray)
    {
      button.service(mux_register & 1 << n, timestamp);
    }
  }

  class MuxedButton : public MagicButton
  {
    const uint16_t * const register;
    const uint8_t bitNum;

    void service(const long long * const timestamp)
    {
      newReadVal = (*register) & (1 >> bitNum);
      newTimeVal = (*timestamp);

      // ... do everything else like a regular MagicButton
    }

    problem: MagicButton needs to support pin == -1???
  }

  loop() { mux}
};



*/
