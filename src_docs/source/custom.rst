.. _custom-boards:

Custom boards
#############

Its possible to add custom boards to the build process. These can be built from the source code, I dont take any 
responsibiity for testing and ensuring those work as intended only that they will build without errors. I dont 
have the boards to do testing. You are Welcome to submit issues for these targets and I will assign those to the 
contributor. 

.. list-table:: Custom boards
   :widths: 40 30 30
   :header-rows: 1

   * - board-id
     - added in
     - contributor
   * - Olimex ESP32-C3-DevKit-Lipo
     - 2.1.0
     - simonidelh


Olimex ESP32-C3-DevKit-Lipo
+++++++++++++++++++++++++++

This Model is supported by gravitymon. It is tested with the GY-521 only! So the temperature also gets measured by the gyro.
To have battery voltage monitoring, solder the bridges 'PAT_PWR_E1' and 'BAT_SENS_E1' at the bottom side together, to measure the voltage with pin 3.
The LiPo/LiIon-battery gets connected to the 2.00 mm pitch connector ontop of the board. 

Gyro gets connected to 3,3V, GND and SDA to PIN 1 and SCL to PIN 0 of the olimex board.

     