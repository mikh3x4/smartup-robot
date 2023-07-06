#pragma once


#include "pico/stdlib.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include <encoder.hpp>

const uint16_t PWM_top=1000-1;

class MotorHardware{

  int pin_a;
  int pin_b;

  uint slice_num;

  public:
  Encoder encoder; //will template on this for 4th encoder

  bool init(int motor_pin_a, int motor_pin_b, int encoder_pin_a, int encoder_pin_b){
    encoder.init(encoder_pin_a, encoder_pin_b);
    pin_a = motor_pin_a;
    pin_b = motor_pin_b;

    gpio_set_function(pin_a, GPIO_FUNC_PWM);
    gpio_set_function(pin_b, GPIO_FUNC_PWM);

    slice_num = pwm_gpio_to_slice_num(pin_a);
    uint other_slice_num = pwm_gpio_to_slice_num(pin_b);

    ASSERTM(slice_num == other_slice_num, "One motor's pwm pins need to be on the same pwm slice!");

    pwm_set_wrap(slice_num, PWM_top);

    pwm_set_gpio_level(pin_a, 0);
    pwm_set_gpio_level(pin_b, 0);
    
    //we need inverted PWM to keep 0 as bottom MOS on
    pwm_set_output_polarity(slice_num,true,true);

    //commented by desing. we need to enable them in sync at once.
    //pwm_set_enabled(slice_num, true);
    return true;
  }

    void drive_power(int power){
        ASSERT(power < 256);
        ASSERT(power > -256);

        int current_pin = pin_b;
        int other_pin = pin_a;

        if(power < 0){
            current_pin = pin_a;
            other_pin = pin_b;
            power = -power;
        }

        pwm_set_gpio_level(other_pin, 0);
        pwm_set_gpio_level(current_pin, power);
    }

    // TODO Implement more of them
    void drive_speed(int speed){
    }

    bool is_done(){
        return false;
    }

};

static void inline enable_PWM()
{
    //set PWMs 90 degrees out of phase for lower noise
    pwm_set_counter(0,0*PWM_top/4);
    pwm_set_counter(1,1*PWM_top/4);
    pwm_set_counter(2,2*PWM_top/4);
    pwm_set_counter(5,3*PWM_top/4);
    //enable all channels at once
    pwm_set_mask_enabled(0b100111);
}