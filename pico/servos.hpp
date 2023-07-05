#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "hardware/spi.h"
#include "hardware/dma.h"

#include "error.hpp"
#include "pins.h"

// USE THOSE DEIFNITIONG FROM PINS.H
// #define SERVO_CS 5
// #define SERVO_CLK 6
// #define SERVO_DATA 7

class ServosHardware{
    public:

    bool init(){
        return true;
    }


    void set_angle(int servo_number, int angle){
    }

    void set_power(int servo_number, bool power){
    }

};
