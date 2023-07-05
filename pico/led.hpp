/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "led_pwm.pio.h"

// Write `period` to the input shift register
static void pio_pwm_set_period(PIO pio, uint sm, uint32_t period) {
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_put_blocking(pio, sm, period);
    pio_sm_exec(pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));
    pio_sm_set_enabled(pio, sm, true);
}

static inline void pwm_program_init(PIO pio, uint sm, uint offset, uint pin) {
   pio_gpio_init(pio, pin);
   pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
   pio_sm_config c = led_pwm_program_get_default_config(offset);
   sm_config_set_sideset_pins(&c, pin);
   pio_sm_init(pio, sm, offset, &c);

   pio_pwm_set_period(pio, sm, 0xFF);
}


class RBGLed{

    static PIO pio;
    public:

    bool init(int red, int green, int blue){
        uint offset = pio_add_program(pio, &led_pwm_program);

        pwm_program_init(pio, 0, offset, red);
        pwm_program_init(pio, 1, offset, green);
        pwm_program_init(pio, 2, offset, blue);
        return true;
    }


    void set_color(uint8_t red, uint8_t green, uint8_t blue){
        pio_sm_put_blocking(pio, 0, red);
        pio_sm_put_blocking(pio, 1, green);
        pio_sm_put_blocking(pio, 2, blue);
    }

};

PIO RBGLed::pio = pio1;
