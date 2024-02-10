/*
MIT License

Copyright (c) 2021-2024 Magnus

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
#if defined(ESP8266)
#include <ESP8266mDNS.h>
#else
#include <ESPmDNS.h>
#endif
#include <MQTT.h>

#include <basepush.hpp>
#include <log.hpp>
#include <utils.hpp>

void BasePush::probeMFLN(String serverPath) {
#if defined(ESP8266)
  // serverPath=: http:://servername:port/path
  int port = 443;                         // default SSL port
  String host = serverPath.substring(8);  // remove the prefix or the probe will
                                          // fail, it needs a pure host name.
  int idx = host.indexOf("/");
  if (idx != -1) host = host.substring(0, idx);

  idx =
      host.indexOf(":");  // If a server port is defined, lets extract that part
  if (idx != -1) {
    String p = host.substring(idx + 1);
    port = p.toInt();
    host = host.substring(0, idx);
  }

  Log.notice(F("PUSH: Probing server to max fragment %s:%d" CR), host.c_str(),
             port);
  if (_wifiSecure.probeMaxFragmentLength(host, port, 512)) {
    Log.notice(F("PUSH: Server supports smaller SSL buffer." CR));
    _wifiSecure.setBufferSizes(512, 512);
  }
#endif
}

void BasePush::sendInfluxDb2(String& payload, const char* target,
                             const char* org, const char* bucket,
                             const char* token) {
  Log.notice(F("PUSH: Sending values to influxdb2." CR));
  _lastResponseCode = 0;
  _lastSuccess = false;

  String serverPath = String(target) + "/api/v2/write?org=" + String(org) +
                      "&bucket=" + String(bucket);

#if LOG_LEVEL == 6
  Log.verbose(F("PUSH: url %s." CR), serverPath.c_str());
  Log.verbose(F("PUSH: data %s." CR), payload.c_str());
#endif

  String authHeader = "Token " + String(token);

  if (isSecure(target)) {
    Log.notice(F("PUSH: InfluxDB, SSL enabled without validation." CR));
    _wifiSecure.setInsecure();
    probeMFLN(serverPath);
    _httpSecure.setTimeout(_config->getPushTimeout() * 1000);
    _httpSecure.begin(_wifiSecure, serverPath);
    _httpSecure.addHeader(F("Authorization"), authHeader.c_str());
    _lastResponseCode = _httpSecure.POST(payload);
  } else {
    _http.setTimeout(_config->getPushTimeout() * 1000);
    _http.begin(_wifi, serverPath);
    _http.addHeader(F("Authorization"), authHeader.c_str());
    _lastResponseCode = _http.POST(payload);
  }

  if (_lastResponseCode == 204) {
    _lastSuccess = true;
    Log.notice(F("PUSH: InfluxDB2 push successful, response=%d" CR),
               _lastResponseCode);
  } else {
    Log.notice(F("PUSH: InfluxDB2 push failed, response=%d" CR),
               _lastResponseCode);
  }

  if (isSecure(target)) {
    _httpSecure.end();
    _wifiSecure.stop();
  } else {
    _http.end();
    _wifi.stop();
  }

  tcp_cleanup();
}

void BasePush::addHttpHeader(HTTPClient& http, String header) {
  // header=: Header: details

  if (!header.length()) return;

  int i = header.indexOf(":");
  if (i) {
    String name = header.substring(0, i);
    String value = header.substring(i + 1);
    value.trim();
    Log.notice(F("PUSH: Adding header '%s': '%s'" CR), name.c_str(),
               value.c_str());
    http.addHeader(name, value);
  } else {
    Log.error(F("PUSH: Unable to set header, invalid value %s" CR), header);
  }
}

String BasePush::sendHttpPost(String& payload, const char* target,
                              const char* header1, const char* header2) {
  Log.notice(F("PUSH: Sending values to HTTP post" CR));
  _lastResponseCode = 0;
  _lastSuccess = false;
  String _response = "{}";

#if LOG_LEVEL == 6
  Log.verbose(F("PUSH: url %s." CR), target);
  Log.verbose(F("PUSH: json %s." CR), payload.c_str());
#endif

  if (isSecure(target)) {
    Log.notice(F("PUSH: HTTP, SSL enabled without validation." CR));
    _wifiSecure.setInsecure();
    probeMFLN(target);
    _httpSecure.setTimeout(_config->getPushTimeout() * 1000);
    _httpSecure.begin(_wifiSecure, target);
    addHttpHeader(_httpSecure, header1);
    addHttpHeader(_httpSecure, header2);
    _lastResponseCode = _httpSecure.POST(payload);
  } else {
    _http.setTimeout(_config->getPushTimeout() * 1000);
    _http.begin(_wifi, target);
    addHttpHeader(_http, header1);
    addHttpHeader(_http, header2);
    _lastResponseCode = _http.POST(payload);
  }

  if (_lastResponseCode == 200) {
    _lastSuccess = true;
    Log.notice(F("PUSH: HTTP post successful, response=%d" CR),
               _lastResponseCode);
    if (isSecure(target)) {
      _response = _httpSecure.getString();
    } else {
      _response = _http.getString();
    }
  } else {
    Log.error(F("PUSH: HTTP post failed, response=%d" CR), _lastResponseCode);
  }

  if (isSecure(target)) {
    _httpSecure.end();
    _wifiSecure.stop();
  } else {
    _http.end();
    _wifi.stop();
  }

  tcp_cleanup();
  return _response;
}

String BasePush::sendHttpGet(String& payload, const char* target,
                             const char* header1, const char* header2) {
  Log.notice(F("PUSH: Sending values via HTTP get" CR));
  _lastResponseCode = 0;
  _lastSuccess = false;
  String _response = "{}";

  String url = String(target) + payload;

#if LOG_LEVEL == 6
  Log.verbose(F("PUSH: url %s." CR), url.c_str());
#endif

  if (isSecure(target)) {
    Log.notice(F("PUSH: HTTP, SSL enabled without validation." CR));
    _wifiSecure.setInsecure();
    probeMFLN(target);
    _httpSecure.setTimeout(_config->getPushTimeout() * 1000);
    _httpSecure.begin(_wifiSecure, url);
    addHttpHeader(_httpSecure, header1);
    addHttpHeader(_httpSecure, header2);
    _lastResponseCode = _httpSecure.GET();
  } else {
    _http.setTimeout(_config->getPushTimeout() * 1000);
    _http.begin(_wifi, url);
    addHttpHeader(_http, header1);
    addHttpHeader(_http, header2);
    _lastResponseCode = _http.GET();
  }

  if (_lastResponseCode == 200) {
    _lastSuccess = true;
    Log.notice(F("PUSH: HTTP get successful, response=%d" CR),
               _lastResponseCode);
    if (isSecure(target)) {
      _response = _httpSecure.getString();
    } else {
      _response = _http.getString();
    }
  } else {
    Log.error(F("PUSH: HTTP get failed, response=%d" CR), _lastResponseCode);
  }

  if (isSecure(target)) {
    _httpSecure.end();
    _wifiSecure.stop();
  } else {
    _http.end();
    _wifi.stop();
  }

  tcp_cleanup();
  return _response;
}

void BasePush::sendMqtt(String& payload, const char* target, int port,
                        const char* user, const char* pass) {
  // payload=: topic1,data1|topic2,data2|...

  Log.notice(F("PUSH: Sending values to mqtt." CR));
  _lastResponseCode = 0;
  _lastSuccess = false;

  MQTTClient mqtt(512);

  if (port > 8000) {
    Log.notice(F("PUSH: MQTT, SSL enabled without validation." CR));
    _wifiSecure.setInsecure();

#if defined(ESP8266)
    if (_wifiSecure.probeMaxFragmentLength(target, port, 512)) {
      Log.notice(F("PUSH: MQTT server supports smaller SSL buffer." CR));
      _wifiSecure.setBufferSizes(512, 512);
    }
#endif

    mqtt.setTimeout(_config->getPushTimeout() * 1000);
    mqtt.begin(target, port, _wifiSecure);
  } else {
    mqtt.setTimeout(_config->getPushTimeout() * 1000);
    mqtt.begin(target, port, _wifi);
  }

  mqtt.connect(_config->getMDNS(), user, pass);

#if LOG_LEVEL == 6
  Log.verbose(F("PUSH: url %s." CR), target);
  Log.verbose(F("PUSH: data %s." CR), payload.c_str());
#endif

  int lines = 1;
  // Find out how many lines are in the document. Each line is one
  // topic/message. | is used as new line.
  for (unsigned int i = 0; i < payload.length() - 1; i++) {
    if (payload.charAt(i) == '|') lines++;
  }

  int index = 0;
  while (lines) {
    int next = payload.indexOf('|', index);
    String line = payload.substring(index, next);

    // Each line equals one topic post, format is <topic>:<value>
    String topic = line.substring(0, line.indexOf(":"));
    String value = line.substring(line.indexOf(":") + 1);
#if LOG_LEVEL == 6
    Log.verbose(F("PUSH: topic '%s', value '%s'." CR), topic.c_str(),
                value.c_str());
#endif
    if (mqtt.publish(topic, value)) {
      _lastSuccess = true;
      Log.notice(F("PUSH: MQTT publish successful on %s" CR), topic.c_str());
      _lastResponseCode = 0;
    } else {
      _lastResponseCode = mqtt.lastError();
      Log.error(F("PUSH: MQTT publish failed on %s with error %d" CR),
                topic.c_str(), mqtt.lastError());
    }

    index = next + 1;
    lines--;
  }

  mqtt.disconnect();

  if (port > 8000) {
    _wifiSecure.stop();
  } else {
    _wifi.stop();
  }

  tcp_cleanup();
}

bool BasePush::sendHttpPost(String& payload) {
  sendHttpPost(payload, _config->getTargetHttpPost(),
               _config->getHeader1HttpPost(), _config->getHeader2HttpPost());
  return _lastSuccess;
}

bool BasePush::sendHttpPost2(String& payload) {
  sendHttpPost(payload, _config->getTargetHttpPost2(),
               _config->getHeader1HttpPost2(), _config->getHeader2HttpPost2());
  return _lastSuccess;
}

bool BasePush::sendHttpGet(String& payload) {
  sendHttpGet(payload, _config->getTargetHttpGet(),
              _config->getHeader1HttpGet(), _config->getHeader2HttpGet());
  return _lastSuccess;
}

bool BasePush::sendInfluxDb2(String& payload) {
  sendInfluxDb2(payload, _config->getTargetInfluxDB2(),
                _config->getOrgInfluxDB2(), _config->getBucketInfluxDB2(),
                _config->getTokenInfluxDB2());
  return _lastSuccess;
}

bool BasePush::sendMqtt(String& payload) {
  sendMqtt(payload, _config->getTargetMqtt(), _config->getPortMqtt(),
           _config->getUserMqtt(), _config->getPassMqtt());
  return _lastSuccess;
}

// EOF
