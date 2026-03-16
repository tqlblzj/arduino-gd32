/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.

    Based on mbed-os/targets/TARGET_GigaDevice/TARGET_GD32F30X/analogin_api.c
    Based on mbed-os/targets/TARGET_GigaDevice/TARGET_GD32F30X/analogout_api.c
*/

#include "Arduino.h"
#include "analog.h"
#include "pwm.h"

#define DAC_NUMS  2
#define PWM_NUMS  40
#if (defined(GD32F30X_HD) || defined(GD32F30X_XD))
    #define ADC_NUMS  3
#else
    #define ADC_NUMS  2
#endif

analog_t ADC_[ADC_NUMS] = {0};


//pwm set value
void set_pwm_value(uint32_t ulPin, uint32_t value)
{
    uint16_t ulvalue = 1000 * value / 65535;
    PWM pwm(ulPin);
    pwm.setPeriodCycle(1000, ulvalue, FORMAT_US);
    pwm.start();
}

//get adc value
uint16_t get_adc_value(PinName pinname)
{
    uint16_t value;
    uint32_t adc_periph = pinmap_peripheral(pinname, PinMap_ADC);
    uint8_t index = get_dac_index(adc_periph);
    uint8_t channel = get_adc_channel(pinname);
    if(!ADC_[index].isactive) {
        pinmap_pinout(pinname, PinMap_ADC);
        adc_clock_enable(adc_periph);
        adc_clock_config(ADC_ADCCK_PCLK2_DIV6);
        adc_resolution_config(ADC_RESOLUTION_12B);
        adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
        adc_channel_length_config(ADC_ROUTINE_CHANNEL, 1U);
        adc_external_trigger_config(ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
        adc_enable();
        delay(1U);
        ADC_[index].isactive = true;
    }
    adc_routine_channel_config(0U, channel, ADC_SAMPLETIME_14POINT5);
    adc_software_trigger_enable(ADC_ROUTINE_CHANNEL);
    while(!adc_flag_get(ADC_FLAG_EOC));
    adc_flag_clear(ADC_FLAG_EOC);
    value = adc_routine_data_read();
    return value;
}

//get adc index value
uint8_t get_adc_index(uint32_t instance)
{
    uint8_t index;
    switch(instance) {
        case ADC:
            index = 0;
            break;
        default:
            index = 0;
            break;
    }
    return index;
}

// get dac index value
uint8_t get_dac_index(uint32_t instance)
{
    return 0;
}

//get adc channel
uint8_t get_adc_channel(PinName pinname)
{
    uint32_t function = pinmap_function(pinname, PinMap_ADC);
    uint32_t channel = GD_PIN_CHANNEL_GET(function);
    uint32_t gd_channel = 0;
    switch(channel) {
        case 0:
            gd_channel = ADC_CHANNEL_0;
            break;
        case 1:
            gd_channel = ADC_CHANNEL_1;
            break;
        case 2:
            gd_channel = ADC_CHANNEL_2;
            break;
        case 3:
            gd_channel = ADC_CHANNEL_3;
            break;
        case 4:
            gd_channel = ADC_CHANNEL_4;
            break;
        case 5:
            gd_channel = ADC_CHANNEL_5;
            break;
        case 6:
            gd_channel = ADC_CHANNEL_6;
            break;
        case 7:
            gd_channel = ADC_CHANNEL_7;
            break;
        case 8:
            gd_channel = ADC_CHANNEL_8;
            break;
        case 9:
            gd_channel = ADC_CHANNEL_9;
            break;
        case 10:
            gd_channel = ADC_CHANNEL_10;
            break;
        default:
            gd_channel = 0xFF;
            break;
    }
    return gd_channel;
}

//adc clock enable
void adc_clock_enable(uint32_t instance)
{
    rcu_periph_enum temp;
    switch(instance) {
        case ADC:
            temp = RCU_ADC;
            break;
        default:
            break;
    }
    rcu_periph_clock_enable(temp);
}
