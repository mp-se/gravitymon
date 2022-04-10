.. _data-formats:

Data Formats
############

.. _data-formats-ispindle:

HTTP Post, iSpindle format 
==========================

This is the format used for standard http posts. 

* ``corr-gravity`` is an extended parameter containing a temperature corrected gravity reading.
* ``gravity-format`` is an extended parameter containing the gravity format (G or P).
* ``run-time`` is an extended parameter containing the number of seconds the execution took.

.. code-block:: json

   { 
      "name" : "gravmon",
      "ID": "2E6753",
      "token" : "gravmon",
      "interval": 900,
      "temperature": 20.5,
      "temp_units": "C",
      "gravity": 1.0050,
      "angle": 45.34,
      "battery": 3.67,
      "rssi": -12,

      "corr-gravity": 1.0050,
      "gravity-unit": "G",
      "run-time": 6
   }

This is the format template used to create the json above. 

.. code-block::

  {
   "name" : "${mdns}",
   "ID": "${id}",
   "token" : "gravmon",
   "interval": ${sleep-interval}, 
   "temperature": ${temp},
   "temp_units": "${temp-unit}",
   "gravity": ${gravity},
   "angle": ${angle},
   "battery": ${battery},
   "rssi": ${rssi},
   "corr-gravity": ${corr-gravity},
   "gravity-unit": "${gravity-unit}",
   "run-time": ${run-time}
  }


.. _data-formats-influxdb2:

HTTP Get
========

This is the format added to the URL when using HTTP get

.. code-block::
   
   ?name=<mdns>,id=<id>,token=<token>&interval=300&temperature=20.1&temp-units=<C|F>&
   gravity=$1.004&angle=45.5&battery=3.96&rssi=-18&corr-gravity=1.004&gravity-unit=<G|P>&run-time=2.1

This is the format template used to create the data above. 

.. code-block::

  ?name=${mdns}&id=${id}&token=${token2}&interval=${sleep-interval}&temperature=${temp}&
  temp-units=${temp-unit}&gravity=${gravity}&angle=${angle}&battery=${battery}&rssi=${rssi}&
  corr-gravity=${corr-gravity}&gravity-unit=${gravity-unit}&run-time=${run-time}


Influx DB v2
============

This is the format for InfluxDB v2

.. code-block::
   
   measurement,host=<mdns>,device=<id>,temp-format=<C|F>,gravity-format=SG,
   gravity=1.0004,corr-gravity=1.0004,angle=45.45,temp=20.1,battery=3.96,rssi=-18


This is the format template used to create the json above. 

.. code-block::

  measurement,host=${mdns},device=${id},temp-format=${temp-unit},
  gravity-format=${gravity-unit} gravity=${gravity},corr-gravity=${corr-gravity},
  angle=${angle},temp=${temp},battery=${battery},rssi=${rssi}


.. _data-formats-mqtt:

MQTT
====

This is the format used to send data to MQTT. Each of the lines are specific topics

.. code-block::
   
   ispindel/device_name/tilt 89.96796
   ispindel/device_name/temperature 21.375
   ispindel/device_name/temp_units C
   ispindel/device_name/battery 0.04171
   ispindel/device_name/gravity 33.54894
   ispindel/device_name/interval 1
   ispindel/device_name/RSSI -58


This is the format template used to create the json above. 

.. tip::

   Each line in the format is treated as one topic. The `|` is used as separator between lines and the first `:` is used as separator between topic and value. Each line is formatted as `<topic>:<value>`

.. code-block::

  ispindel/${mdns}/tilt:${angle}|
  ispindel/${mdns}/temperature:${temp}|
  ispindel/${mdns}/temp_units:${temp-unit}|
  ispindel/${mdns}/battery:${battery}|
  ispindel/${mdns}/gravity:${gravity}|
  ispindel/${mdns}/interval:${sleep-interval}|
  ispindel/${mdns}/RSSI:${rssi}|

This is a format template that is compatible with v0.6. Just replace the `topic` with the topic you want to post data to.

.. code-block::
   
   topic:{"name":"gravmon", "ID":"${id}", "token":"gravmon", "interval": ${sleep-interval},
   "temperature": ${temp}, "temp_units": "${temp-unit}", "gravity":${gravity},
   "angle": ${angle}, "battery":${battery}, "rssi": ${rssi}, "corr-gravity":${corr-gravity},
   "gravity-unit": "${gravity-unit}", "run-time": ${run-time}}|


version.json
============

Contents version.json. The version is used by the device to check if the this version is newer. The html files will also be downloaded if the are present on the server. This way it's easy to 
upgrade to a version that serve the html files from the file system. If they dont exist nothing will happen, the OTA flashing will still work. If the html files are missing from the file system 
they can be uploaded manually afterwards. 

.. code-block:: json

   { 
      "project":"gravmon", 
      "version":"0.7.0",  
      "html": [ 
         "index.min.htm", 
         "test.min.htm", 
         "config.min.htm", 
         "format.min.htm", 
         "calibration.min.htm", 
         "about.min.htm" 
      ] 
   }
