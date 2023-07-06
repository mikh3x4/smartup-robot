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

static inline uint pwm_program_init(PIO pio, uint offset, uint pin) {
   pio_gpio_init(pio, pin);

   uint sm = pio_claim_unused_sm(pio, false);
   ASSERTM(sm >= 0, "Can't initialize PIO");

   pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);

   pio_sm_config c = led_pwm_program_get_default_config(offset);
   sm_config_set_sideset_pins(&c, pin);
   pio_sm_init(pio, sm, offset, &c);

   pio_pwm_set_period(pio, sm, 0xFF);

   return sm;
}


class RBGLed{

    static PIO pio;
    uint offset;

    uint red_sm;
    uint green_sm;
    uint blue_sm;
    public:

    bool init(int red, int green, int blue){

        if(pio == NULL){
            pio = pio1; // This pio is shared with the cyw43 driver which uses 1 sm and 6 instructions
                        // https://forums.raspberrypi.com/viewtopic.php?p=2111438
            offset = pio_add_program(pio, &led_pwm_program);
        }

        red_sm = pwm_program_init(pio, offset, red);
        green_sm = pwm_program_init(pio, offset, green);
        blue_sm = pwm_program_init(pio, offset, blue);
        return true;
    }


    void set_color(int16_t red, int16_t green, int16_t blue){
        pio_sm_put_blocking(pio, red_sm, red);
        pio_sm_put_blocking(pio, green_sm, green);
        pio_sm_put_blocking(pio, blue_sm, blue);
    }

};

PIO RBGLed::pio = NULL;
