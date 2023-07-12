#pragma once


#include "pico/stdlib.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include <encoder.hpp>
#include <command.hpp>
#include <v_bat.hpp>

// enum MOTOR_MODE {OFF, POWER, SPEED, DISTANCE};
//
// typedef struct {
//     enum MOTOR_MODE mode = OFF;
//     int16_t power;
//     int16_t speed;
//     int32_t distance;
//
//     float ff = 1.0;
//     float kp = 1.0;
//     float ki = 0.0;
//     float kd = 0.0;
// } Motor;
//1024 max PWM from python + 5% for charge pump
const uint16_t PWM_top=1075-1;
const float VBAT_MIN=6.2f;
const float VBAT_MAX=8.6f;

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

    //commented by design. we need to enable them in sync at once.
    //pwm_set_enabled(slice_num, true);
    return true;
  }


    void exec_command(Motor command){
    switch (command.mode){
      case OFF:
        printf("motor off \n");
        drive_power(0);
        break;
      case POWER:
        printf("motor %d\n", command.power);
        drive_power(command.power);
        break;
      default:
        ASSERTM(false, "Not Implemented");
    }

  }

    void drive_power(int power){
        ASSERT(power < PWM_top);
        ASSERT(power > -PWM_top);
        //check if vbat is out of range
        if(ADC.cached_vbat<VBAT_MIN)
          return;
        //check if vbat is out of range
        if(ADC.cached_vbat>VBAT_MAX)
          return;
        //scale power to 6V
        power=power*6.0f/ADC.cached_vbat;

        uint16_t power_left,power_right;
        constexpr uint16_t max_power=PWM_top*0.95;
        //we also need to clip PWM to 95% to give chance to charge pump to do it's work
    
        if(power < 0){
          if((-power)>max_power)
            power_left=max_power;
          else
            power_left=-power;
          power_right=0;
        }else{
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
