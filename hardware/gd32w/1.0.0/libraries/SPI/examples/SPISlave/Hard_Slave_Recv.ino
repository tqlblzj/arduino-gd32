/*
 * Hard_Slave_Recv.ino
 *
 * SPI Slave Example with Hardware NSS Control
 *
 * Description:
 * - Demonstrates SPI slave mode with hardware NSS (Chip Select) control
 * - Receives data from SPI master
 * - Displays SPI configuration and received data
 *
 * Features:
 * - Hardware NSS control
 * - Interrupt-driven data reception
 * - SPI prescaler and clock frequency display
 * - Device mode detection
 *
 * IMPORTANT:
 * - This example requires SPI slave mode configuration
 * - If spiobj->spi_struct.device_mode is SPI_MASTER, change it to SPI_SLAVE
 * - Modify drv_spi.c to set device_mode = SPI_SLAVE for slave operation
 *
 * Hardware Requirements:
 * - SPI slave device with hardware NSS support
 * - SPI master device to send data
 *
 * Serial Baud Rate: 115200
 */

#include "Arduino.h"
#include "SPI.h"

SPIClass *hard_spi = nullptr;
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n========== Arduino SPI Slave TEST ==========");

    hard_spi = new SPIClass(DIGITAL_TO_PINNAME(MOSI), DIGITAL_TO_PINNAME(MISO), DIGITAL_TO_PINNAME(SCK), DIGITAL_TO_PINNAME(SS));

    hard_spi->begin();
    Serial.println("Slave initialized");

    printSPIPrescale(hard_spi->getSpiDev()->spi_struct.prescale);
    printSPIClockFreq(hard_spi);

    if( hard_spi->getSpiDev()->spi_struct.nss == SPI_NSS_HARD )
        Serial.println("NSS Mode: Hardware Control");
    else
        Serial.println("NSS Mode: Software Control");

    if( hard_spi->getSpiDev()->spi_struct.device_mode == SPI_MASTER ){
        Serial.println("Device Mode: Master");
        Serial.println("spiobj->spi_struct.device_mode (drv_spi.c) should be changed to SPI_SLAVE !!!");
    }else
        Serial.println("Device Mode: Slave");
}
void loop() {
    if(spi_flag_get(SPI_FLAG_RBNE) == SET) {
        int v = SPI.receive();
        Serial.println(v, HEX);
    }
}

void printSPIPrescale(uint32_t prescale)
{
    Serial.print("SPI Prescale: ");

    switch(prescale) {
        case SPI_PSC_2:
            Serial.println("SPI_PSC_2 (÷2)");
            break;
        case SPI_PSC_4:
            Serial.println("SPI_PSC_4 (÷4)");
            break;
        case SPI_PSC_8:
            Serial.println("SPI_PSC_8 (÷8)");
            break;
        case SPI_PSC_16:
            Serial.println("SPI_PSC_16 (÷16)");
            break;
        case SPI_PSC_32:
            Serial.println("SPI_PSC_32 (÷32)");
            break;
        case SPI_PSC_64:
            Serial.println("SPI_PSC_64 (÷64)");
            break;
        case SPI_PSC_128:
            Serial.println("SPI_PSC_128 (÷128)");
            break;
        case SPI_PSC_256:
            Serial.println("SPI_PSC_256 (÷256)");
            break;
        default:
            Serial.print("Unknown prescale value: 0x");
            Serial.println(prescale, HEX);
            break;
    }
}

uint32_t getSPIPrescaleValue(uint32_t prescale)
{
    switch(prescale) {
        case SPI_PSC_2:   return 2;
        case SPI_PSC_4:   return 4;
        case SPI_PSC_8:   return 8;
        case SPI_PSC_16:  return 16;
        case SPI_PSC_32:  return 32;
        case SPI_PSC_64:  return 64;
        case SPI_PSC_128: return 128;
        case SPI_PSC_256: return 256;
        default:          return 0;
    }
}

void printSPIClockFreq(SPIClass *spi)
{
    uint32_t prescale = spi->getSpiDev()->spi_struct.prescale;
    uint32_t prescale_value = getSPIPrescaleValue(prescale);

    uint32_t apb2_freq = rcu_clock_freq_get(CK_APB2);
    float spi_freq = apb2_freq / prescale_value;

    Serial.print("APB2 Frequency: ");
    Serial.print(apb2_freq / 1000000);
    Serial.println("MHz");

    Serial.print("SPI Clock Frequency: ");
    Serial.print(spi_freq / 1000000);
    Serial.println("MHz");
}