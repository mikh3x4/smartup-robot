
#pragma once


#include "hardware/adc.h"

#include "pins.h"


class {

    const float conversion_factor = 3*3.3f / (1 << 12);
public:
    float smoothed_state;
    bool init(){
        adc_init();
        adc_gpio_init(VBAT_PIN);
        adc_set_temp_sensor_enabled(true);
        //732 sps
        adc_set_clkdiv(65535.0f);
        adc_irq_set_enabled(false);
        adc_select_input(VBAT_ADC);
        adc_fifo_setup(true,false,2,false,false);
        adc_run(true);
        smoothed_state=adc_fifo_get_blocking();
        return true;
    }

    float get_vbat(){
        float ADC_voltage = adc_fifo_get() * conversion_factor;
        smoothed_state=0.99*smoothed_state+0.01*ADC_voltage;
        return ADC_voltage;
    }
    float get_smoothed_vbat(){
        get_vbat();
        return smoothed_state;
    }

    /*float get_core_temp(){
        adc_select_input(4);
        float ADC_voltage = adc_read() * conversion_factor;
        return 27 - (ADC_voltage - 0.706)/0.001721;
    }*/
} ADC;
