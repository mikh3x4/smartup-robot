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
    //no prescaler we run at full 125Mhz
    pwm_set_clkdiv_int_frac(slice_num,1,0);
    
    pwm_set_both_levels(slice_num,0,0);
    
    //we need inverted PWM to keep 0 as bottom MOS on
    pwm_set_output_polarity(slice_num,false,false);

    //commented by desing. we need to enable them in sync at once.
    //pwm_set_enabled(slice_num, true);
    return true;
  }

    void drive_power(int power){
        ASSERT(power < PWM_top);
        ASSERT(power > -PWM_top);

        uint16_t power_left,power_right;
        constexpr uint16_t max_power=PWM_top*0.95;
        //we also need to clip PWM to 95% to give chance to charge pump to do it's work
        if(power < 0){
          if((-power)>max_power)
            power_left=max_power;
          else
            power_left=-power;
          power_right=0;
        }
        else{
          power_left=0;
          if(power>max_power)
            power_right=max_power;
          else
            power_right=power;
        }
        //we need to set both channels at once
        pwm_set_both_levels(slice_num,power_left,power_right);
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