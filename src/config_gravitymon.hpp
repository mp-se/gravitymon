/*
MIT License

Copyright (c) 2021-2025 Magnus

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
#ifndef SRC_CONFIG_GRAVITYMON_HPP_
#define SRC_CONFIG_GRAVITYMON_HPP_

#if defined(GRAVITYMON)

#include <config_brewing.hpp>
#include <gyro.hpp>
#include <main_gravitymon.hpp>

constexpr auto CONFIG_GRAVITY_FORMULA = "gravity_formula";
constexpr auto CONFIG_GRAVITY_UNIT = "gravity_unit";
constexpr auto CONFIG_GRAVITY_TEMP_ADJ = "gravity_temp_adjustment";
constexpr auto CONFIG_GYRO_CALIBRATION = "gyro_calibration_data";
constexpr auto CONFIG_GYRO_TEMP = "gyro_temp";
constexpr auto CONFIG_GYRO_DISABLED = "gyro_disabled";
constexpr auto CONFIG_GYRO_FILTER = "gyro_filter";
constexpr auto CONFIG_GYRO_SWAP_XY = "gyro_swap_xy";
constexpr auto CONFIG_STORAGE_SLEEP = "storage_sleep";
constexpr auto CONFIG_FORMULA_DATA = "formula_calculation_data";
constexpr auto CONFIG_GRAVITY = "gravity";
constexpr auto CONFIG_BLE_TILT_COLOR = "ble_tilt_color";
constexpr auto CONFIG_GYRO_READ_COUNT = "gyro_read_count";
constexpr auto CONFIG_GYRO_MOVING_THREASHOLD = "gyro_moving_threashold";
constexpr auto CONFIG_FORMULA_DEVIATION = "formula_max_deviation";
constexpr auto CONFIG_FORMULA_CALIBRATION_TEMP = "formula_calibration_temp";

enum GravitymonBleFormat {
  BLE_DISABLED = 0,
  BLE_TILT = 1,
  BLE_TILT_PRO = 2,
  BLE_GRAVITYMON_EDDYSTONE = 4,
  BLE_GRAVITYMON_IBEACON = 5
};

// Used for holding formulaData (used for calculating formula on device)
#define FORMULA_DATA_SIZE 20

struct RawFormulaData {
  double a[FORMULA_DATA_SIZE];
  double g[FORMULA_DATA_SIZE];
};

class GravitymonConfig : public BrewingConfig, public GyroConfigInterface {
 private:
  String _gravityFormula = "";
  String _bleTiltColor;

  RawGyroData _gyroCalibration = {0, 0, 0, 0, 0, 0};
  RawFormulaData _formulaData = {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
  GravitymonBleFormat _gravitymonBleFormat = GravitymonBleFormat::BLE_DISABLED;

  bool _gravityTempAdj = false;
  bool _ignoreLowAngles = false;
  bool _storageSleep = false;
  bool _gyroDisabled = false;
  bool _gyroSwapXY = false;
  bool _gyroFilter = false;
#if defined(FLOATY)
  bool _gyroTemp = true;
  bool _batterySaving = false;
#else
  bool _gyroTemp = false;
  bool _batterySaving = true;
#endif

  int _voltagePin = PIN_VOLT;

  char _gravityUnit = 'G';

  float _maxFormulaCreationDeviation = 0.01;  // SG
  float _defaultCalibrationTemp = 20.0;       // C

  int _gyroSensorMovingThreashold = 500;
  int _gyroReadCount = 50;
  int _gyroReadDelay = 3150;  // us, empirical, to hold sampling to 200 Hz

 public:
  GravitymonConfig(String baseMDNS, String fileName);

  bool isGyroTemp() const { return _gyroTemp; }
  void setGyroTemp(bool b) {
#if defined(FLOATY)
    // Floaty hardware dont have a temp sensor, uses gyro temperature
#else
    _gyroTemp = b;
    _saveNeeded = true;
#endif
  }

  int getVoltagePin() const { return _voltagePin; }
  void setVoltagePin(int v) {
    _voltagePin = v;
    _saveNeeded = true;
  }

  bool isStorageSleep() const { return _storageSleep; }
  void setStorageSleep(bool b) {
    _storageSleep = b;
    _saveNeeded = true;
  }

  bool isGyroDisabled() const { return _gyroDisabled; }
  void setGyroDisabled(bool b) {
    _gyroDisabled = b;
    _saveNeeded = true;
  }

  bool isGyroFilter() const { return _gyroFilter; }
  void setGyroFilter(bool b) {
    _gyroFilter = b;
    _saveNeeded = true;
  }

  bool isGyroSwapXY() const { return _gyroSwapXY; }
  void setGyroSwapXY(bool b) {
    _gyroSwapXY = b;
    _saveNeeded = true;
  }

  const char* getGravityFormula() const { return _gravityFormula.c_str(); }
  void setGravityFormula(String s) {
    _gravityFormula = s;
    _saveNeeded = true;
  }

  bool isGravityTempAdj() const { return _gravityTempAdj; }
  void setGravityTempAdj(bool b) {
    _gravityTempAdj = b;
    _saveNeeded = true;
  }

  char getGravityUnit() const { return _gravityUnit; }
  void setGravityUnit(char c) {
    if (c == 'G' || c == 'P') {
      _gravityUnit = c;
      _saveNeeded = true;
    }
  }
  bool isGravitySG() const { return _gravityUnit == 'G'; }
  bool isGravityPlato() const { return _gravityUnit == 'P'; }

  const char* getBleTiltColor() const { return _bleTiltColor.c_str(); }
  void setBleTiltColor(String c) {
    _bleTiltColor = c;
    _saveNeeded = true;
  }

  const RawGyroData& getGyroCalibration() const { return _gyroCalibration; }
  void setGyroCalibration(const RawGyroData& r) {
    _gyroCalibration = r;
    _saveNeeded = true;
  }

  bool hasGyroCalibration() const {
    return (!_gyroCalibration.ax && !_gyroCalibration.ay &&
            !_gyroCalibration.az && !_gyroCalibration.gx &&
            !_gyroCalibration.gy && !_gyroCalibration.gz)
               ? false
               : true;
  }

  const RawFormulaData& getFormulaData() const { return _formulaData; }
  void setFormulaData(const RawFormulaData& r) {
    _formulaData = r;
    _saveNeeded = true;
  }

  float getMaxFormulaCreationDeviation() const {
    return _maxFormulaCreationDeviation;
  }
  void setMaxFormulaCreationDeviation(float f) {
    _maxFormulaCreationDeviation = f;
    _saveNeeded = true;
  }

  float getDefaultCalibrationTemp() const { return _defaultCalibrationTemp; }
  void SetDefaultCalibrationTemp(float t) {
    _defaultCalibrationTemp = t;
    _saveNeeded = true;
  }

  int getGyroSensorMovingThreashold() const {
    return _gyroSensorMovingThreashold;
  }
  void setGyroSensorMovingThreashold(int t) {
    _gyroSensorMovingThreashold = t;
    _saveNeeded = true;
  }

  int getGyroReadCount() const { return _gyroReadCount; }
  void setGyroReadCount(int c) {
    _gyroReadCount = c;
    _saveNeeded = true;
  }

  int getGyroReadDelay() const { return _gyroReadDelay; }
  void setGyroReadDelay(int d) {
    _gyroReadDelay = d;
    _saveNeeded = true;
  }

  bool isBleActive() const {
    return (_gravitymonBleFormat != GravitymonBleFormat::BLE_DISABLED);
  }

  const GravitymonBleFormat getGravitymonBleFormat() const {
    return _gravitymonBleFormat;
  }
  void setGravitymonBleFormat(int b) {
    _gravitymonBleFormat = (GravitymonBleFormat)b;
    _saveNeeded = true;
  }
  void setGravitymonBleFormat(GravitymonBleFormat b) {
    _gravitymonBleFormat = b;
    _saveNeeded = true;
  }

  bool isIgnoreLowAngles() const { return _ignoreLowAngles; }
  void setIgnoreLowAngles(bool b) {
    _ignoreLowAngles = b;
    _saveNeeded = true;
  }

  bool isBatterySaving() const { return _batterySaving; }
  void setBatterySaving(bool b) {
    _batterySaving = b;
    _saveNeeded = true;
  }

  void createJson(JsonObject& doc) const;
  void parseJson(JsonObject& doc);
  void migrateSettings();
  void migrateHwSettings();

  // Wrappers for GyroConfig
  int getSleepInterval() const { return BrewingConfig::getSleepInterval(); }
  bool saveFile() { return BaseConfig::saveFile(); }
};

#endif  // GRAVITYMON

#endif  // SRC_CONFIG_GRAVITYMON_HPP_

// EOF
