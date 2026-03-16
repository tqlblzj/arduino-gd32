/*
 * GD32VW553 OTA Update Example
 *
 * This example demonstrates how to perform OTA (Over-The-Air) firmware updates
 * using the WebServer library. The device can receive new firmware via HTTP POST
 * and update itself.
 *
 * Hardware: GD32VW553
 *
 * Features:
 * - Web interface for firmware upload
 * - Progress indication during upload
 * - Automatic reboot after successful update
 * - Dual image system (IMAGE_0 and IMAGE_1)
 *
 * Usage:
 * 1. Connect to WiFi
 * 2. Open browser to http://<device_ip>/
 * 3. Select firmware .bin file and upload
 * 4. Device will reboot with new firmware
 *
 * Firmware Requirements:
 * - The firmware .bin file should be compiled for the correct memory layout
 * - Use the same compiler flags as the original firmware
 * - Maximum firmware size: ~1.92MB (RE_IMG_1_OFFSET - RE_IMG_0_OFFSET)
 */

#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include "rom_export.h"
#include "config_gdm32.h"
#include "gd32vw55x_eclic.h"

// Include raw_flash_api from MSDK
extern "C" {
    // Flash API functions from MSDK
    int raw_flash_read(uint32_t addr, void *data, uint32_t size);
    int raw_flash_write(uint32_t addr, const void *data, uint32_t size);
    int raw_flash_erase(uint32_t addr, uint32_t size);
}

// WiFi credentials
char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";

// Dual image system with automatic image selection
#define FLASH_SECTOR_SIZE     0x1000    // 4KB sector size

// Web server on port 80
WebServer server(80);

// OTA state
// Reference: ota_demo.c http_rsp_image() function
struct OTAState {
    bool isUpdating;
    size_t bytesReceived;
    size_t totalBytes;
    uint32_t offset;
    uint32_t eraseStartAddr;
    uint32_t newImgAddr;
    uint32_t imgSize;
    uint8_t runningIdx;
} otaState;

// HTML page for OTA upload
const char otaHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GD32VW553 OTA Update</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 16px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            max-width: 500px;
            width: 100%;
            padding: 40px;
        }
        h1 {
            color: #333;
            margin-bottom: 10px;
            font-size: 28px;
        }
        .subtitle {
            color: #666;
            margin-bottom: 30px;
            font-size: 14px;
        }
        .info-box {
            background: #f8f9fa;
            border-radius: 8px;
            padding: 15px;
            margin-bottom: 20px;
            font-size: 13px;
        }
        .info-box strong { color: #495057; }
        .info-box span { color: #6c757d; }
        .upload-area {
            border: 2px dashed #dee2e6;
            border-radius: 8px;
            padding: 30px;
            text-align: center;
            margin-bottom: 20px;
            transition: all 0.3s;
            cursor: pointer;
        }
        .upload-area:hover, .upload-area.dragover {
            border-color: #667eea;
            background: #f8f9ff;
        }
        .upload-area input[type="file"] {
            display: none;
        }
        .upload-icon {
            font-size: 48px;
            color: #adb5bd;
            margin-bottom: 10px;
        }
        .upload-text {
            color: #495057;
            margin-bottom: 5px;
        }
        .upload-hint {
            color: #adb5bd;
            font-size: 12px;
        }
        .btn {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            padding: 12px 30px;
            border-radius: 8px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            width: 100%;
            transition: transform 0.2s;
        }
        .btn:hover {
            transform: translateY(-2px);
        }
        .btn:disabled {
            opacity: 0.6;
            cursor: not-allowed;
            transform: none;
        }
        .progress-container {
            display: none;
            margin-top: 20px;
        }
        .progress-bar {
            height: 8px;
            background: #e9ecef;
            border-radius: 4px;
            overflow: hidden;
            margin-bottom: 10px;
        }
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #667eea, #764ba2);
            width: 0%;
            transition: width 0.3s;
        }
        .progress-text {
            text-align: center;
            font-size: 14px;
            color: #666;
        }
        .status {
            padding: 12px;
            border-radius: 8px;
            margin-top: 15px;
            display: none;
            font-size: 14px;
        }
        .status.success {
            background: #d4edda;
            color: #155724;
            display: block;
        }
        .status.error {
            background: #f8d7da;
            color: #721c24;
            display: block;
        }
        .status.info {
            background: #d1ecf1;
            color: #0c5460;
            display: block;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔄 OTA Update</h1>
        <p class="subtitle">Upload new firmware to update your device</p>

        <div class="info-box">
            <div><strong>Device:</strong> <span>GD32VW553</span></div>
            <div><strong>Max Size:</strong> <span>~1.92MB</span></div>
            <div><strong>Format:</strong> <span>.bin file</span></div>
        </div>

        <form id="uploadForm" enctype="multipart/form-data">
            <div class="upload-area" id="uploadArea">
                <input type="file" id="fileInput" accept=".bin">
                <div class="upload-icon">📁</div>
                <div class="upload-text">Click or drag firmware file here</div>
                <div class="upload-hint">Supports .bin files up to 1.92MB</div>
            </div>

            <button type="submit" class="btn" id="uploadBtn">Upload & Update</button>
        </form>

        <div class="progress-container" id="progressContainer">
            <div class="progress-bar">
                <div class="progress-fill" id="progressFill"></div>
            </div>
            <div class="progress-text" id="progressText">0%</div>
        </div>

        <div class="status" id="status"></div>
    </div>

    <script>
        const uploadArea = document.getElementById('uploadArea');
        const fileInput = document.getElementById('fileInput');
        const uploadForm = document.getElementById('uploadForm');
        const uploadBtn = document.getElementById('uploadBtn');
        const progressContainer = document.getElementById('progressContainer');
        const progressFill = document.getElementById('progressFill');
        const progressText = document.getElementById('progressText');
        const status = document.getElementById('status');

        uploadArea.addEventListener('click', () => fileInput.click());

        uploadArea.addEventListener('dragover', (e) => {
            e.preventDefault();
            uploadArea.classList.add('dragover');
        });

        uploadArea.addEventListener('dragleave', () => {
            uploadArea.classList.remove('dragover');
        });

        uploadArea.addEventListener('drop', (e) => {
            e.preventDefault();
            uploadArea.classList.remove('dragover');
            if (e.dataTransfer.files.length) {
                fileInput.files = e.dataTransfer.files;
                updateFileName();
            }
        });

        fileInput.addEventListener('change', updateFileName);

        function updateFileName() {
            if (fileInput.files.length) {
                document.querySelector('.upload-text').textContent = fileInput.files[0].name;
            }
        }

        uploadForm.addEventListener('submit', async (e) => {
            e.preventDefault();

            if (!fileInput.files.length) {
                showStatus('Please select a firmware file', 'error');
                return;
            }

            const file = fileInput.files[0];

            // Validate file extension
            if (!file.name.endsWith('.bin')) {
                showStatus('Please select a .bin firmware file', 'error');
                return;
            }

            // Validate file size (max 1.92MB)
            if (file.size > 2013264) {
                showStatus('File too large. Maximum size is 1.92MB', 'error');
                return;
            }

            uploadBtn.disabled = true;
            progressContainer.style.display = 'block';
            showStatus('Starting upload...', 'info');

            const formData = new FormData();
            formData.append('firmware', file);

            try {
                const xhr = new XMLHttpRequest();
                let uploadComplete = false;

                // Debug: log all ready state changes
                xhr.addEventListener('readystatechange', () => {
                    console.log('XHR readyState:', xhr.readyState, 'status:', xhr.status);
                });

                xhr.upload.addEventListener('progress', (e) => {
                    console.log('Upload progress:', e.loaded, '/', e.total, 'lengthComputable:', e.lengthComputable);
                    if (e.lengthComputable) {
                        const percent = Math.round((e.loaded / e.total) * 100);
                        progressFill.style.width = percent + '%';
                        progressText.textContent = percent + '%';

                        // Mark upload as complete when 100%
                        if (percent >= 100) {
                            uploadComplete = true;
                            console.log('Upload complete flag set to true');
                        }
                    }
                });

                // Upload finished event - this fires when upload is complete
                xhr.upload.addEventListener('load', () => {
                    console.log('Upload load event fired - upload complete');
                    uploadComplete = true;
                });

                xhr.addEventListener('load', () => {
                    console.log('XHR load event triggered. status:', xhr.status, 'responseText:', xhr.responseText);
                    // Check if response contains "Update successful" or status is 200
                    if (xhr.status === 200 || (xhr.responseText && xhr.responseText.includes('Update successful'))) {
                        showSuccessMessage();
                    } else {
                        showStatus('✗ Update failed: ' + xhr.responseText, 'error');
                        uploadBtn.disabled = false;
                    }
                });

                xhr.addEventListener('error', (e) => {
                    console.log('XHR error event:', e, 'uploadComplete:', uploadComplete);
                    // If upload was complete (100%), device reboot caused connection loss - this is expected
                    if (uploadComplete) {
                        console.log('Upload was complete, treating as success');
                        showSuccessMessage();
                    } else {
                        showStatus('✗ Network error. Please try again.', 'error');
                        uploadBtn.disabled = false;
                    }
                });

                xhr.open('POST', '/update');
                xhr.send(formData);

            } catch (error) {
                showStatus('✗ Error: ' + error.message, 'error');
                uploadBtn.disabled = false;
            }
        });

        function showSuccessMessage() {
            progressFill.style.width = '100%';
            progressText.textContent = '100%';
            showStatus('✓ Firmware updated successfully! Device will reboot in 5 seconds...', 'success');
            setTimeout(() => {
                showStatus('Now you can do what you want!', 'info');
            }, 8000);
        }

        function showStatus(message, type) {
            status.textContent = message;
            status.className = 'status ' + type;
        }
    </script>
</body>
</html>
)rawliteral";

// Initialize OTA state
void initOTAState() {
    sys_memset(&otaState, 0, sizeof(otaState));
    otaState.isUpdating = false;

    // Get current running image index
    int32_t res = rom_sys_status_get(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &otaState.runningIdx);
    if (res < 0) {
        Serial.print("Get sys running idx failed! (res = ");
        Serial.print(res);
        Serial.println(")");
        otaState.runningIdx = IMAGE_0;
    }

    // Determine new image address and size based on running index
    if (otaState.runningIdx == IMAGE_0) {
        otaState.newImgAddr = RE_IMG_1_OFFSET;
        otaState.imgSize = RE_IMG_1_END - RE_IMG_1_OFFSET;
    } else {
        otaState.newImgAddr = RE_IMG_0_OFFSET;
        otaState.imgSize = RE_IMG_1_OFFSET - RE_IMG_0_OFFSET;
    }
}

// Handle firmware upload
void handleFirmwareUpload() {
    HTTPUpload &upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        Serial.print("Upload Start: ");
        Serial.println(upload.filename.c_str());

        // Initialize OTA state
        initOTAState();
        otaState.isUpdating = true;
        otaState.offset = 0;
        otaState.eraseStartAddr = otaState.newImgAddr;
        otaState.totalBytes = upload.totalSize;

        // Validate file size (img_size check in ota_demo.c)
        if (upload.totalSize > otaState.imgSize) {
            Serial.println("Content too long!!!");
            server.send(413, "text/plain", "Content too long");
            return;
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        size_t recv_len = upload.currentSize;
        uint32_t offset = otaState.offset;
        uint32_t new_img_addr = otaState.newImgAddr;

        if (offset + recv_len > otaState.imgSize) {
            Serial.print("received too long: ");
            Serial.println(offset + recv_len);
            server.send(413, "text/plain", "received too long");
            return;
        }

        while(offset + new_img_addr + recv_len > otaState.eraseStartAddr) {
            int ret = raw_flash_erase(otaState.eraseStartAddr, FLASH_SECTOR_SIZE);
            if (ret != 0) {
                Serial.print("Flash erase failed at 0x");
                Serial.println(otaState.eraseStartAddr, HEX);
                server.send(500, "text/plain", "Flash erase failed");
                return;
            }
            otaState.eraseStartAddr += FLASH_SECTOR_SIZE;  // erase_start_addr += 0x1000
        }

        // Write to flash
        int ret = raw_flash_write((new_img_addr + offset), upload.buf, recv_len);
        if (ret != 0) {
            Serial.print("Flash write failed at 0x");
            Serial.println(new_img_addr + offset, HEX);
            server.send(500, "text/plain", "Flash write failed");
            return;
        }

        // Update offset
        otaState.offset += recv_len;
        otaState.bytesReceived += recv_len;
    } else if (upload.status == UPLOAD_FILE_END) {
        Serial.println("\nUpload complete");

        int32_t res = 0;
        res = rom_sys_set_img_flag(otaState.runningIdx, (IMG_FLAG_IA_MASK | IMG_FLAG_NEWER_MASK), (IMG_FLAG_IA_OK | IMG_FLAG_OLDER));
        res |= rom_sys_set_img_flag(!otaState.runningIdx, (IMG_FLAG_IA_MASK | IMG_FLAG_VERIFY_MASK | IMG_FLAG_NEWER_MASK), 0);
        res |= rom_sys_set_img_flag(!otaState.runningIdx, IMG_FLAG_NEWER_MASK, IMG_FLAG_NEWER);

        if (res != 0) {
            Serial.print("Set sys image status failed! (res = ");
            Serial.print(res);
            Serial.println(")");
            server.send(500, "text/plain", "Set image status failed");
            return;
        }

        otaState.isUpdating = false;

        Serial.println("Firmware update successful!");
        Serial.println("Rebooting in 5 seconds so new image can be used!");

        // Send response BEFORE any delays
        server.send(200, "text/plain", "Update successful");

        // Flush and close connection to ensure response is fully sent
        server.client().flush();
        delay(100);  // Short delay to allow data to be sent
        server.client().stop();

        // Long delay to ensure response is fully sent before reboot
        for (int i = 5; i > 0; i--) {
            Serial.print("Rebooting in ");
            Serial.print(i);
            Serial.println(" seconds...");
            delay(1000);
        }

        // Reboot the device
        eclic_system_reset();
    }
}

// Handle root page - serve OTA upload form
void handleRoot() {
    server.send_P(200, "text/html", otaHTML);
}

// Handle system info
void handleInfo() {
    // Refresh running image index
    rom_sys_status_get(SYS_RUNNING_IMG, LEN_SYS_RUNNING_IMG, &otaState.runningIdx);

    String json = "{";
    json += "\"chip\": \"GD32VW553\",";
    json += "\"runningImage\": \"IMAGE_" + String(otaState.runningIdx);
    json += "}";
    server.send(200, "application/json", json);
}

// Handle firmware info request
void handleFirmwareInfo() {
    String json = "{";
    json += "\"Built Date\": \"" + String(__DATE__) + " " + String(__TIME__) + "\",";
    json += "\"Device\": \"GD32VW553\"";
    json += "}";
    server.send(200, "application/json", json);
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n============= GD32VW553 OTA Update =============\n");

    // Initialize OTA state
    initOTAState();

    // Connect to WiFi
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Setup web server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/info", HTTP_GET, handleInfo);
    server.on("/firmware", HTTP_GET, handleFirmwareInfo);

    // Handle firmware upload
    server.on("/update", HTTP_POST,
        []() {
            // Response is handled in UPLOAD_FILE_END
        },
        handleFirmwareUpload
    );

    // Start server
    server.begin();
    Serial.println("/nHTTP server started");
    Serial.println("Open http://" + WiFi.localIP().toString() + "/ in your browser");
    Serial.println("System ready for OTA update\n");
}

void loop() {
    server.handleClient();
    delay(2);
}
