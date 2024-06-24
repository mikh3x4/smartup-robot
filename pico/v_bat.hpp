
#pragma once

#include <cstdint>
#include "hardware/adc.h"
#include "hardware/dma.h"

#include "pins.h"

const uint buffer_length=16; //nie ruszac zahardcodowane w innych miejscach

class {
    const float conversion_factor = (43.0 / 10.0)*3.3f / (1 << 12);
    uint8_t dma_ch1,dma_ch2;
public:
    uint16_t ADC_buffer[buffer_length] __attribute__((aligned(buffer_length*sizeof(uint16_t *))));;
    uint16_t *ADC_buff_pointer;
    float smoothed_state;
    bool init(){
        //we need pointer to buffer for DMA
        ADC_buff_pointer=ADC_buffer;
        adc_init();
        //temp sensor and pin 28
        adc_set_round_robin(10100);
        //start from vbat ADC
        adc_select_input(2);
        adc_gpio_init(VBAT_PIN);
        adc_set_temp_sensor_enabled(true);
        //732 sps
        adc_set_clkdiv(65535.0f);
        adc_irq_set_enabled(false);    
        adc_fifo_setup(
            true,    // Write each completed conversion to the sample FIFO
            true,    // Enable DMA data request (DREQ)
            1,       // DREQ (and IRQ) asserted when at least 1 sample present
            false,   // We won't see the ERR bit because of 8 bit reads; disable.
            false     // Don't shift each sample to 8 bits when pushing to FIFO
        );
        //setup DMAs that'll read ADC continously
        dma_ch1=dma_claim_unused_channel(true);
        dma_ch2=dma_claim_unused_channel(true);


        dma_channel_config c = dma_channel_get_default_config(dma_ch1);
        channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
        channel_config_set_dreq(&c, DREQ_ADC);
        channel_config_set_read_increment(&c, false);
        channel_config_set_write_increment(&c, true);
        //channel_config_set_chain_to(&c, dma_ch2);
        channel_config_set_ring(&c,true,5);
        dma_channel_configure(dma_ch1, &c,
                            ADC_buff_pointer, // write address
                            &(adc_hw->fifo), // read address
                            0xffffffff, // element count (each element is of size transfer_data_size)
                            true); // do start
        // channel_config_set_chain_to(&c, dma_ch1);
        // channel_config_set_write_increment(&c, false);
        // channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
        // channel_config_set_dreq(&c, 0x3f);
        // dma_channel_configure(dma_ch2, &c,
        //                     &(dma_hw->ch[dma_ch1].al2_write_addr_trig), //write address (trigger of previous dma)
        //                     &ADC_buff_pointer, // read address
        //                     1, // element count (each element is of size transfer_data_size)
        //                     false); // don't start yet
        //turn on ADC
        adc_run(true);
        smoothed_state=adc_fifo_get_blocking();
        return true;
    }

    float get_vbat(){
        float ADC_voltage = 0;
        for(uint i=0;i<buffer_length;i+=2)
            ADC_voltage+=ADC_buffer[i];
        return ADC_voltage * conversion_factor*2.0f/buffer_length;
    }
    float get_smoothed_vbat(){
        return get_vbat();
    }

    float get_core_temp(){
        uint32_t sum=0;
        for(uint i=0;i<buffer_length;i+=2)
            sum+=ADC_buffer[i+1];
        float ADC_voltage = sum * (3.3f / (1 << 12))*2.0f/buffer_length;;
        return 27 - (ADC_voltage - 0.706)/0.001721;
    }
} ADC;
