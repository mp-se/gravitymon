.. _custom:

Custom boards
#############

Its possible to add custom boards to the build process. These are the current ones supported and who has contributed:

.. list-table:: Custom boards
   :widths: 20 60 20
   :header-rows: 1

   * - boardid
     - description
     - contributor
   * - id
     - This is an example id
     - contributor tag or name




Olimex ESP32-C3-DevKit-Lipo
+++++++++++++++++++++++++++

This Model is supported by gravitymon. It is tested with the GY-521 only! So the temperature also gets measured by the gyro.
To have battery voltage monitoring, solder the bridges 'PAT_PWR_E1' and 'BAT_SENS_E1' at the bottom side together, to measure the voltage with pin 3.
The LiPo/LiIon-battery gets connected to the 2.00 mm pitch connector ontop of the board. 

Gyro gets connected to 3,3V, GND and SDA to PIN 1 and SCL to PIN 0 of the olimex board.

     