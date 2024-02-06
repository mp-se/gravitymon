# Code coverage

This directory will contain the code coverage reports from testing.

## The current status of the unit tests for gravitymon is as follows

| File | Coverage | Comment |
| ----------- | ----------- | ----------- |
| calc.cpp | 95 % | Good |
| gyro.cpp | 20 % | To be evaluated again |
| helper.cpp | 44 % | Lot of code that is board specific so hard to get better coverage |
| ble.cpp | - | Manual tests |
| main.cpp | - | Manual tests + Pytests  |
| pushtarget.cpp | - | Covered by pytests |
| resources.cpp | - | No tests needed, only data |
| templating.cpp | 80 % | Good |
| tempsensor.cpp | 80 % | Good |
| webserver.cpp | 76 % | Covered by api tests (apitests.py) |
| wifi.cpp | 33 % | Covered by api tests (apitests.py) |
| wifimanager.cpp | - | Manual tests needed |
