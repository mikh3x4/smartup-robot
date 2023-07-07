#pragma once


#include "pico/stdlib.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"

#include <encoder.hpp>
#include <command.hpp>

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

    pwm_set_wrap(slice_num, 0xFF);

    pwm_set_gpio_level(pin_a, 0);
    pwm_set_gpio_level(pin_b, 0);

    pwm_set_enabled(slice_num, true);
    return true;
  }


    void exec_command(Motor command){
    switch (command.mode){
      case OFF:
        printf("motor off");
        drive_power(0);
        break;
      case POWER:
        printf("motor %d", command.power);
        drive_power(command.power);
        break;
      default:
        ASSERTM(false, "Not Implemented");
    }

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

