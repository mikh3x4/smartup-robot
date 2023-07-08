
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

  public:
  bool init(int pin, int second_pin){
    ASSERTM((pin + 1 == second_pin), "pio encoder pins need to be consecutive");

    if(pio == NULL){
      pio = pio0;
      pio_add_program(pio, &quadrature_encoder_program);
    }

    // quadrature_encoder_program_init(pio, sm, pin, 0);
    sm = pio_claim_unused_sm(pio, false);
    ASSERTM(sm >=0, "Can't initalize PIO");

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

PIO Encoder::pio = NULL;



///////////////////////////////////////


//
// inline static gpio_irq_callback_t isr_list[32];
//
// static void do_nothing(uint gpio, uint32_t events) {}
//
// static void gpio_callback(uint gpio, uint32_t events);

class InteruptEncoder{

  static int count;
  static char state;

  static int pin_A;
  static int pin_B;

  public:
  bool init(int pin, int second_pin){


   // for (int c = 0; c < 2; ++c)
   //  {
   //      for (int p = 0; p <= 32; ++p)
   //      {
   //          isr_list[c][p] = do_nothing;
   //      }
   //  }


    gpio_init(pin);
    gpio_init(second_pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_set_dir(second_pin, GPIO_IN);

    pin_A = pin;
    pin_B = second_pin;
    count = 0;
    state = (gpio_get(second_pin) << 1) | gpio_get(pin);

    return true;
  }


  static void gpio_callback(uint gpio, uint32_t events) {
      char mask = 1;
      if (gpio == pin_B){
        mask = mask << 1;
      }else if(gpio == pin_A){
        mask = mask;
      }else{
        return;
      }

    char new_state;
     if (events == GPIO_IRQ_EDGE_RISE){
        new_state = state | mask;
    }else{
        new_state = state & (~mask);
    }

    char state_tuple = (state << 2) | new_state;

    switch(state_tuple){
      case 0b0000:          break;
      case 0b0001: count--; break;
      case 0b0010: count++; break;
      case 0b0011:          break;

      case 0b0100: count++; break;
      case 0b0101:          break;
      case 0b0110:          break;
      case 0b0111: count--; break;

      case 0b1000: count--; break;
      case 0b1001:          break;
      case 0b1010:          break;
      case 0b1011: count++; break;

      case 0b1100:          break;
      case 0b1101: count++; break;
      case 0b1110: count--; break;
      case 0b1111:          break;

    }
    state = new_state;
  }
  
  bool init_core_sepecific(){
    gpio_set_irq_enabled(pin_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(pin_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);

    gpio_set_irq_callback(&gpio_callback);
    irq_set_enabled(IO_IRQ_BANK0, true);
    return true;
  }

  int get_count(){
    return count;
  }

};

int InteruptEncoder::count;
char InteruptEncoder::state;
int InteruptEncoder::pin_A;
int InteruptEncoder::pin_B;
