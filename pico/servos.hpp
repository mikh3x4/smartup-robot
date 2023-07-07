#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "hardware/spi.h"
#include "hardware/dma.h"

#include "error.hpp"
#include "pins.h"

// USE THOSE DEFINITIONS FROM PINS.H
// #define SERVO_CS 5
// #define SERVO_CLK 6
// #define SERVO_DATA 7

const uint transfer_length=2001;

class ServosHardware{
    private:
    uint8_t dma_channel,reload_channel;
    //to be sure it's allocated statically and just once
    uint8_t txbuf[transfer_length];
    uint8_t *txbuf_pointer;
    uint16_t servo_value[4];
    bool servo_power[4];

    void generate_array()
    {
        for(uint i=0;i<transfer_length;i++)
        {
            uint8_t tmp=0;
            //zero if on, one if off (pmos inverts signal)
            tmp+=servo_power[0]?0:(1<<0);
            //zero if on, one if off (pmos inverts signal)
            tmp+=servo_power[1]?0:(1<<2);
            //zero if on, one if off (pmos inverts signal)
            tmp+=servo_power[2]?0:(1<<4);
            //zero if on, one if off (pmos inverts signal)
            tmp+=servo_power[3]?0:(1<<6);
            
            tmp+=(i>servo_value[0])?0:(1<<1);
            tmp+=(i>servo_value[1])?0:(1<<3);
            tmp+=(i>servo_value[2])?0:(1<<5);
            tmp+=(i>servo_value[3])?0:(1<<7);
            txbuf[i]=tmp;
        }
    }

    public:

    bool init(){
        //we need pointer to array for DMA to have something to copy
        txbuf_pointer=txbuf;
        //set servos to center
        servo_value[0]=servo_value[1]=servo_value[2]=servo_value[3]=1000;
        //set servos to off
        servo_power[0]=servo_power[1]=servo_power[2]=servo_power[3]=0;
        generate_array();
        // Enable SPI at 10 MHz and connect to GPIOs
        // that should give 1us output tick (8 clock cycles of data and 2 of CS)
        spi_init(spi_default, 1000 * 1000 * 10);
        gpio_set_function(SERVO_CS  , GPIO_FUNC_SPI);
        gpio_set_function(SERVO_CLK , GPIO_FUNC_SPI);
        gpio_set_function(SERVO_DATA, GPIO_FUNC_SPI);
        //grab DMA
        dma_channel   =dma_claim_unused_channel(true);
        reload_channel=dma_claim_unused_channel(true);
        //configure 50hz PWM that'll trigger second DMA
        //clock after prescaler 488.2khz
        pwm_set_clkdiv_int_frac(DMA_PWM_SLICE,255,0);
        //interrupt every 50hz
        pwm_set_wrap(DMA_PWM_SLICE, 9764);
        //we don't configure anything else as we only care about wrap flag
        pwm_set_enabled(DMA_PWM_SLICE, true);
        //configuring SPI sending DMA
        dma_channel_config c = dma_channel_get_default_config(dma_channel);
        channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
        channel_config_set_dreq(&c, spi_get_dreq(spi0, true));
        dma_channel_configure(dma_channel, &c,
                            &spi_get_hw(spi0)->dr, // write address
                            txbuf, // read address
                            transfer_length, // element count (each element is of size transfer_data_size)
                            false); // don't start yet
        //configuring retriggering DMA
        c = dma_channel_get_default_config(reload_channel);
        channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
        channel_config_set_read_increment(&c,false);
        channel_config_set_write_increment(&c,false);
        channel_config_set_dreq(&c, 24+DMA_PWM_SLICE);
        dma_channel_configure(reload_channel, &c,
                            &(dma_hw->ch[dma_channel].al3_read_addr_trig), //write address (trigger of previous dma)
                            &txbuf_pointer, // read address
                            0xffffff, // with 50hz load rate this should be sufficient for 23khours
                            true); // start

        return true;
    }


    void set_angle(int servo_number, int angle){
        ASSERT(servo_number>=0);
        ASSERT(servo_number<4);
        //TODO: implement actual angle to pulse width conversion
        ASSERT(angle>=1000);
        ASSERT(angle<=2000);
        servo_value[servo_number]=angle;
        generate_array();
    }

    void set_power(int servo_number, bool power){
        ASSERT(servo_number>=0);
        ASSERT(servo_number<4);
        servo_power[servo_number]=power;
        generate_array();
    }

};
