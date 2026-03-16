#include "pins_arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

/* digital pins for pinmap list */
const PinName digital_pins[] = {
    PORTA_8  , // Arduino D0   UART1_RX
    PORTB_15 , // Arduino D1   UART1_TX
    PORTB_0  , // Arduino D2   GPIO_OUT
    PORTB_2  , // Arduino D3   GPIO_OUT
    PORTB_3  , // Arduino D4   GPIO_OUT
    PORTB_4  , // Arduino D5   TIMER1_CH0
    PORTB_12 , // Arduino D6   GPIO_OUT
    PORTB_13 , // Arduino D7   GPIO_OUT
    PORTA_0  , // Arduino D8   SPI_MOSI
    PORTA_1  , // Arduino D9   SPI_MISO
    PORTA_2  , // Arduino D10  I2C0_SCL
    PORTA_3  , // Arduino D11  I2C0_SDA
    PORTA_4  , // Arduino D12  ADC_IN or SPI_CS
    PORTA_5  , // Arduino D13  SPI_CLK
    PORTA_6  , // Arduino D14  UART2_TX
    PORTA_7  , // Arduino D15  UART2_RX
};

/* analog pins for pinmap list */
const uint32_t analog_pins[] = {
    8,  // PORTA_0,    // Arduion A0 ADC_IN10
    9,  // PORTA_1,    // Arduion A1 ADC_IN11
    10, // PORTA_2,    // Arduion A2 ADC_IN12
    11, // PORTA_3,    // Arduion A3 ADC_IN13
    12, // PORTA_4,    // Arduion A4 ADC_IN14
    13, // PORTA_5,    // Arduion A5 ADC_IN15
    14, // PORTA_6,    // Arduion A6 ADC_IN16
    15, // PORTA_7,    // Arduion A7 ADC_IN17
};

#ifdef __cplusplus
}
#endif
