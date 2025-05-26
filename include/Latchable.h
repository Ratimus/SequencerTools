// ------------------------------------------------------------------------
// Latchable.h
//
// Defines a software model of a hardware D-type latch or register's
// set/clock/clear functionality. Allows you to set and forget the next value
// you want the output to take; the output won't change until the precise
// moment you want it to (i.e. when you set its clock input HIGH)
//
// Nov. 2023
// Ryan "Ratimus" Richardson
//
// EXAMPLE USAGE:
//  latchable<uint8_t> button_latch;
//
//  button_latch.onChange([](const uint8_t& from, const uint8_t& to)
//  {
//    std::cout << "Button changed from " << from << " to " << to << "\n";
//  });
//
//  button_latch.set(1);
//  button_latch.clock();
//
// ------------------------------------------------------------------------
#pragma once
#include <mutex>
#include <type_traits>
#include <functional>


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

  mutable std::mutex latch_mutex;
  std::function<void(const T&, const T&)> on_change_callback;

public:
  const T& out;     // Read-only OUTPUT state
  T& in;            // DATA input/SET value

  // Default constructor (only enabled if T is default-constructible)
  template <typename U = T,
            typename = typename std::enable_if<std::is_default_constructible<U>::value>::type>
  latchable():
    ParamR(),
    ParamQ(),
    ParamS(ParamQ),
    enabled(true),
    out(ParamQ),
    in(ParamS)
  { ; }

  // CTOR
  latchable(T data):
    ParamR(static_cast<T>(data)),
    ParamQ(static_cast<T>(data)),
    ParamS(ParamQ),
    enabled(true),
    out(ParamQ),
    in(ParamS)
  { ; }

  // Copy CTOR
  latchable(const latchable<T>& L):
    ParamR(L.ParamR),
    ParamQ(L.ParamQ),
    ParamS(ParamQ),
    enabled(true),
    out(ParamQ),
    in(ParamS)
  { ; }

  // DTOR
  ~latchable() { ; }

  // Just like on a HW latch - set LOW and it won't do anything
  virtual bool enable(bool en = true)
  {
    enabled = en;
    return enabled;
  }

  // Loads input but doesn't set ouput until a clock is received
  virtual T set(T val)
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    if (enabled)
    {
      ParamS = val;
    }

    return ParamS;
  }

  // Latches internal state to output
  virtual T clock(void)
  {
    std::function<void(const T&, const T&)> callback_copy;
    T previous;
    T current;

    {
      std::lock_guard<std::mutex> lock(latch_mutex);
      if (enabled && ParamQ != ParamS)
      {
        previous = ParamQ;
        ParamQ = ParamS;
        current = ParamQ;
        callback_copy = on_change_callback;
      }
      else
      {
        return out; // No change; no callback
      }
    }

    // Invoke callback outside the lock to prevent deadlocks
    if (callback_copy)
    {
      callback_copy(previous, current);
    }

    return current;
  }

  // Latches in data and sets output in a single step
  virtual T clockIn(T val)
  {
    std::function<void(const T&, const T&)> callback_copy;
    T previous;
    T current;

    {
      std::lock_guard<std::mutex> lock(latch_mutex);
      if (!enabled)
      {
        return out;
      }

      ParamS = val;
      if (ParamQ == ParamS)
      {
        return out;
      }

      previous      = ParamQ;
      ParamQ        = ParamS;
      current       = ParamQ;
      callback_copy = on_change_callback;
    }

    if (callback_copy)
    {
      callback_copy(previous, current);
    }

    return current;
  }


  // Clears internal state without affecting output
  virtual void clear()
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    set(ParamR);
  }

  // Clears internal state and outputs
  virtual void reset()
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    clear();
    clock();
  }

  // Returns true if current output state does not match input state
  virtual bool pending()
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    return (in != out);
  }

  // Change the default value to which element reverts on RESET
  virtual void preEnable(T val)
  {
    ParamR = val;
  }

  // Comparison to another latchable<T>, returns true if both outputs match
  // (input, enable, and reset values ignored)
  virtual bool operator == (latchable<T> comp)
  {
    return (comp.ParamQ == this->ParamQ);
  }

  // Comparison to base type, returns true if output == comparison value
  virtual bool operator == (T comp)
  {
    return (comp == this->ParamQ);
  }

  void onChange(std::function<void(const T&, const T&)> cb)
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    on_change_callback = cb;
  }

  template <typename N>
  T operator = (N) = delete;

  template <typename N>
  bool operator == (N) = delete;
};
