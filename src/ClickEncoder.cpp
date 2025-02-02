// ----------------------------------------------------------------------------
// Rotary Encoder Driver with Acceleration
// Supports Click, DoubleClick, Long Click
// Integrates debounced "MagicButton" and interfaces with ClickEncoderInterface
//
// Ryan "Ratimus" Richardson
// Nov. 2022
// ----------------------------------------------------------------------------

#include "ClickEncoder.h"
#include <DirectIO.h>

// ----------------------------------------------------------------------------
// Acceleration configuration (for 1000Hz calls to ::service())
//
const uint16_t ENC_ACCEL_TOP (3072);  // max. acceleration: *12 (val >> 8)
const uint8_t  ENC_ACCEL_INC (25);
const uint8_t  ENC_ACCEL_DEC (2);

// ----------------------------------------------------------------------------

ClickEncoder::ClickEncoder(int8_t A,
                           int8_t B,
                           int8_t BTN,
                           uint8_t stepsPerNotch,
                           bool usePulllResistor) :
   doubleClickable(true),
   accelerationEnabled(false),
   position(0),
   delta(0),
   acceleration(0),
   steps(stepsPerNotch),
   pinA(A),
   pinB(B),
   lastEncoded(0),
   activeLow(usePulllResistor),
   encoderMutex(xSemaphoreCreateRecursiveMutex())
{
  if (pinA != -1)
  {
    hwButton = std::make_shared<MagicButton>(BTN, activeLow, doubleClickable);
    uint8_t configType = activeLow ? INPUT_PULLUP : INPUT;
    pinMode(pinA,   configType);
    pinMode(pinB,   configType);

    MSB = readA();
    LSB = readB();
  }
  else
  {
    hwButton = nullptr;
  }
}


// ----------------------------------------------------------------------------
// call this every 1 millisecond via timer ISR
//
void ClickEncoder::service(void)
{
  lock();
  long encoded = 0;
  long tmpMSB  = (long)readA();
  long tmpLSB  = (long)readB();

  // TODO: we're mimicking the hardware interrupts here, so we need to handle the first three
  // lines for one bit then run the equivalent of the ISR once *BEFORE* we do the same thing
  // for the other pin. If you want, put the redundant stuff in its own function so you can
  // call it without disturbing the other logic, since the sequence here is very important
  if (MSB != tmpMSB)
  {
    MSB      = tmpMSB;
    encoded  = (MSB << 1) | LSB;
    long sum = (lastEncoded << 2) | encoded;  // Add it to the previous encoded value
    switch(sum)
    {
      case 0b1101:
      case 0b0100:
      case 0b0010:
      case 0b1011:
        ++delta;
        break;

      case 0b1110:
      case 0b0111:
      case 0b0001:
      case 0b1000:
        --delta;
        break;

      default:
        break;
    }

    int16_t oldPos = position;
    while (delta >= (int16_t)steps)
    {
      delta -= (int16_t)steps;
      ++position;
    }

    while (delta <= -(int16_t)steps)
    {
      delta += (int16_t)steps;
      --position;
    }

    lastEncoded = encoded;
  }

  if (LSB != tmpLSB)
  {
    LSB      = tmpLSB;
    encoded  = (MSB << 1) | LSB;
    long sum = (lastEncoded << 2) | encoded;  // Add it to the previous encoded value
    switch(sum)
    {
      case 0b1101:
      case 0b0100:
      case 0b0010:
      case 0b1011:
        ++delta;
        break;

      case 0b1110:
      case 0b0111:
      case 0b0001:
      case 0b1000:
        --delta;
        break;

      default:
        break;
    }

    int16_t oldPos = position;
    while (delta >= (int16_t)steps)
    {
      delta -= (int16_t)steps;
      ++position;
    }

    while (delta <= -(int16_t)steps)
    {
      delta += (int16_t)steps;
      --position;
    }

    lastEncoded = encoded;
  }

  unlock();
  hwButton->service();
}


bool ClickEncoder::readA()
{
  return (directRead(pinA) ^ activeLow);
}


bool ClickEncoder::readB()
{
  return (directRead(pinB) ^ activeLow);
}


bool ClickEncoder::lock(void)
{
  return (xSemaphoreTakeRecursive(encoderMutex, MUTEX_TIMEOUT) == pdTRUE);
}


void ClickEncoder::unlock(void)
{
  xSemaphoreGiveRecursive(encoderMutex);
}


void ClickEncoder::onPinChange()
{
  lock();
  MSB = readB();
  LSB = readA();

  int encoded = (MSB << 1) | LSB;           // Convert pin B to single number
  int sum  = (lastEncoded << 2) | encoded;  // Add it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
  {
    ++delta;
  }

  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
  {
    --delta;
  }

  while (delta >= steps)
  {
    delta -= steps;
    ++position;
  }

  while (delta <= -steps)
  {
    delta += steps;
    --position;
  }

  lastEncoded = encoded;
  unlock();
}

// ----------------------------------------------------------------------------

int16_t ClickEncoder::readPosition(void)
{
  lock();
  int16_t ret = position;
  unlock();

  return ret;
}


// ----------------------------------------------------------------------------
// Resets buttonState and returns value prior to reset; encBtnState output state
// persists until this function is called && encBtnState has been released
ButtonState ClickEncoder::readButton(void)
{
  lock();
  ButtonState ret = hwButton->read();
  unlock();
  return ret;
}

