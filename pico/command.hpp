


#ifndef __COMMAND_H__
#define __COMMAND_H__


#include<stdio.h>
#include<stdint.h>

#include "lwip/pbuf.h"
#include "lwip/udp.h"

typedef struct 
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint16_t blink;
} Led;


typedef struct 
{
    bool on = false;
    int angle = 5;
} Servo;


enum MOTOR_MODE {OFF, POWER, SPEED, DISTANCE};


typedef struct 
{
    enum MOTOR_MODE mode = OFF;
    int16_t power;
    int16_t speed;
    int32_t distance;

    float ff = 1.0;
    float kp = 1.0;
    float ki = 0.0;
    float kd = 0.0;
} Motor;


typedef struct 
{
    Led led;
    Motor motors[4];
    Servo servos[4];

    ip_addr_t telemetry_address;
    u16_t telemetry_port;
    absolute_time_t recv_time;

    void estop(){
        led.red = 255;
        led.green = 0;
        led.blue = 0;
        led.blink = 100;

        motors[0].mode = OFF;
        motors[1].mode = OFF;
        motors[2].mode = OFF;
        motors[3].mode = OFF;

        servos[0].on = false;
        servos[1].on = false;
        servos[2].on = false;
        servos[3].on = false;
    }

} Command;



#endif /* __COMMAND_H__ */
