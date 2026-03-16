#ifndef _VARIANT_
#define _VARIANT_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
/* GPIO pins definitions */

#define PA8   0    // Arduino D0   UART1_RX
#define PB15  1    // Arduino D1   UART1_TX
#define PB0   2    // Arduino D2   GPIO_OUT
#define PB2   3    // Arduino D3   GPIO_OUT
#define PB3   4    // Arduino D4   GPIO_OUT
#define PB4   5    // Arduino D5   TIMER1_CH0
#define PB12  6    // Arduino D6   GPIO_OUT
#define PB13  7    // Arduino D7   GPIO_OUT
#define PA0   8    // Arduino D8   SPI_MOSI
#define PA1   9    // Arduino D9   SPI_MISO
#define PA2   10   // Arduino D10  I2C0_SCL
#define PA3   11   // Arduino D11  I2C0_SDA
#define PA4   12   // Arduino D12  ADC_IN or SPI_CS
#define PA5   13   // Arduino D13  SPI_CLK
#define PA6   14   // Arduino D14  UART2_TX
#define PA7   15   // Arduino D15  UART2_RX

/* digital pins and analog pins number definitions */
#define DIGITAL_PINS_NUM        16
#define ANALOG_PINS_NUM         8
#define ANALOG_PINS_START       8
#define ANALOG_PINS_LAST        15

/* LED definitions */
#define LED0                    PA5

/* I2C definitions */
#define USE_I2C                 1
#define PIN_WIRE_SCL            PA2
#define PIN_WIRE_SDA            PA3

/* TIMER or PWM definitions */
#define TIMER_TONE              TIMER0
// If you use PWM0 and flash the code to the development board via JTAG,
// you may encounter an error similar to "Error: riscv.cpu: IR capture error; saw 0x00 not 0x01".
// In this case, you need to flash the image.bin file provided in variants/GD32VW553_START_MINI
// to the development board by dragging and dropping it, so that you can flash code via JTAG again.
#define PWM0                    PB4

/* ADC definitions */
#define ADC_IN                  PA4

/* SPI definitions */
#define PIN_SPI_MOSI            PA0
#define PIN_SPI_MISO            PA1
#define PIN_SPI_SCK             PA5
#define PIN_SPI_SS              PA4

/* USART definitions */
#define PIN_SERIAL_RX1          PA8  // not used
#define PIN_SERIAL_TX1          PB15 // not used

#define USE_USART2_SERIAL       1
#define PIN_SERIAL_RX2          PA7
#define PIN_SERIAL_TX2          PA6

/* ADC definitions */
#define  ADC_RESOLUTION         10

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _VARIANT_ */
