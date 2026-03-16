/*
Copyright (c) 2025, GigaDevice Semiconductor Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef HTTPOTA_H
#define HTTPOTA_H

#include <Arduino.h>
#include <WebServer.h>
#include "rom_export.h"
#include "config_gdm32.h"
#include "gd32vw55x_eclic.h"

// Flash API functions from MSDK
extern "C" {
    int raw_flash_read(uint32_t addr, void *data, uint32_t size);
    int raw_flash_write(uint32_t addr, const void *data, uint32_t size);
    int raw_flash_erase(uint32_t addr, uint32_t size);
}

// HttpOTA configuration constants
#define FLASH_SECTOR_SIZE     0x1000    // 4KB sector size
#define HTTPOTA_MAX_FIRMWARE_SIZE 2013264   // ~1.92MB max firmware size

// HttpOTA status codes
typedef enum {
    HTTPOTA_STATUS_IDLE = 0,        // Idle state, no HttpOTA operation in progress
    HTTPOTA_STATUS_UPLOADING,       // Firmware upload in progress
    HTTPOTA_STATUS_SUCCESS,         // Firmware update completed successfully
    HTTPOTA_STATUS_ERROR            // Firmware update failed with error
} HttpOTAStatus;

// HttpOTA error codes
typedef enum {
    HTTPOTA_ERROR_NONE = 0,                     // No error
    HTTPOTA_ERROR_CONTENT_TOO_LONG,             // Uploaded firmware content exceeds image partition size
    HTTPOTA_ERROR_RECEIVED_TOO_LONG,            // Received data exceeds expected size
    HTTPOTA_ERROR_FLASH_ERASE_FAILED,           // Flash erase operation failed
    HTTPOTA_ERROR_FLASH_WRITE_FAILED,           // Flash write operation failed
    HTTPOTA_ERROR_SET_IMAGE_STATUS_FAILED,      // Failed to set image flags
    HTTPOTA_ERROR_INVALID_FILE_TYPE,            // Invalid file type (not .bin file)
    HTTPOTA_ERROR_FILE_TOO_LARGE,               // File size exceeds maximum limit (1.92MB)
    HTTPOTA_ERROR_UNKNOWN                        // Unknown error occurred
} HttpOTAErrorCode;

// HttpOTA state structure
struct HttpOTAState {
    bool isUpdating;              // true: firmware update in progress, false: idle
    size_t bytesReceived;         // Number of bytes received so far
    size_t totalBytes;            // Total firmware size in bytes, no use now
    uint32_t offset;              // Current write offset in flash (relative to new image start address)
    uint32_t eraseStartAddr;      // Next flash sector address to erase (dynamic erase strategy)
    uint32_t newImgAddr;          // Flash start address for new firmware image
    uint32_t imgSize;             // Max image size in bytes that can be written to new image partition
    uint8_t runningIdx;           // Current running image index (0=IMAGE_0, 1=IMAGE_1)
    HttpOTAStatus status;             // Current HttpOTA status
    HttpOTAErrorCode lastError;       // Last error code
    char errorMessage[256];       // Error message string (null-terminated)
};

// HttpOTA class
class HttpOTA {
public:
    HttpOTA(WebServer &server);
    ~HttpOTA();
    bool begin();
    void handleFirmwareUpload();
    const HttpOTAState& getState() const { return _state; }
    uint8_t getRunningImageIndex() const { return _state.runningIdx; }
    uint32_t getNewImageAddress() const { return _state.newImgAddr; }
    bool isUpdating() const { return _state.isUpdating; }
    const char* getLastError() const { return _state.errorMessage; }
    HttpOTAErrorCode getLastErrorCode() const { return _state.lastError; }
    String getSystemInfo();
    String getFirmwareInfo();
    void setError(HttpOTAErrorCode code, const char* message);

private:
    WebServer &_server;
    HttpOTAState _state;

    void initHttpOTAState();
    void handleUploadStart(HTTPUpload &upload);
    void handleUploadWrite(HTTPUpload &upload);
    void handleUploadEnd();
    bool validateFile(HTTPUpload &upload);
    bool eraseFlashSectors(uint32_t offset, size_t size);
    bool writeFlash(uint32_t addr, const uint8_t *data, size_t size);
    bool setImageFlags();
    void reboot();
};

#endif // HTTPOTA_H
