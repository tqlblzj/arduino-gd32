#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include "LittleFS.h"
#include "FS.h"
#include "gdlittlefs.h"

extern "C" {
#include "lfs.h"
}

char *ssid = "Testing-WIFI";
char *password = "Testwifi2020";
const char *host = "gd32fs";
WebServer server(80);
//holds the current upload
fs::File fsUploadFile;

void printFSInfo(){
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

void fsPrepareData(bool reset=false){
  Serial.println("[1] Initializing LittleFS...");
  if (!LittleFS.begin()) {
      Serial.println("     ERROR: LittleFS initialization failed!");
      while (1) {
          delay(1000);
      }
  }
  Serial.println("     SUCCESS: LittleFS initialized!\n");

  if(reset){
    LittleFS.rmdir("/");
    Serial.println("     You Choose to Reset FS!!!!!!!!!\n");
    printFSInfo();
  }

  // ==================== Create Directory ====================
  Serial.println("[2] Creating directory '/testdir'...");
  if (LittleFS.mkdir("/testdir")) {
      Serial.println("     SUCCESS: Directory created\n");
  } else {
      Serial.println("     INFO: Directory may already exist\n");
  }
  printFSInfo();

  // ==================== Write to File ====================
  Serial.println("[3] Writing to file '/testdir/txt/data.txt'...");
  fs::File writeFile = LittleFS.open("/testdir/txt/data.txt", FILE_WRITE);
  if (writeFile) {
      writeFile.println("Hello, LittleFS!");
      writeFile.println("This is a comprehensive test.");
      writeFile.print("Line 3: ");
      writeFile.println(12345);
      writeFile.print("Float value: ");
      writeFile.println(3.14159, 4);
      writeFile.flush();  // Ensure data is written to storage
      writeFile.close();
      Serial.println("     SUCCESS: Data written to file\n");
  } else {
      Serial.println("     ERROR: Failed to create file!\n");
  }
  printFSInfo();

  // ==================== Append to File ====================
  Serial.println("[4] Appending data to file '/testdir/txt/data.txt'...");
  fs::File appendFile = LittleFS.open("/testdir/txt/data.txt", FILE_APPEND);
  if (appendFile) {
      appendFile.println("This line is appended.");
      appendFile.close();
      Serial.println("     SUCCESS: Data appended to file\n");
  } else {
      Serial.println("     ERROR: Failed to open file for appending!\n");
  }
  printFSInfo();

  // ==================== Write HTML File ====================
  Serial.println("[5] Writing HTML file '/testdir/html/test.html'...");
  fs::File htmlFile = LittleFS.open("/testdir/html/test.html", FILE_WRITE);
  if (htmlFile) {
      htmlFile.println("<!DOCTYPE html>");
      htmlFile.println("<html>");
      htmlFile.println("<head>");
      htmlFile.println("  <meta charset=\"UTF-8\">");
      htmlFile.println("  <title>GD32 Test Page</title>");
      htmlFile.println("  <style>");
      htmlFile.println("    body { font-family: Arial; margin: 20px; }");
      htmlFile.println("    h1 { color: #2c3e50; }");
      htmlFile.println("    .info { background: #ecf0f1; padding: 15px; border-radius: 5px; }");
      htmlFile.println("  </style>");
      htmlFile.println("</head>");
      htmlFile.println("<body>");
      htmlFile.println("  <h1>GD32 LittleFS Test</h1>");
      htmlFile.println("  <div class=\"info\">");
      htmlFile.println("    <p><strong>Chip:</strong> GD32VW553</p>");
      htmlFile.println("    <p><strong>Flash:</strong> 2MB SPI Flash</p>");
      htmlFile.println("    <p><strong>File System:</strong> LittleFS</p>");
      htmlFile.println("  </div>");
      htmlFile.println("  <p>Current time: <span id=\"time\"></span></p>");
      htmlFile.println("  <script>");
      htmlFile.println("    document.getElementById('time').innerText = new Date().toLocaleString();");
      htmlFile.println("  </script>");
      htmlFile.println("</body>");
      htmlFile.println("</html>");
      htmlFile.flush();
      htmlFile.close();
      Serial.println("     SUCCESS: HTML file created\n");
  } else {
      Serial.println("     ERROR: Failed to create HTML file!\n");
  }
  printFSInfo();

  // ==================== Write GZ Compressed File ====================
  Serial.println("[6] Writing GZ compressed file '/testdir/gz/test.txt.gz'...");
  fs::File gzFile = LittleFS.open("/testdir/gz/test.txt.gz", FILE_WRITE);
  if (gzFile) {
      // Write some compressed data (in real use, pre-compress your data)
      // This is a simple example showing how to write binary data
      const char* sampleData = "This is a test file for GZ compression.\n";
      gzFile.write((const uint8_t*)sampleData, strlen(sampleData));
      gzFile.flush();
      gzFile.close();
      Serial.println("     SUCCESS: GZ file created\n");
      Serial.println("     NOTE: In production, use pre-compressed .gz files\n");
  } else {
      Serial.println("     ERROR: Failed to create GZ file!\n");
  }

  printFSInfo();
}

//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool canReadFile(String path) {
  bool canReadFile = false;

  if(!LittleFS.exists(path)){
    canReadFile = false;
    return canReadFile;
  }

  struct lfs_info info;
  LittleFS.state(path.c_str(), (void *)&info);

  // we can not read a directory
  if(info.type == LFS_TYPE_DIR) canReadFile = false;
  else if(info.type == LFS_TYPE_REG) canReadFile = true;

  return canReadFile;
}

void printFileContent(fs::File* File){
  Serial.println("--- File Content ---");
  while (File->available()) {
      Serial.write(File->read());
  }
  Serial.println("--- End of Content ---");

  // Display file size
  Serial.print("File size: ");
  Serial.print(File->size());
  Serial.println(" bytes\n");
}

bool handleFileRead(String path) {
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "index.htm";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (canReadFile(pathWithGz) || canReadFile(path)) {
    if (canReadFile(pathWithGz)) {
      path += ".gz";
    }
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    Serial.println("File " + path + " Transfer Complete");
    server.send(200, "FILE READED");
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);
    fsUploadFile = LittleFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.flush();
      fsUploadFile.close();
      fsUploadFile = LittleFS.open(upload.filename, "r");
      Serial.println("Upload data: ");
      printFileContent(&fsUploadFile);
      fsUploadFile.close();
      printFSInfo();
    }
    Serial.print("handleFileUpload Size: ");
    Serial.println(upload.totalSize);
    server.send(200, "text/plain", "UPLOAD FINISHED");
  }
}

void handleFileDelete() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!LittleFS.exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  LittleFS.remove(path);
  server.send(200, "text/plain", "FILE DELETED");
  path = String();
  printFSInfo();
}

void handleFileAppend() {
  if (server.args() < 2) {
    return server.send(500, "text/plain", "BAD ARGS: need path and data");
  }
  String path = server.arg("path");
  String data = server.arg("data");
  Serial.println("handleFileAppend: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!LittleFS.exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  File file = LittleFS.open(path, FILE_APPEND);
  if (file) {
    file.println(data);
    file.close();
    server.send(200, "text/plain", "");
    file = LittleFS.open(path, FILE_READ);
    Serial.println("After appending: ");
    printFileContent(&file);
    file.close();
    printFSInfo();
    server.send(200, "text/plain", "TEXT APPENDED");
  } else {
    return server.send(500, "text/plain", "APPEND FAILED");
  }
}

void handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (LittleFS.exists(path)) {
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  File file = LittleFS.open(path, "w");
  if (file) {
    file.close();
    server.send(500, "text/plain", "FILE CREATED");
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.send(200, "text/plain", "");
  path = String();
  printFSInfo();
}

void handleFileList() {
  String path = "/";
  if (server.hasArg("dir")) {
    path = server.arg("dir");
  }
  Serial.println("handleFileList: " + path);

  lfs_scan_result_t result;
  int count = lfs_scan_and_store(path.c_str(), &result);

  if (count < 0) {
    server.send(500, "text/plain", "FAILED");
    return;
  }

  if(count == 0){
    server.send(200, "text/plain", "[]");
  }

  String output = "[";
  for (uint16_t i = 0; i < result.count; i++) {
    if (output != "[") {
      output += ',';
    }
    output += "{\"type\":\"";
    output += (result.files[i].type == LFS_TYPE_DIR) ? "dir" : "file";
    output += "\",\path\":\"";
    output += result.files[i].path;
    output += "\",\"name\":\"";

    const char* fileName = result.files[i].name;
    if (fileName[0] == '/') {
      fileName++;
    }
    output += fileName;
    output += "\"}";
  }
  output += "]";
  server.send(200, "text/json", output);
}

void setup(void) {
  Serial.begin(115200);
  Serial.print("\n");
  LittleFS.begin();

  // use fsPrepareData(true) to reset fs (clear all dir and files)
  fsPrepareData(true);

  //WIFI INIT
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  //SERVER INIT
  //list directories and files
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/testdir/html/test.htm")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  //create file
  //example: curl -X PUT "http://192.168.22.100/edit?path=/testdir/txt/data.txt"
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  //example: curl -X DELETE "http://192.168.22.100/edit?path=/testdir/txt/data.txt"
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //append to file
  //example: curl -X PATCH "http://192.168.22.100/edit?path=/testdir/txt/data.txt&data=Hello"
  server.on("/edit", HTTP_PATCH, handleFileAppend);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  //example: curl -X POST "http://192.168.22.100/edit" -F "file=@D:\personal\lzj\test.txt;filename=/testdir/upload/upload.txt"
  server.on(
    "/edit", HTTP_POST,
    []() {
      server.send(200, "text/plain", "");
    },
    handleFileUpload
  );

  //called when the url is not defined here
  //use it to load content from LittleFS
  //example: curl -X GET "http://192.168.22.102/edit?path=/testdir/txt/data.txt"
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound or you try to read a directory!!!");
    }
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2);
}
