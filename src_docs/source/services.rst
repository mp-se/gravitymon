.. _services:

Service Integration
###################

This chapter contains a list of targets and what configuration is needed to interact with them.

Brewfather
++++++++++

Just enter the http adress found on brewfather, not other settings are needed. The endpoint has the following format:

.. code-block::

   http://log.brewfather.net/http://log.brewfather.net/stream?id=<yourid>


The URL is found under settings.

Fermentrack
+++++++++++

`Fermentrack <https://www.fermentrack.com>`_ is a open source brewing software to monitor and control fermentation.

GravityMon can be installed and used as an iSpindle. Just register the device as an iSpindle and use the defined endpoint which normally is:

.. code-block::

   http://myservername/ispindel

UBIdots
+++++++

`UBIdots <https://www.ubidots.com>`_ is a IoT service that display data collected various sources. 

For this service there are two options to configure the integration. First you will need your default token which is found under `API Credentials` (<api-tokem> in the example below).
Swap the text <devicename> with the name you want to show in ubidots. 

**Option 1** - token as an URL parameter

Enter the following as URL:

.. code-block::

   http://industrial.api.ubidots.com/api/v1.6/devices/<devicename>/?token=<api-token>


This is the less secure option. 

**Option 2** - token as the http header

Enter the following as URL, use either standard or ssl.
   
.. code-block::

   http://industrial.api.ubidots.com/api/v1.6/devices/<devicename>
   https://industrial.api.ubidots.com/api/v1.6/devices/<devicename>


Under `Headers` (button after the http url in the UI) enter the following string: 
   
.. code-block::

   X-Auth-Token: <api-token>


This is the more secure option.

Even though ubidots can handle the default ispindle format it probably better to just post the data you want. This is an example of a 
format template that can be used. For information on customizing the format see :ref:`format-editor`.

.. code-block:: 

   { 
      "temperature": ${temp}, 
      "gravity": ${gravity}, 
      "angle": ${angle}, 
      "battery": ${battery}, 
      "rssi": ${rssi} 
   }

