import requests
import json

host = "192.168.1.1"           # IP adress (or name) of the device to send these settings to
id = "ee1bfc"                  # Device ID (shown in serial console during startup or in UI)

def set_config( url, json ):
    headers = { "ContentType": "application/json" }
    print( url )
    #print( json )
    resp = requests.post( url, headers=headers, data=json )
    if resp.status_code != 200 :
        print ( "Failed "  )
    else :
        print ( "Success "  )

#
# Configure the device settings.
#
url = "http://" + host + "/api/config/device"
json = { "id": id, 
         "mdns": "gravmon",             # Name of the device
         "temp-format": "C",            # Temperature format C or F
         "sleep-interval": 30           # Sleep interval in seconds
        }
set_config( url, json )

#
# Configure the push settings. Blank means that its no used.
#
url = "http://" + host + "/api/config/push"
json = { "id": id, 
         "token": "",
         "http-push": "http://192.168.1.1/ispindel",    # HTTP endpoint 
         "http-push2": "",                              # HTTP endpoint2
         "http-push-h1": "Content-Type: application/json",
         "http-push-h2": "",
         "http-push2-h1": "Content-Type: application/json",
         "http-push2-h2": "",
         "influxdb2-push": "",                          # InfluxDB2 settings
         "influxdb2-org": "",
         "influxdb2-bucket": "",
         "influxdb2-auth": "" ,
         "mqtt-push": "",
         "mqtt-port": 1883,
         "mqtt-user": "",
         "mqtt-pass": ""
         }  
set_config( url, json )


#
# Configure the gravity settings.
#
url = "http://" + host + "/api/config/gravity"
json = { "id": id, 
         "gravity-formula": "",                         # If you want to set the gravity formula
         "gravity-format": "G",
         "gravity-temp-adjustment": "off"              # on or off
         }
set_config( url, json )

#
# Configure the hardware settings.
#
url = "http://" + host + "/api/config/hardware"
json = { "id": id, 
         "voltage-factor": 1.59,                        # Default value for voltage calculation
         "temp-adjustment": 0,                          # If temp sensor needs to be corrected
         "gyro-temp": "off",                            # Use the temp sensor in the gyro
         "ota-url": ""                                  # if the device should seach for a new update when active
        }
set_config( url, json )

#
# Configure the gravity formula settings. If this is called the formula will be updated based on these measurements (zero angle values will be ignored)
#
url = "http://" + host + "/api/formula"
json = { "id": id, 
         "a1": 22.4, 
         "a2": 54.4, 
         "a3": 58, 
         "a4": 0, 
         "a5": 0, 
         "g1": 1.000, 
         "g2": 1.053, 
         "g3": 1.062, 
         "g4": 1, 
         "g5": 1 
         }
set_config( url, json )
