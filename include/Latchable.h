// ------------------------------------------------------------------------
// Latchable.h
//
// Defines a software model of a hardware latch or register's set/clock/clear
// functionality. Allows you to set and forget the next value you want the
// output to take; the output won't change until the precise moment you want
// it to (i.e. when you set its clock input HIGH)
//
// Nov. 2023
// Ryan "Ratimus" Richardson
// ------------------------------------------------------------------------
#ifndef LASH_A_BULL_DOT_AITCH
#define LASH_A_BULL_DOT_AITCH
#include <Arduino.h>


template <typename T>
  class latchable
{
protected:
  T ParamR;
  T ParamQ;
  T ParamS;
  bool enabled;

public:
  const T& Q;
  T& D;

  explicit latchable(T data):
    ParamR(data),
    ParamQ(ParamR),
    ParamS(ParamQ),
    enabled(true),
    Q(ParamQ),
    D(ParamS)
  { ; }

  // Just like on a HW latch - set LOW and it won't do anything
  bool enable(bool en)
  {
    enabled = en;
    return enabled;
  }

  // Loads input but doesn't set ouput until a clock is received
  virtual void set(T data)
  {
    if (enabled)
    {
      ParamS = data;
    }
  }

  // Latches internal state to output
  virtual T clock(void)
  {
    if (enabled) { ParamQ = ParamS; }
    return Q;
  }

  // Latches in data and sets output in a single step
  T clockIn(T data)
  {
    set(data);
    return clock();
  }

  // Clears internal state without affecting output
  void clear()
  {
    set(ParamR);
  }

  // Clears internal state and outputs
  void reset()
  {
    clear();
    clock();
  }

  // Returns true if current output state does not match input state
  T pending()
  {
    return (D - Q);
  }

  // Change the default value to which element reverts on RESET
  void preEnable(T data)
  {
    ParamR = data;
  }
};


#endif