/*
MIT License

Copyright (c) 2021-2026 Magnus

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
#ifndef ESPFWK_DISABLE_WIFI

#ifdef ESP8266
#include <ESP8266mDNS.h>
#endif

#include <MQTT.h>

#include <battery.hpp>
#include <config_brewing.hpp>
#include <cstdio>
#include <helper.hpp>
#include <main.hpp>
#include <perf.hpp>
#include <pushtarget.hpp>
#include <templating.hpp>

constexpr auto PUSHINT_FILENAME = "/push.dat";

void PushIntervalTracker::update(const int index, const int defaultValue) {
  if (_counters[index] <= 0)
    _counters[index] = defaultValue;
  else
    _counters[index]--;
}

void PushIntervalTracker::load() {
  File intFile = LittleFS.open(PUSHINT_FILENAME, "r");

  if (intFile) {
    String line = intFile.readStringUntil('\n');
    Log.notice(F("PUSH: Read interval tracker %s." CR), line.c_str());

    char temp[80];
    char *s, *p = &temp[0];
    int i = 0;

    snprintf(&temp[0], sizeof(temp), "%s", line.c_str());
    while ((s = strtok_r(p, ":", &p)) != NULL) {
      _counters[i++] = atoi(s);
    }

    intFile.close();
  }

#if LOG_LEVEL == 6
  Log.verbose(F("PUSH: Parsed trackers: %d:%d:%d:%d:%d." CR), _counters[0],
              _counters[1], _counters[2], _counters[3], _counters[4]);
#endif
}

void PushIntervalTracker::save() {
  update(0, _brewingConfig->getPushIntervalPost());
  update(1, _brewingConfig->getPushIntervalPost2());
  update(2, _brewingConfig->getPushIntervalGet());
  update(3, _brewingConfig->getPushIntervalInflux());
  update(4, _brewingConfig->getPushIntervalMqtt());

  // If this feature is disabled we skip saving the file
  if (!_brewingConfig->isPushIntervalActive()) {
#if LOG_LEVEL == 6
    Log.notice(F("PUSH: Variabled push interval disabled." CR));
#endif
    LittleFS.remove(PUSHINT_FILENAME);
  } else {
    Log.notice(
        F("PUSH: Variabled push interval enabled, updating counters." CR));
    File intFile = LittleFS.open(PUSHINT_FILENAME, "w");

    if (intFile) {
      // Format=http1:http2:http3:influx:mqtt
      intFile.printf("%d:%d:%d:%d:%d\n", _counters[0], _counters[1],
                     _counters[2], _counters[3], _counters[4]);
      intFile.close();
    }
  }
}

BrewingPush::BrewingPush(BrewingConfig* brewingConfig)
    : BasePush(brewingConfig) {
  _brewingConfig = brewingConfig;
}

void BrewingPush::sendAll(TemplatingEngine& engine, MeasurementType type,
                          bool enableHttpPost, bool enableHttpPost2,
                          bool enableHttpGet, bool enableInfluxdb2,
                          bool enableMqtt) {
  printHeap("PUSH");
  _http->setReuse(true);

  PushIntervalTracker intDelay(_brewingConfig);
  intDelay.load();

  if (_brewingConfig->hasTargetHttpPost() && intDelay.useHttp1() &&
      enableHttpPost) {
    PERF_BEGIN("push-http");
    String tpl;

    if (type == MeasurementType::GRAVITY)
      tpl = getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP1);
    else
      tpl = getTemplate(BrewingPush::PRESSURE_TEMPLATE_HTTP1);

    String doc = engine.create(tpl.c_str());

    if (_brewingConfig->isHttpPostSSL() && _brewingConfig->isSkipSslOnTest() &&
        runMode != RunMode::measurementMode)
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      sendHttpPost(doc);
    PERF_END("push-http");
    yield();
  }

  if (_brewingConfig->hasTargetHttpPost2() && intDelay.useHttp2() &&
      enableHttpPost2) {
    PERF_BEGIN("push-http2");
    String tpl;

    if (type == MeasurementType::GRAVITY)
      tpl = getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP2);
    else
      tpl = getTemplate(BrewingPush::PRESSURE_TEMPLATE_HTTP2);

    String doc = engine.create(tpl.c_str());

    if (_brewingConfig->isHttpPost2SSL() && _brewingConfig->isSkipSslOnTest() &&
        runMode != RunMode::measurementMode)
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      sendHttpPost2(doc);
    PERF_END("push-http2");
    yield();
  }

  if (_brewingConfig->hasTargetHttpGet() && intDelay.useHttp3() &&
      enableHttpGet) {
    PERF_BEGIN("push-http3");
    String tpl;

    if (type == MeasurementType::GRAVITY)
      tpl = getTemplate(BrewingPush::GRAVITY_TEMPLATE_HTTP3);
    else
      tpl = getTemplate(BrewingPush::PRESSURE_TEMPLATE_HTTP3);

    String doc = engine.create(tpl.c_str());

    if (_brewingConfig->isHttpGetSSL() && _brewingConfig->isSkipSslOnTest() &&
        runMode != RunMode::measurementMode)
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      sendHttpGet(doc);
    PERF_END("push-http3");
    yield();
  }

  if (_brewingConfig->hasTargetInfluxDb2() && intDelay.useInflux() &&
      enableInfluxdb2) {
    PERF_BEGIN("push-influxdb2");
    String tpl;

    if (type == MeasurementType::GRAVITY)
      tpl = getTemplate(BrewingPush::GRAVITY_TEMPLATE_INFLUX);
    else
      tpl = getTemplate(BrewingPush::PRESSURE_TEMPLATE_INFLUX);

    String doc = engine.create(tpl.c_str());

    if (_brewingConfig->isHttpInfluxDb2SSL() &&
        _brewingConfig->isSkipSslOnTest() &&
        runMode != RunMode::measurementMode)
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      sendInfluxDb2(doc);
    PERF_END("push-influxdb2");
    yield();
  }

  if (_brewingConfig->hasTargetMqtt() && intDelay.useMqtt() && enableMqtt) {
    PERF_BEGIN("push-mqtt");
    String tpl;

    if (type == MeasurementType::GRAVITY)
      tpl = getTemplate(BrewingPush::GRAVITY_TEMPLATE_MQTT);
    else
      tpl = getTemplate(BrewingPush::PRESSURE_TEMPLATE_MQTT);

    String doc = engine.create(tpl.c_str());

    if (_brewingConfig->isMqttSSL() && _brewingConfig->isSkipSslOnTest() &&
        runMode != RunMode::measurementMode)
      Log.notice(F("PUSH: SSL enabled, skip run when not in gravity mode." CR));
    else
      sendMqtt(doc);
    PERF_END("push-mqtt");
    yield();
  }

  intDelay.save();
}

const char* BrewingPush::getTemplate(Templates t, bool useDefaultTemplate) {
  String fname;
  _baseTemplate.reserve(600);

  // Load templates from memory
  switch (t) {
    case GRAVITY_TEMPLATE_HTTP1:
      _baseTemplate = String(iGravityHttpPostFormat);
      fname = TPL_GRAVITY_FNAME_POST;
      break;
    case GRAVITY_TEMPLATE_HTTP2:
      _baseTemplate = String(iGravityHttpPostFormat);
      fname = TPL_GRAVITY_FNAME_POST2;
      break;
    case GRAVITY_TEMPLATE_HTTP3:
      _baseTemplate = String(iGravityHttpGetFormat);
      fname = TPL_GRAVITY_FNAME_GET;
      break;
    case GRAVITY_TEMPLATE_INFLUX:
      _baseTemplate = String(iGravityInfluxDbFormat);
      fname = TPL_GRAVITY_FNAME_INFLUXDB;
      break;
    case GRAVITY_TEMPLATE_MQTT:
      _baseTemplate = String(iGravityMqttFormat);
      fname = TPL_GRAVITY_FNAME_MQTT;
      break;

    case PRESSURE_TEMPLATE_HTTP1:
      _baseTemplate = String(iPressureHttpPostFormat);
      fname = TPL_PRESSURE_FNAME_POST;
      break;
    case PRESSURE_TEMPLATE_HTTP2:
      _baseTemplate = String(iPressureHttpPostFormat);
      fname = TPL_PRESSURE_FNAME_POST2;
      break;
    case PRESSURE_TEMPLATE_HTTP3:
      _baseTemplate = String(iPressureHttpGetFormat);
      fname = TPL_PRESSURE_FNAME_GET;
      break;
    case PRESSURE_TEMPLATE_INFLUX:
      _baseTemplate = String(iPressureInfluxDbFormat);
      fname = TPL_PRESSURE_FNAME_INFLUXDB;
      break;
    case PRESSURE_TEMPLATE_MQTT:
      _baseTemplate = String(iPressureMqttFormat);
      fname = TPL_PRESSURE_FNAME_MQTT;
      break;
  }

  if (!useDefaultTemplate) {
    File file = LittleFS.open(fname, "r");
    if (file) {
      size_t fileSize = file.size();
      char* buf = reinterpret_cast<char*>(malloc(fileSize + 1));
      if (buf == nullptr) {
        Log.error(F("PUSH: Failed to allocate %d bytes for template %s." CR),
                  fileSize, fname.c_str());
        file.close();
        return _baseTemplate.c_str();
      }

      memset(buf, 0, fileSize + 1);
      size_t bytesRead = file.readBytes(buf, fileSize);
      file.close();

      if (bytesRead == fileSize) {
        _baseTemplate = String(buf);
        Log.notice(F("PUSH: Template loaded from disk %s (%d bytes)." CR),
                   fname.c_str(), bytesRead);
      } else {
        Log.warning(F("PUSH: Only read %d of %d bytes from %s." CR), bytesRead,
                    fileSize, fname.c_str());
      }

      free(buf);
    }
  }

#if LOG_LEVEL == 6
  Log.verbose(F("TPL : Base '%s'." CR), _baseTemplate.c_str());
#endif

  return _baseTemplate.c_str();
}

#endif  // ESPFWK_DISABLE_WIFI

// EOF
