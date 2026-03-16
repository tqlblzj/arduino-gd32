/*
 * Master_Send.ino
 *
 * SPI Master Send Example
 *
 * Description:
 * - Demonstrates basic SPI master mode data transmission
 * - Sends data to SPI slave device using three different transfer methods
 * - Simplified example without detailed configuration display
 *
 * Features:
 * - Hardware NSS control
 * - Three data transfer methods:
 *   - Method 1: transfer(txdata, rxdata, len) - buffer transfer
 *   - Method 2: transfer(byte) - single byte transfer in loop
 *   - Method 3: beginTransaction() with custom SPI settings
 *
 * Note:
 * - For detailed SPI configuration and diagnostics, refer to Hard_NSS.ino and Soft_NSS.ino
 *
 * Hardware Requirements:
 * - SPI slave device with hardware NSS support
 * - Connect MOSI, MISO, SCK, and SS pins between master and slave
 * - Common ground connection
 *
 * Serial Baud Rate: 115200
 */

#include "Arduino.h"
#include "SPI.h"

SPIClass *hard_spi = nullptr;

uint8_t testData1[] = {0x11, 0x12, 0x13, 0x14};
uint8_t testData2[] = {0x21, 0x22, 0x23, 0x24};
uint8_t testData3[] = {0x31, 0x32, 0x33, 0x34};
uint8_t rxbuf[4];
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n========== Arduino SPI MASTER TEST: Hardware Mode ==========");

    hard_spi = new SPIClass(DIGITAL_TO_PINNAME(MOSI), DIGITAL_TO_PINNAME(MISO), DIGITAL_TO_PINNAME(SCK), DIGITAL_TO_PINNAME(SS));

    hard_spi->begin();
    Serial.println("Master initialized");
}
void loop()
{
    //The first way to send data
    testMaster1(hard_spi, testData1, rxbuf, 4);
    delay(1000);
    //The second way to send data
    testMaster2(hard_spi, testData2, rxbuf, 4);
    delay(1000);
    //The third way to send data
    testMaster3(hard_spi, testData3, rxbuf, 4);
    delay(1000);
}

void testMaster1(SPIClass *spi, uint8_t* txdata, uint8_t* rxdata, uint8_t len){
    spi->begin();
    spi->transfer(txdata, rxdata, len);
    spi->end();
}

void testMaster2(SPIClass *spi, uint8_t* txdata, uint8_t* rxdata, uint8_t len){
    spi->begin();
    for(int i = 0; i < len; i++) {
        rxdata[i] = spi->transfer(txdata[i]);
        delay(300);
    }
    spi->end();

}

void testMaster3(SPIClass *spi, uint8_t* txdata, uint8_t* rxdata, uint8_t len){
    // For the first two ways, default SPI settings are used
    // Users can also set SPI settings before data transfer
    // In this way, you should also reset the speed of peer slave
    spi->beginTransaction(SPISettings(100000, MSBFIRST, SPI_MODE0));
    for(int i = 0; i < len; i++) {
        rxdata[i] = spi->transfer(txdata[i]);
        delay(300);
    }
    spi->endTransaction();
}
