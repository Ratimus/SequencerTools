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
//  button_latch.register_callback(
//    [](const uint8_t& from, const uint8_t& to)
//    {
//      std::cout << "Button changed from " << from << " to " << to << "\n";
//    }
//  );
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
private:
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
  std::function<void(const T&, const T&)> on_change_callback = nullptr;

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
    ParamR(data),
    ParamQ(data),
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
  bool enable(bool en = true)
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    enabled = en;
    return enabled;
  }

  // Sets INPUT to argument but doesn't set output
  T set_input(const T& val)
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    ParamS = val;
    return ParamS;
  }

  // Sets OUTPUT to value of INPUT if ENABLED
  T clock()
  {
    bool changed = false;
    T previous;
    T current;

    {
      std::lock_guard<std::mutex> lock(latch_mutex);
      previous = ParamQ;
      current  = ParamS;

      if (enabled && (previous != current))
      {
        ParamQ = ParamS;
        changed = true;
      }
    }

    // Invoke callback outside the lock to prevent deadlocks
    if (changed && on_change_callback)
    {
      on_change_callback(previous, current);
    }

    return current;
  }

  // Latches in data and sets output in a single step if ENABLED
  T clock_in(const T& val)
  {
    set_input(val);
    return clock();
  }

  // Resets INPUT state to RESET value and CLOCKS it to the OUTPUT if ENABLED
  void reset()
  {
    clock_in(ParamR);
  }

  ///////////////////////////////////////////////////////////////////////////////////
  //  ASYNCHRONOUS FUNCTIONS

  // Asynchronous - forces INPUT to immediately take the value of RESET if ENABLED
  void jam()
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    if (enabled)
    {
      ParamS = ParamR;
    }
  }

  // Asynchronous - foces RESET and INPUT to immediately take the value of agument if ENABLED
  void jam(const T& val)
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    if (enabled)
    {
      ParamS = ParamR = val;
    }
  }

  // Asynchronous - forces INPUT and OUTPUT to immediately take value of RESET if ENABLED
  T clear()
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    if (enabled)
    {
      ParamQ = ParamS = ParamR;
    }

    return out;
  }

  // Asynchronous - forces OUTPUT to immediately take value of INPUT if ENABLED
  T preset()
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    if (enabled)
    {
      ParamQ = ParamS;
    }

    return out;
  }

  // Asynchronous - forces OUTPUT and INPUT to immediately take value of argument if ENABLED
  T preset(const T& val)
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    ParamS = val;
    if (enabled)
    {
      ParamQ = ParamS;
    }

    return out;
  }

  // Asynchronous - immediately sets INPUT to OUTPUT
  T loopback()
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    if (enabled)
    {
      ParamS = ParamQ;
    }

    return out;
  }
  ///////////////////////////////////////////////////////////////////////////////////
  //  COMPARATOR FUNCTIONS

  // Register a callback to fire when value changes
  void register_callback(std::function<void(const T&, const T&)> cb)
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    on_change_callback = std::move(cb);
  }

  // Returns true if OUTPUT state does not match INPUT state
  bool pending()
  {
    std::lock_guard<std::mutex> lock(latch_mutex);
    return (in != out);
  }

  // Comparison to another latchable<T>, returns true if both outputs match
  // (input, enable, and reset values ignored)
  bool operator == (const latchable<T>& comp) const
  {
    return (comp.ParamQ == this->ParamQ);
  }

  // Comparison to base type, returns true if output == comparison value
  bool operator == (T comp) const
  {
    return (comp == this->ParamQ);
  }

  template <typename N>
  T operator = (N) = delete;

  template <typename N>
  bool operator == (N) = delete;
};
