/*
 * LittleFSFullTest.ino
 *
 * LittleFS Full Feature Test Example
 *
 * Description:
 * - Comprehensive test of all LittleFS library features
 * - Demonstrates file and directory operations
 * - Tests file seek, read, write operations
 *
 * Features:
 * - Initialize filesystem
 * - Create directories and subdirectories
 * - Write to and read from files
 * - File seek operations
 * - Get file position and size
 * - Flush file data
 * - Check file/directory existence
 * - Rename files/directories
 * - Delete files/directories
 * - Remove directories with contents
 *
 * Serial Baud Rate: 115200
 */

#include <Arduino.h>
#include <LittleFS.h>

void printDirInfo(){
    lfs_scan_result_t scanResult;
    int fileCount = lfs_scan_and_store(NULL, &scanResult);

    Serial.print("     Total files(directories) found: ");
    Serial.println(fileCount);

    for (int i = 0; i < scanResult.count; i++) {
        const lfs_file_info_t* info = &scanResult.files[i];

         Serial.print("     ");
        for (int j = 0; j < info->depth; j++) {
            Serial.print("     ");
        }

        if (info->type == LFS_TYPE_DIR) {
             Serial.print("[DIR] ");
            Serial.println(info->name);
        } else {
            Serial.print("[FILE] ");
            Serial.print(info->name);
            Serial.print(" (");
            Serial.print(info->size);
            Serial.println(" bytes)");
        }
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n========================================");
    Serial.println("   LittleFS Full Feature Test");
    Serial.println("========================================\n");

    // ==================== Initialize Filesystem ====================
    Serial.println("[1] Initializing LittleFS...");
    if (!LittleFS.begin()) {
        Serial.println("     ERROR: LittleFS initialization failed!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("     SUCCESS: LittleFS initialized!\n");

    // ==================== Create Directory ====================
    Serial.println("[2] Creating directory '/testdir'...");
    if (LittleFS.mkdir("/testdir")) {
        Serial.println("     SUCCESS: Directory created\n");
    } else {
        Serial.println("     INFO: Directory may already exist\n");
    }
    printDirInfo();

    // ==================== Create Subdirectory ====================
    Serial.println("[3] Creating subdirectory '/testdir/subdir'...");
    if (LittleFS.mkdir("/testdir/subdir")) {
        Serial.println("     SUCCESS: Subdirectory created\n");
    } else {
        Serial.println("     INFO: Subdirectory may already exist\n");
    }
    printDirInfo();

    // ==================== Write to File ====================
    Serial.println("[4] Writing to file '/testdir/data.txt'...");
    fs::File writeFile = LittleFS.open("/testdir/data.txt", FILE_WRITE);
    if (writeFile) {
        writeFile.println("     Hello, LittleFS!");
        writeFile.println("     This is a comprehensive test.");
        writeFile.print("     Line 3: ");
        writeFile.println(12345);
        writeFile.print("     Float value: ");
        writeFile.println(3.14159, 4);
        writeFile.flush();  // Ensure data is written to storage
        writeFile.close();
        Serial.println("     SUCCESS: Data written to file\n");
    } else {
        Serial.println("     ERROR: Failed to create file!\n");
    }
    printDirInfo();

    // ==================== Append to File ====================
    Serial.println("[4.5] Appending data to file '/testdir/data.txt'...");
    fs::File appendFile = LittleFS.open("/testdir/data.txt", FILE_APPEND);
    if (appendFile) {
        appendFile.println("     This line is appended.");
        appendFile.close();
        Serial.println("     SUCCESS: Data appended to file\n");
    } else {
        Serial.println("     ERROR: Failed to open file for appending!\n");
    }
    printDirInfo();

    // ==================== Check File Existence ====================
    Serial.println("[5] Checking file existence...");
    if (LittleFS.exists("/testdir/data.txt")) {
        Serial.println("     SUCCESS: File exists\n");
    } else {
        Serial.println("     ERROR: File does not exist!\n");
    }
    printDirInfo();
    // ==================== Check Directory Existence ====================
    Serial.println("[6] Checking directory existence...");
    if (LittleFS.exists("/testdir")) {
        Serial.println("     SUCCESS: Directory exists\n");
    } else {
        Serial.println("     ERROR: Directory does not exist!\n");
    }
    printDirInfo();
    // ==================== Read from File ====================
    Serial.println("[7] Reading from file '/testdir/data.txt'...");
    fs::File readFile = LittleFS.open("/testdir/data.txt", FILE_READ);
    if (readFile) {
        Serial.println("     --- File Content ---");
        while (readFile.available()) {
            Serial.write(readFile.read());
        }
        Serial.println("     --- End of Content ---");

        // Display file size
           Serial.print("     File size: ");
         Serial.print(readFile.size());
        Serial.println(" bytes\n");

        readFile.close();
    } else {
        Serial.println("     ERROR: Failed to open file for reading!\n");
    }
    printDirInfo();
    // ==================== Test File Position ====================
    Serial.println("[8] Testing file position operations...");
    fs::File posFile = LittleFS.open("/testdir/data.txt", FILE_READ);
    if (posFile) {
        Serial.print("     Initial position: ");
        Serial.println(posFile.position());

        // Read some bytes
        char buffer[20];
        int bytesRead = posFile.read((uint8_t*)buffer, 10);
        buffer[bytesRead] = '\0';
        Serial.print("     After reading 10 bytes: ");
        Serial.println(posFile.position());

        // Seek to specific position
        posFile.seek(0);  // Seek to beginning
        Serial.print("     After seek(0): ");
        Serial.println(posFile.position());

        posFile.seek(10);  // Seek to position 10
        Serial.print("     After seek(10): ");
        Serial.println(posFile.position());

        posFile.close();
        Serial.println();
    }
    printDirInfo();
    // ==================== Test Seek from End ====================
    Serial.println("[9] Testing seek from end...");
    fs::File seekFile = LittleFS.open("/testdir/data.txt", FILE_READ);
    if (seekFile) {
        size_t fileSize = seekFile.size();
        Serial.print("     File size: ");
        Serial.println(fileSize);

        // Seek to 10 bytes before end
        if (seekFile.seek(fileSize - 10, fs::SeekSet)) {
             Serial.print("     Last 10 bytes: ");
            while (seekFile.available()) {
                Serial.write(seekFile.read());
            }
            Serial.println();
        }

        seekFile.close();
        Serial.println();
    }
    printDirInfo();
    // ==================== Rename File ====================
    Serial.println("[10] Renaming file...");
    if (LittleFS.exists("/testdir/data.txt")) {
        if (LittleFS.rename("/testdir/data.txt", "/testdir/data_renamed.txt")) {
            Serial.println("     SUCCESS: File renamed to '/testdir/data_renamed.txt'\n");
        } else {
            Serial.println("     ERROR: Failed to rename file\n");
        }
    }
    printDirInfo();

    // ==================== Verify Rename ====================
    Serial.println("[11] Verifying rename...");
    if (LittleFS.exists("/testdir/data_renamed.txt")) {
        Serial.println("     SUCCESS: Renamed file exists");
    }
    if (!LittleFS.exists("/testdir/data.txt")) {
        Serial.println("     SUCCESS: Original file no longer exists\n");
    }
    printDirInfo();
    // ==================== Write to Another File ====================
    Serial.println("[12] Writing to '/testdir/subdir/nested.txt'...");
    fs::File nestedFile = LittleFS.open("/testdir/subdir/nested.txt", FILE_WRITE);
    if (nestedFile) {
        nestedFile.println("This is a nested file.");
        nestedFile.close();
        Serial.println("     SUCCESS: Nested file created\n");
    }
    printDirInfo();

    // ==================== List Directory Contents ====================
    Serial.println("[13] Listing directory contents using scan and store...");
    printDirInfo();

    // ==================== Delete Single File ====================
    Serial.println("[14] Deleting single file '/testdir/subdir/nested.txt'...");
    if (LittleFS.remove("/testdir/subdir/nested.txt")) {
        Serial.println("     SUCCESS: File deleted\n");
    } else {
        Serial.println("     ERROR: Failed to delete file\n");
    }
    printDirInfo();

    // ==================== Delete Directory with Contents ====================
    Serial.println("[15] Deleting directory '/testdir' with all contents...");
    if (LittleFS.rmdir("/testdir")) {
        Serial.println("     SUCCESS: Directory and all contents deleted\n");
    } else {
        Serial.println("     ERROR: Failed to delete directory\n");
    }
    printDirInfo();

    // ==================== Verify Deletion ====================
    Serial.println("[16] Verifying deletion...");
    if (!LittleFS.exists("/testdir")) {
        Serial.println("     SUCCESS: Directory no longer exists\n");
    } else {
        Serial.println("     ERROR: Directory still exists!\n");
    }
    printDirInfo();
    // ==================== Create Multiple Files Test ====================
    Serial.println("[17] Creating multiple files for testing...");
    LittleFS.mkdir("/multitest");
    for (int i = 0; i < 3; i++) {
        char filename[32];
        sprintf(filename, "/multitest/file%d.txt", i);
        fs::File f = LittleFS.open(filename, FILE_WRITE);
        if (f) {
            f.print("File number ");
            f.println(i);
            f.close();
        }
    }
    Serial.println("     SUCCESS: 3 files created\n");
    printDirInfo();

    // ==================== Test Flush Operation ====================
    Serial.println("[18] Testing flush operation...");
    fs::File flushFile = LittleFS.open("/multitest/file0.txt", FILE_WRITE);
    if (flushFile) {
        flushFile.println("Data before flush");
        flushFile.flush();  // Explicit flush
        Serial.println("     SUCCESS: Data flushed to storage");
        flushFile.close();
    }
    printDirInfo();

    // ==================== Cleanup ====================
    Serial.println("[19] Cleaning up test files...");
    LittleFS.rmdir("/multitest");

    if(LittleFS.exists("/multitest")){
        Serial.println("     FAIL: Cleanup failed");
    }else{
        Serial.println("     SUCCESS: Cleanup complete");
    }
    printDirInfo();

    Serial.println("========================================");
    Serial.println("   All Tests Completed!");
    Serial.println("========================================");
}

void loop() {
    delay(10000);
}