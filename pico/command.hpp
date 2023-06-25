


#ifndef __COMMAND_H__
#define __COMMAND_H__


#include<stdio.h>
#include<stdint.h>

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
} Motor;


typedef struct 
{
    Led led;
    Motor motors[4];
    Servo servos[4];
} Command;



#endif /* __COMMAND_H__ */
