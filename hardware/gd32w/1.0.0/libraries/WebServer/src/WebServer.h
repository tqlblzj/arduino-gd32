/*
  WebServer.h - Dead simple web-server.
  Supports only one simultaneous client, knows how to handle GET and POST.

  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.

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

  Modified 8 May 2015 by Hristo Gochkov (proper post and file upload handling)
  Modified 12 Feb 2026 by GigaDevice Semiconductor Inc (adapt to the first version of gd32 arduino sdk).
*/

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <functional>
#include <memory>
#include "FS.h"
#include "Network.h"
#include "middleware/Middleware.h"
#include "detail/RequestHandler.h"
#include "detail/mimetable.h"
#include "HTTP_Method.h"
#include "Uri.h"

using namespace fs;

#ifdef send
#undef send
#endif

enum HTTPUploadStatus {
  UPLOAD_FILE_START,
  UPLOAD_FILE_WRITE,
  UPLOAD_FILE_END,
  UPLOAD_FILE_ABORTED
};
enum HTTPRawStatus {
  RAW_START,
  RAW_WRITE,
  RAW_END,
  RAW_ABORTED
};

enum HTTPClientStatus {
  HC_NONE,
  HC_WAIT_READ,
  HC_WAIT_CLOSE
};

#ifndef HTTP_UPLOAD_BUFLEN
#define HTTP_UPLOAD_BUFLEN 1436
#endif

#ifndef HTTP_RAW_BUFLEN
#define HTTP_RAW_BUFLEN 1436
#endif

#define HTTP_MAX_DATA_WAIT      5000  //ms to wait for the client to send the request
#define HTTP_MAX_POST_WAIT      5000  //ms to wait for POST data to arrive
#define HTTP_MAX_SEND_WAIT      5000  //ms to wait for data chunk to be ACKed
#define HTTP_MAX_CLOSE_WAIT     5000  //ms to wait for the client to close the connection
#define HTTP_MAX_BASIC_AUTH_LEN 256   // maximum length of a basic Auth base64 encoded username:password string

#define CONTENT_LENGTH_UNKNOWN ((size_t) - 1)
#define CONTENT_LENGTH_NOT_SET ((size_t) - 2)

struct HTTPUpload {
  HTTPUploadStatus status;
  String filename;
  String name;
  String type;
  size_t totalSize;    // file size
  size_t currentSize;  // size of data currently in buf
  uint8_t buf[HTTP_UPLOAD_BUFLEN];
  bool needAuthBeforeUpload;
};

struct HTTPRaw {
  HTTPRawStatus status;
  size_t totalSize;    // content size
  size_t currentSize;  // size of data currently in buf
  uint8_t buf[HTTP_RAW_BUFLEN];
  void *data;  // additional data
};

enum HTTPAuthMethod {
  BASIC_AUTH,
  DIGEST_AUTH,
  OTHER_AUTH
};

class WebServer{
public:
  WebServer(IPAddress addr, int port = 80);
  WebServer(int port = 80);
  virtual ~WebServer();

  virtual void begin();
  virtual void begin(uint16_t port);
  virtual void handleClient();

  virtual void close();
  void stop();

  const String AuthTypeDigest = F("Digest");
  const String AuthTypeBasic = F("Basic");

  void chunkResponseBegin(const char *contentType = "text/plain");
  void chunkWrite(const char *data, size_t length);
  void chunkResponseEnd();

  typedef std::function<String *(HTTPAuthMethod mode, String enteredUsernameOrReq, String extraParams[])> THandlerFunctionAuthCheck;
  bool authenticate(THandlerFunctionAuthCheck fn);
  bool authenticate(const char *username, const char *password);
  bool authenticateBasicSHA1(const char *_username, const char *_sha1AsBase64orHex);
  void requestAuthentication(HTTPAuthMethod mode = BASIC_AUTH, const char *realm = NULL, const String &authFailMsg = String(""));

  void send(int code, const char *content_type = NULL, const String &content = String(""));
  void send(int code, char *content_type, const String &content);
  void send(int code, const String &content_type, const String &content);
  void send(int code, const char *content_type, const char *content);

  void send_P(int code, PGM_P content_type, PGM_P content);
  void send_P(int code, PGM_P content_type, PGM_P content, size_t contentLength);

  void sendContent(const String &content);
  void sendContent(const char *content, size_t contentLength);
  void sendHeader(const String &name, const String &value, bool first = false);

  void sendContent_P(PGM_P content);
  void sendContent_P(PGM_P content, size_t size);

  // Bring handler function types into class scope for compatibility
  typedef std::function<void(void)> THandlerFunction;
  typedef std::function<bool(class WebServer &server)> FilterFunction;
  typedef std::function<String(fs::FS &fs, const String &fName)> ETagFunction;
  RequestHandler &on(const Uri &uri, THandlerFunction fn);
  RequestHandler &on(const Uri &uri, HTTPMethod method, THandlerFunction fn);
  RequestHandler &on(const Uri &uri, HTTPMethod method, THandlerFunction fn, THandlerFunction ufn);  //ufn handles file uploads

  void onNotFound(THandlerFunction fn);     //called when handler is not assigned
  const String version() const;     // get the HTTP version string
  static String responseCodeToString(int code);

  void setContentLength(const size_t contentLength);

  template<typename T> size_t streamFile(T &file, const String &contentType, const int code = 200) {
    _streamFileCore(file.size(), file.name(), contentType, code);
    return _currentClient.write(file);
  }

  WebServer &addMiddleware(Middleware *middleware);
  WebServer &addMiddleware(Middleware::Function fn);
  WebServer &removeMiddleware(Middleware *middleware);

  String uri() const {
      return _currentUri;
  }

  HTTPMethod method() const {
      return _currentMethod;
  }

  int WebServer::args() const {
      return _currentArgCount;
  }

  HTTPUpload &upload() {
    return *_currentUpload;
  }

  HTTPRaw &raw() {
    return *_currentRaw;
  }

  virtual NetworkClient &client() {
    return _currentClient;
  }

  String pathArg(unsigned int i) const;                                         // get request path argument by number
  String argName(int i) const;                                                  // get request argument name by number
  String arg(int i) const;                                                      // get request argument value by number
  String arg(const String &name) const;                                         // get request argument value by name
  void collectAllHeaders();                                                     // collect all request headers
  bool hasArg(const String &name) const;                                        // check if argument exists
  String header(const String &name) const;                                      // get request header value by name
  String header(int i) const;                                                   // get request header value by number
  void collectHeaders(const char *headerKeys[], const size_t headerKeysCount);  // set the request headers to collect
  bool hasHeader(const String &name) const;                                     // check if header exists
  int headers() const;                                                          // get header count
  String headerName(int i) const;                                               // get request header name by number

  // Response methods
  int responseCode() const { return _responseCode; }
  int responseHeaders() const { return _responseHeaderCount; }
  String responseHeaderName(int i) const;
  String responseHeader(int i) const;

  static String urlDecode(const String &text);
  bool _eTagEnabled = false;
  ETagFunction _eTagFunction = nullptr;

private:
  bool _chunkedResponseActive = false;
  // NetworkClient _chunkedClient;  // Store by value, no dangling pointer

protected:
  virtual size_t _currentClientWrite(const char *b, size_t l) {
      return _currentClient.write((const uint8_t *)b, l);
  }
  virtual size_t _currentClientWrite_P(PGM_P b, size_t l) {
    return _currentClient.write((const uint8_t *)b, l);
  }

  bool _parseRequest(NetworkClient &client);
  void _prepareHeader(String &response, int code, const char *content_type, size_t contentLength);

  void _clearResponseHeaders();
  void _clearRequestHeaders();
  bool _handleRequest();
  bool _collectHeader(const char *headerName, const char *headerValue);
  void _parseArguments(const String &data);
  bool _parseForm(NetworkClient &client, const String &boundary, uint32_t len);
  void _finalizeResponse();
  void _uploadWriteByte(uint8_t b);
  int _uploadReadByte(NetworkClient &client);
  bool _parseFormUploadAborted();
  void _addRequestHandler(RequestHandler *handler);
  void _streamFileCore(const size_t fileSize, const String &fileName, const String &contentType, const int code = 200);
  String _extractParam(String &authReq, const String &param, const char delimit = '"');
  String _getRandomHexString();

  struct RequestArgument {
      String key;
      String value;
      RequestArgument *next;
  };

  NetworkServer _server;
  int _headerKeysCount = 0;

  NetworkClient _currentClient;
  HTTPClientStatus _currentStatus = HC_NONE;
  unsigned long _statusChange = 0;
  bool _nullDelay = true;

  size_t _contentLength = 0;

  int _responseCode = 0;
  int _responseHeaderCount = 0;
  RequestArgument *_responseHeaders = nullptr;

  RequestHandler *_currentHandler = nullptr;
  RequestHandler *_firstHandler = nullptr;
  RequestHandler *_lastHandler = nullptr;
  THandlerFunction _notFoundHandler = nullptr;
  THandlerFunction _fileUploadHandler = nullptr;

  MiddlewareChain *_chain = nullptr;

  std::unique_ptr<HTTPUpload> _currentUpload;
  std::unique_ptr<HTTPRaw> _currentRaw;

  HTTPMethod _currentMethod = HTTP_ANY;
  String _currentUri;
  int _currentArgCount = 0;
  RequestArgument *_currentArgs = nullptr;
  RequestArgument *_currentHeaders = nullptr;
  uint8_t _currentVersion = 0;

  bool _collectAllHeaders = false;
  bool _chunked = false;
  int _clientContentLength = 0;  // "Content-Length" from header of incoming POST or GET request
  String _hostHeader;
  RequestArgument *_postArgs = nullptr;
  int _postArgsLen = 0;
  bool _corsEnabled = false;

  String _snonce;  // Store noance and opaque for future comparison
  String _sopaque;
  String _srealm;  // Store the Auth realm between Calls

};

#endif //WEBSERVER_H