
#pragma once


#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"

#include "encoder.pio.h"

class Encoder{

  static PIO pio;
  uint sm;

// max_step_rate is used to lower the clock of the state machine to save power
// if the application doesn't require a very high sampling rate. Passing zero
// will set the clock to the maximum

// static inline void quadrature_encoder_program_init(PIO pio, uint sm, uint pin, int max_step_rate)
// {
// }

  public:


  bool init(uint sm, int pin){

    //possibly runs multiple times but each time does the same thing
    // pio = pio0;

    pio_add_program(pio, &quadrature_encoder_program);

    // quadrature_encoder_program_init(pio, sm, pin, 0);

    pio_sm_set_consecutive_pindirs(pio, sm, pin, 2, false);
    gpio_pull_up(pin);
    gpio_pull_up(pin + 1);

    pio_sm_config c = quadrature_encoder_program_get_default_config(0);

    sm_config_set_in_pins(&c, pin);
    sm_config_set_jmp_pin(&c, pin);

    // shift to left, autopull disabled
    sm_config_set_in_shift(&c, false, false, 32);
    // don't join FIFO's
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_NONE);

    // passing "0" as the sample frequency,
    // if (max_step_rate == 0) {
        sm_config_set_clkdiv(&c, 1.0);
    // } else {
    //     // one state machine loop takes at most 10 cycles
    //     float div = (float)clock_get_hz(clk_sys) / (10 * max_step_rate);
    //     sm_config_set_clkdiv(&c, div);
    // }

    pio_sm_init(pio, sm, 0, &c);
    pio_sm_set_enabled(pio, sm, true);

    return true;
  }


  int get_count(){

    uint ret;
    int n;

    // if the FIFO has N entries, we fetch them to drain the FIFO,
    // plus one entry which will be guaranteed to not be stale
    n = pio_sm_get_rx_fifo_level(pio, sm) + 1;
    while (n > 0) {
        ret = pio_sm_get_blocking(pio, sm);
        n--;
    }

    return ret;
  }

};

PIO Encoder::pio = pio0;

