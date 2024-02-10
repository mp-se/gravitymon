/*
MIT License

Copyright (c) 2023-2024 Magnus

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */
#include <log.hpp>
#include <serialws.hpp>

void SerialWebSocket::begin(AsyncWebServer *server, Print *secondary) {
  Log.notice(F("WS  : Starting serial websocket" CR));
  _server = server;
  _secondayLog = secondary;
  _webSocket = new AsyncWebSocket("/serialws");
  _server->addHandler(_webSocket);
}

size_t SerialWebSocket::write(uint8_t c) {
  _buf[_bufSize++] = c;

  if (_bufSize >= sizeof(_buf) || c == '\n') {
    flush();
  }

  return sizeof(c);
}

void SerialWebSocket::flush() {
  if (_secondayLog) _secondayLog->write(_buf, _bufSize);

  if (_webSocket->count()) {
    // Only send data to socket if there are connected clients
    _webSocket->textAll(reinterpret_cast<const char *>(_buf), _bufSize);
  }

  memset(_buf, 0, sizeof(_buf));
  _bufSize = 0;
}

// EOF
