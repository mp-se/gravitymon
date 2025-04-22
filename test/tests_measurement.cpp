/*
MIT License

Copyright (c) 2025 Magnus

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
#include <AUnit.h>

#if defined(GATEWAY)

#include <measurement.hpp>

test(measure_list) {
  MeasurementList list;

  assertEqual(list.size(), 0);
  assertEqual(list.findMeasurementById("test"), -1);  

  std::unique_ptr<MeasurementBaseData> td;
  
  td.reset(new TiltData(MeasurementSource::NoSource, TiltColor::Red, 11.5, 2.2, 10, -10, false)); 
  list.updateData(td);
  // const TiltData* ptr = list.getMeasurementEntry(0)->getTiltData();

  assertEqual(list.findMeasurementById("Red"), 0);  
  assertEqual(list.size(), 1);  
  assertEqual(list.getMeasurementEntry(0)->getTiltData()->getRssi(), -10);
  assertEqual(list.getMeasurementType(0), MeasurementType::Tilt);  
  assertEqual(list.getMeasurementType(-1), MeasurementType::NoType);  
  assertEqual(list.getMeasurementType(1), MeasurementType::NoType);  
  td.reset(new TiltData(MeasurementSource::NoSource, TiltColor::Blue, 11.5, 2.2, 10, -12, false)); 

  list.updateData(td);
  assertEqual(list.findMeasurementById("Blue"), 1);  
  assertEqual(list.size(), 2);  
  assertEqual(list.getMeasurementType(1), MeasurementType::Tilt);  
  assertEqual(list.getMeasurementEntry(1)->getTiltData()->getRssi(), -12);

  td.reset(new TiltData(MeasurementSource::NoSource, TiltColor::Blue, 12.5, 3.2, 12, -18, false)); 
  list.updateData(td);
  assertEqual(list.findMeasurementById("Blue"), 1);  
  assertEqual(list.size(), 2);  
  Serial.printf("%d ", list.getMeasurementType(0));
  Serial.printf("%d ", list.getMeasurementType(1));
  assertEqual(list.getMeasurementType(1), MeasurementType::Tilt);  
  assertEqual(list.getMeasurementEntry(1)->getTiltData()->getRssi(), -18);
}

test(measure_container) {
  MeasurementEntry d("test");

  assertEqual(d.getId(), "test");
  assertEqual(d.isUpdated(), false);
  assertEqual(d.getType(), MeasurementType::NoType);
  assertEqual(d.getData(), nullptr);

  d.setUpdated();
  assertEqual(d.isUpdated(), true);

  d.setPushed();
  assertEqual(d.isUpdated(), false);
}

test(measure_base) {
  MeasurementBaseData d("test", MeasurementType::NoType, MeasurementSource::NoSource);

  assertEqual(d.getId(), "test");
  assertEqual(d.getType(), MeasurementType::NoType);
  assertEqual(d.getSource(), MeasurementSource::NoSource);
}

test(measure_tilt) {
  TiltData d(MeasurementSource::NoSource, TiltColor::Red, 11.5, 2.2, 10, -10, false);

  assertEqual(d.getType(), MeasurementType::Tilt);
  assertEqual(d.getId(), "Red");
  float t = 11.5;
  assertEqual(d.getTempF(), t);
  t = 2.2;
  assertEqual(d.getGravity(), t);
  assertEqual(d.getTxPower(), 10);
  assertEqual(d.getRssi(), -10);
  assertEqual(d.getTiltColor(), TiltColor::Red);
}

test(measure_gravity) {
  GravityData d(MeasurementSource::NoSource, "id", "name", "token", 11.5, 2.2, 3.3, 4.4, 10, -10, 5);

  assertEqual(d.getType(), MeasurementType::Gravitymon);
  assertEqual(d.getId(), "id");
  assertEqual(d.getName(), "name");
  assertEqual(d.getToken(), "token");
  float t = 11.5;
  assertEqual(d.getTempC(), t);
  t = 2.2;
  assertEqual(d.getGravity(), t);
  t = 3.3;
  assertEqual(d.getAngle(), t);
  t = 4.4;
  assertEqual(d.getBattery(), t);
  assertEqual(d.getTxPower(), 10);
  assertEqual(d.getRssi(), -10);
  assertEqual(d.getInterval(), 5);
}

test(measure_pressure) {
  PressureData d(MeasurementSource::NoSource, "id", "name", "token", 11.5, 2.2, 3.3, 4.4, 10, -10, 5);

  assertEqual(d.getType(), MeasurementType::Pressuremon);
  assertEqual(d.getId(), "id");
  assertEqual(d.getName(), "name");
  assertEqual(d.getToken(), "token");
  float t = 11.5;
  assertEqual(d.getTempC(), t);
  t = 2.2;
  assertEqual(d.getPressure(), t);
  t = 3.3;
  assertEqual(d.getPressure1(), t);
  t = 4.4;
  assertEqual(d.getBattery(), t);
  assertEqual(d.getTxPower(), 10);
  assertEqual(d.getRssi(), -10);
  assertEqual(d.getInterval(), 5);
}

test(measure_chamber) {
  ChamberData d(MeasurementSource::NoSource, "id", 11.5, 2.2, -10);

  assertEqual(d.getType(), MeasurementType::Chamber);
  assertEqual(d.getId(), "id");
  float t = 11.5;
  assertEqual(d.getChamberTempC(), t);
  t = 2.2;
  assertEqual(d.getBeerTempC(), t);
  t = 3.3;
  assertEqual(d.getRssi(), -10);
}

#endif // GATEWAY

// EOF
