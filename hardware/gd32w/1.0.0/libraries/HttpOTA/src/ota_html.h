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

#ifndef HTTPOTA_HTML_H
#define HTTPOTA_HTML_H

// HTML page for HttpOTA upload
const char httpotaHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GD32VW553 HttpOTA Update</title>
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
        <h1>🔄 HttpOTA Update</h1>
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



#endif // HTTPOTA_HTML_H
