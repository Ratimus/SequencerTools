# SequencerTools - A library of useful widgets for developing sequencers for synthesizers and drum machines
Contents:
- MagicButton: A state-driven pushbutton driver with a customizable debounce routine
- ClickEncoder: A Rotary Encoder driver with acceleration
- ClickEncoderInterface: An simple interface to a ClickEncoder object that will register state changes with a single function call
- RotaryEvent: A generic, hardware-agnostic implementation of a rotary encoder from M. Smit - github@gangkast.nl
- EncoderWrapper: A library to wrap a clickable rotary encoder in a generic command message structure for use with ArduinoMenuLibrary from Rui Azavedo (https://github.com/neu-rah/ArduinoMenu)
- Latchable: A template class that behaves like a hardware latch or register--a value on its input will not appear on the output until it is clocked. Useful for time-synchronizing state changes or for comparing upcoming state to current state before updating.
- OutputRegister: Extends the "Latchable" concept to a physical hardware serial-to-parallel shift register (e.g. 74HC595)
- SharedControl: A polymorphic, software-defined potentiometer that can be locked on a value when changing modes. When returned to a prior mode, the value of the control will not change until the hardware control is physically moved to the position corresponding to the locked value.
- RatFuncs: Some odds, ends, and debugging utilities
- OutputChannel: Abstracts a single DAC channel so that note values can be written to it without worrying about converting to HW units. Currently supports MCP4728; may add more in the future
- OutputDac: Bank to initialize and hold any number of logical OutputChannels
