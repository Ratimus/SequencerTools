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

template <typename T>
  class latchable
{
protected:
  // These values are protected, so access to them is limited.
  // We have |out|, a const reference to the output state ParamQ, meaning it's essentially
  // a read-only value.
  // We *can* change the value of |in|, our Data input, which is a reference to ParamS.
  // When we update() our latch, the output |out| changes to reflect the current state of
  // its input, |in|.
  T ParamR;         // State after RESET
  T ParamQ;         // Output state
  T ParamS;         // Input state
  bool enabled;     // Set low to hold output state constant regardless of input

public:
  const T& out;     // Read-only OUTPUT state
  T& in;            // DATA input/SET value

  // CTOR
  latchable(T data):
    ParamR(data),
    ParamQ(ParamR),
    ParamS(ParamQ),
    enabled(true),
    out(ParamQ),
    in(ParamS)
  { ; }

  // DTOR
  ~latchable() { ; }

  // Just like on a HW latch - set LOW and it won't do anything
  virtual bool enable(bool en)
  {
    enabled = en;
    return enabled;
  }

  // Loads input but doesn't set ouput until a clock is received
  virtual void set(T val)
  {
    if (enabled)
    {
      ParamS = val;
    }
  }

  // Latches internal state to output
  virtual T clock(void)
  {
    if (enabled) { ParamQ = ParamS; }
    return out;
  }

  // Latches in data and sets output in a single step
  virtual T clockIn(T val)
  {
    set(val);
    return clock();
  }

  // Clears internal state without affecting output
  virtual void clear()
  {
    set(ParamR);
  }

  // Clears internal state and outputs
  virtual void reset()
  {
    clear();
    clock();
  }

  // Returns true if current output state does not match input state
  virtual bool pending()
  {
    return (in != out);
  }

  // Change the default value to which element reverts on RESET
  virtual void preEnable(T val)
  {
    ParamR = val;
  }

  virtual bool operator == (latchable<T> comp) { return comp.ParamQ == this->ParamQ; }
  virtual void operator = (T val) { ParamS = val; }
};


#endif