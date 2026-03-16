/*
 * Soft_NSS.ino
 *
 * SPI Master with Software NSS Example
 *
 * Description:
 * - Demonstrates SPI master mode with software NSS (Chip Select) control
 * - Sends data to SPI slave device using three different transfer methods
 * - Manually controls NSS pin state before and after data transfer
 * - Displays SPI configuration and transfer results
 *
 * Features:
 * - Software NSS control (manual chip select management)
 * - NSS pin pulled LOW before transfer, HIGH after transfer
 * - Multiple data transfer methods:
 *   - Method 1: transfer(txdata, rxdata, len) - buffer transfer
 *   - Method 2: transfer(byte) - single byte transfer in loop
 *   - Method 3: beginTransaction() with custom SPI settings
 * - SPI prescaler and clock frequency display
 * - Device mode and NSS mode detection
 *
 * Hardware Requirements:
 * - SPI slave device
 * - Connect MOSI, MISO, SCK pins between master and slave
 * - SS pin configured as output for software control
 * - Common ground connection
 *
 * Serial Baud Rate: 115200
 */

#include "Arduino.h"
#include "SPI.h"

SPIClass *soft_spi = nullptr;

uint8_t testData1[] = {0x11, 0x12, 0x13, 0x14};
uint8_t testData2[] = {0x21, 0x22, 0x23, 0x24};
uint8_t testData3[] = {0x31, 0x32, 0x33, 0x34};
uint8_t rxbuf[4];
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n========== Arduino SPI MASTER TEST: SoftWare Mode ==========");

    soft_spi = new SPIClass(DIGITAL_TO_PINNAME(MOSI), DIGITAL_TO_PINNAME(MISO), DIGITAL_TO_PINNAME(SCK), NC);

    soft_spi->begin();
    Serial.println("Master initialized");

    printSPIPrescale(soft_spi->getSpiDev()->spi_struct.prescale);
    printSPIClockFreq(soft_spi);

    if( soft_spi->getSpiDev()->spi_struct.nss == SPI_NSS_HARD )
        Serial.println("NSS Mode: Hardware Control");
    else
        Serial.println("NSS Mode: Software Control");

    if( soft_spi->getSpiDev()->spi_struct.device_mode == SPI_MASTER )
        Serial.println("Device Mode: Master");
    else{
        Serial.println("Device Mode: Slave");
        Serial.println("spiobj->spi_struct.device_mode (drv_spi.c) should be changed to SPI_MASTER !!!");
    }

    pinMode(PIN_SPI_SS, OUTPUT);
    digitalWrite(PIN_SPI_SS, HIGH);
}
void loop()
{
    //The first way to send data
    testMaster1(soft_spi, testData1, rxbuf, 4);
    delay(1000);
    //The second way to send data
    testMaster2(soft_spi, testData2, rxbuf, 4);
    delay(1000);
    //The third way to send data
    testMaster3(soft_spi, testData3, rxbuf, 4);
    delay(1000);
}

void testMaster1(SPIClass *spi, uint8_t* txdata, uint8_t* rxdata, uint8_t len){
    digitalWrite(PIN_SPI_SS, LOW);
    spi->begin();
    spi->transfer(txdata, rxdata, len);

    for(int i = 0; i < 4; i++) {
        Serial.print("Sent: 0x");
        if(txdata[i] < 0x10) Serial.print("0");
        Serial.print(txdata[i], HEX);

        Serial.print(" | Received: 0x");
        if(rxdata[i] < 0x10) Serial.print("0");
        Serial.println(rxdata[i], HEX);
    }
    digitalWrite(PIN_SPI_SS, HIGH);
    spi->end();
}

void testMaster2(SPIClass *spi, uint8_t* txdata, uint8_t* rxdata, uint8_t len){
    digitalWrite(PIN_SPI_SS, LOW);
    spi->begin();
    for(int i = 0; i < len; i++) {
        rxdata[i] = spi->transfer(txdata[i]);

        Serial.print("Sent: 0x");
        if(txdata[i] < 0x10) Serial.print("0");
        Serial.print(txdata[i], HEX);

        //By default, SPI master will read default value 0x00
        Serial.print(" | Received: 0x");
        if(rxdata[i] < 0x10) Serial.print("0");
        Serial.println(rxdata[i], HEX);

        delay(300);
    }
    digitalWrite(PIN_SPI_SS, HIGH);
    spi->end();

}

void testMaster3(SPIClass *spi, uint8_t* txdata, uint8_t* rxdata, uint8_t len){
    digitalWrite(PIN_SPI_SS, LOW);
    spi->beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
    printSPIPrescale(spi->getSpiDev()->spi_struct.prescale);
    printSPIClockFreq(spi);
    for(int i = 0; i < 4; i++) {
        rxdata[i] = spi->transfer(txdata[i]);

        Serial.print("Sent: 0x");
        if(txdata[i] < 0x10) Serial.print("0");
        Serial.print(txdata[i], HEX);

        //By default, SPI master will read default value 0x00
        Serial.print(" | Received: 0x");
        if(rxdata[i] < 0x10) Serial.print("0");
        Serial.println(rxdata[i], HEX);

        delay(300);
    }
    spi->endTransaction();
    digitalWrite(PIN_SPI_SS, HIGH);
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

