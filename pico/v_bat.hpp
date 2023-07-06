
#pragma once


#include "hardware/adc.h"

#include "pins.h"

class ADC{

    const float conversion_factor = 3.3f / (1 << 12);
public:
    bool init(){
        adc_init();
        adc_gpio_init(VBAT_PIN);
        adc_set_temp_sensor_enabled(true);
        return true;
    }

    float get_vbat(){
        adc_select_input(VBAT_ADC);
        float ADC_voltage = adc_read() * conversion_factor;
        return ADC_voltage;
    }

    float get_core_temp(){
        adc_select_input(4);
        float ADC_voltage = adc_read() * conversion_factor;
        return 27 - (ADC_voltage - 0.706)/0.001721;
    }
};
