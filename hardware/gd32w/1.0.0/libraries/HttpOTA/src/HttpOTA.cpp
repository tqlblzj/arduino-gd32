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

#include "HttpOTA.h"

// Constructor
HttpOTA::HttpOTA(WebServer &server) : _server(server) {
    // Initialize state
    memset(&_state, 0, sizeof(_state));
    _state.status = HTTPOTA_STATUS_IDLE;
    _state.lastError = HTTPOTA_ERROR_NONE;
}

// Destructor
HttpOTA::~HttpOTA() {
    // Nothing to clean up
}

// Initialize HttpOTA
bool HttpOTA::begin() {
    initHttpOTAState();
    return true;
}

// Initialize HttpOTA state
void HttpOTA::initHttpOTAState() {
    memset(&_state, 0, sizeof(_state));
    _state.isUpdating = false;
    _state.status = HTTPOTA_STATUS_IDLE;
    _state.lastError = HTTPOTA_ERROR_NONE;

    // Get current running image index
    int32_t res = rom_sys_status_get(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &_state.runningIdx);
    if (res < 0) {
        printf("Get sys running idx failed! (res = %d)\r\n", res);
        _state.runningIdx = IMAGE_0;
    }

    // Determine new image address and size based on running index
    if (_state.runningIdx == IMAGE_0) {
        _state.newImgAddr = RE_IMG_1_OFFSET;
        _state.imgSize = RE_IMG_1_END - RE_IMG_1_OFFSET;
    } else {
        _state.newImgAddr = RE_IMG_0_OFFSET;
        _state.imgSize = RE_IMG_1_OFFSET - RE_IMG_0_OFFSET;
    }
}

// Handle firmware upload
void HttpOTA::handleFirmwareUpload() {
    // Increase timeout for large file uploads (30 seconds)
    _server.client().setTimeout(30000);

    HTTPUpload &upload = _server.upload();

    switch (upload.status) {
        case UPLOAD_FILE_START:
            handleUploadStart(upload);
            break;

        case UPLOAD_FILE_WRITE:
            handleUploadWrite(upload);
            break;

        case UPLOAD_FILE_END:
            handleUploadEnd();
            break;

        case UPLOAD_FILE_ABORTED:
            printf("Upload aborted\r\n");
            _state.isUpdating = false;
            _state.status = HTTPOTA_STATUS_ERROR;
            setError(HTTPOTA_ERROR_UNKNOWN, "Upload aborted");
            break;

        default:
            break;
    }
}

// Handle upload start
void HttpOTA::handleUploadStart(HTTPUpload &upload) {
    printf("Upload Start: %s\r\n", upload.filename.c_str());

    // Validate file
    if (!validateFile(upload)) {
        return;
    }

    // Initialize HttpOTA state
    initHttpOTAState();
    _state.isUpdating = true;
    _state.status = HTTPOTA_STATUS_UPLOADING;
    _state.offset = 0;
    _state.eraseStartAddr = _state.newImgAddr;
    _state.bytesReceived = 0;

    // Validate file size
    if (upload.totalSize > _state.imgSize) {
        printf("Content too long!!!\r\n");
        setError(HTTPOTA_ERROR_CONTENT_TOO_LONG, "Content too long");
        _server.send(413, "text/plain", "Content too long");
        _state.isUpdating = false;
        _state.status = HTTPOTA_STATUS_ERROR;
        return;
    }
}

// Handle upload write
void HttpOTA::handleUploadWrite(HTTPUpload &upload) {
    size_t recv_len = upload.currentSize;
    uint32_t offset = _state.offset;
    uint32_t new_img_addr = _state.newImgAddr;

    // Check if received data exceeds image size
    if (offset + recv_len > _state.imgSize) {
        printf("received too long: %lu\r\n", offset + recv_len);
        setError(HTTPOTA_ERROR_RECEIVED_TOO_LONG, "Received too long");
        _server.send(413, "text/plain", "received too long");
        _state.isUpdating = false;
        _state.status = HTTPOTA_STATUS_ERROR;
        return;
    }

    // Erase flash sectors as needed (dynamic erase from httpota_demo.c)
    if (!eraseFlashSectors(offset, recv_len)) {
        return;
    }

    // Write to flash
    if (!writeFlash(new_img_addr + offset, upload.buf, recv_len)) {
        return;
    }

    // Update offset
    _state.offset += recv_len;
    _state.bytesReceived += recv_len;

    // Print progress every 100KB
    if (_state.bytesReceived % (100 * 1024) < recv_len) {
        printf("Progress: %lu bytes\r\n", _state.bytesReceived);
    }
}

// Handle upload end
void HttpOTA::handleUploadEnd() {
    printf("Upload complete\r\n");

    // Set image flags
    if (!setImageFlags()) {
        return;
    }

    _state.isUpdating = false;
    _state.status = HTTPOTA_STATUS_SUCCESS;

    printf("Firmware update successful!\r\n");
    printf("Rebooting in 5 seconds so new image can be used!\r\n");

    // Send response BEFORE any delays
    _server.send(200, "text/plain", "Update successful");

    // Flush and close connection to ensure response is fully sent
    _server.client().flush();
    delay(100);
    _server.client().stop();

    // Long delay to ensure response is fully sent before reboot
    for (int i = 5; i > 0; i--) {
        delay(1000);
    }

    // Reboot the device
    reboot();
}

// Validate file
bool HttpOTA::validateFile(HTTPUpload &upload) {
    // Check file extension
    if (!upload.filename.endsWith(".bin")) {
        printf("Invalid file type: not .bin\r\n");
        setError(HTTPOTA_ERROR_INVALID_FILE_TYPE, "Invalid file type. Please select a .bin file");
        _server.send(400, "text/plain", "Invalid file type. Please select a .bin file");
        return false;
    }

    // Check file size
    if (upload.totalSize > HTTPOTA_MAX_FIRMWARE_SIZE) {
        printf("File too large: %u\r\n", upload.totalSize);
        setError(HTTPOTA_ERROR_FILE_TOO_LARGE, "File too large");
        _server.send(413, "text/plain", "File too large");
        return false;
    }

    return true;
}

// Erase flash sectors
bool HttpOTA::eraseFlashSectors(uint32_t offset, size_t recv_len) {
    uint32_t new_img_addr = _state.newImgAddr;

    // Dynamic erase from httpota_demo.c
    while (offset + new_img_addr + recv_len > _state.eraseStartAddr) {
        int ret = raw_flash_erase(_state.eraseStartAddr, FLASH_SECTOR_SIZE);
        if (ret != 0) {
            setError(HTTPOTA_ERROR_FLASH_ERASE_FAILED, "Flash erase failed");
            _server.send(500, "text/plain", "Flash erase failed");
            _state.isUpdating = false;
            _state.status = HTTPOTA_STATUS_ERROR;
            return false;
        }
        _state.eraseStartAddr += FLASH_SECTOR_SIZE;
    }

    return true;
}

// Write to flash
bool HttpOTA::writeFlash(uint32_t addr, const uint8_t *data, size_t size) {
    int ret = raw_flash_write(addr, data, size);
    if (ret != 0) {
        setError(HTTPOTA_ERROR_FLASH_WRITE_FAILED, "Flash write failed");
        _server.send(500, "text/plain", "Flash write failed");
        _state.isUpdating = false;
        _state.status = HTTPOTA_STATUS_ERROR;
        return false;
    }
    return true;
}

// Set image flags
bool HttpOTA::setImageFlags() {
    int32_t res = 0;
    res = rom_sys_set_img_flag(_state.runningIdx, (IMG_FLAG_IA_MASK | IMG_FLAG_NEWER_MASK), (IMG_FLAG_IA_OK | IMG_FLAG_OLDER));
    res |= rom_sys_set_img_flag(!_state.runningIdx, (IMG_FLAG_IA_MASK | IMG_FLAG_VERIFY_MASK | IMG_FLAG_NEWER_MASK), 0);
    res |= rom_sys_set_img_flag(!_state.runningIdx, IMG_FLAG_NEWER_MASK, IMG_FLAG_NEWER);

    if (res != 0) {
        printf("Set sys image status failed! (res = %d)\r\n", res);
        setError(HTTPOTA_ERROR_SET_IMAGE_STATUS_FAILED, "Set image status failed");
        _server.send(500, "text/plain", "Set image status failed");
        _state.isUpdating = false;
        _state.status = HTTPOTA_STATUS_ERROR;
        return false;
    }

    return true;
}

// Reboot device
void HttpOTA::reboot() {
    eclic_system_reset();
}

// Set error message
void HttpOTA::setError(HttpOTAErrorCode code, const char* message) {
    _state.lastError = code;
    strncpy(_state.errorMessage, message, sizeof(_state.errorMessage) - 1);
    _state.errorMessage[sizeof(_state.errorMessage) - 1] = '\0';
}

// Get system info as JSON
String HttpOTA::getSystemInfo() {
    // Refresh running image index
    rom_sys_status_get(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &_state.runningIdx);

    String json = "{";
    json += "\"chip\": \"GD32VW553\",";
    json += "\"runningImage\": \"IMAGE_" + String(_state.runningIdx) + "\",";
    json += "}";
    return json;
}

// Get firmware info as JSON
String HttpOTA::getFirmwareInfo() {
    String json = "{";
    json += "\"BuiltDate\": \"" + String(__DATE__) + " " + String(__TIME__) + "\",";
    json += "\"Device\": \"GD32VW553\"";
    json += "}";
    return json;
}
