#include "CD4067.h"
#include "MuxedButton.h"
#include "freertos/FreeRTOS.h"

portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

constexpr uint8_t MUX_A0_PIN      = 0;
constexpr uint8_t MUX_A1_PIN      = 4;
constexpr uint8_t MUX_A2_PIN      = 2;
constexpr uint8_t MUX_A3_PIN      = 17;
constexpr uint8_t MUX_0_IO_PIN    = 16;
constexpr uint8_t MUX_1_IO_PIN    = 26;

constexpr uint8_t MUX_ADDR[] = {MUX_A0_PIN, MUX_A1_PIN, MUX_A2_PIN, MUX_A3_PIN};

MultiMux shared_mux;
MuxedButton button;

constexpr uint8_t analog_pin_A    = 0;
constexpr uint8_t analog_pin_B    = 2;
constexpr uint8_t button_pin      = 1;
constexpr uint8_t digital_pin     = 3;


void IRAM_ATTR isr()
{
  portENTER_CRITICAL_ISR(&spinlock);
  shared_mux.service();
  button.service();
  portEXIT_CRITICAL_ISR(&spinlock);
}


void setup_mux_example()
{
  uint8_t analog_mux = shared_mux.add_analog_mux(MUX_0_IO_PIN);
  uint8_t digital_mux = shared_mux.add_digital_mux(MUX_1_IO_PIN);

  shared_mux.add_analog_pin(analog_pin_A, analog_mux);
  shared_mux.add_analog_pin(analog_pin_B, analog_mux);
  shared_mux.add_digital_pin(digital_pin, analog_mux);

  button.init(&shared_mux, button_pin, analog_mux);

  uint8_t clock_divider = 0;

  while (1)
  {
    isr();

    if (clock_divider == 0)
    {
      uint16_t ANALOG_A = shared_mux.get_val(analog_pin_A, analog_mux);
      uint16_t ANALOG_B = shared_mux.get_val(analog_pin_B, analog_mux);

      bool DIGITAL_VAL  = shared_mux.get_val(digital_pin, analog_mux);

      uint16_t REGISTER = 0;
      for (uint8_t n = 0; n < 16; ++n)
      {
        bool pin_val_n = shared_mux.get_val(n, digital_mux);
        REGISTER |= ((uint16_t)pin_val_n) << n;
      }

      ButtonState bn = button.read();
    }

    ++clock_divider;
    clock_divider %= 100;
    delay(1);
  }
}



