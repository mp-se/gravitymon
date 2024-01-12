![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.4.0-beta1/total) <br>

![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.3.0/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.3.0-beta3/total) <br> 
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.3.0-beta2/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.3.1-beta1/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.2.1/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.2.0/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.2.0-beta2/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.2.0-beta1/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.1.1/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.1.0/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.1.0-beta4/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.1.0-beta3/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.1.0-beta2/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.1.0-beta1/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.0.0/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.0.0-beta2/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v1.0.0-beta1/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v0.9.0/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v0.8.0/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v0.7.0/total) <br>
![download](https://img.shields.io/github/downloads/mp-se/gravitymon/v0.6.0/total) <br>


2.2.2-dev(38a443e)
Worker run!
mounting FS... mounted!
reading config file
parsed config:
{"Name":"","Token":"dRKrROCbxVIpvQxxIO5e","Sleep":30,"Server":"","API":16,"Port":80,"Channel":0,"URI":"","Username":"","Password":"","Job":"ispindel","Instance":"000","Hassio":false,"UseHTTPS":false,"Vfact":191.8000031,"TS":0,"OWpin":12,"POLY":"-0.00031*tilt^2+0.557*tilt-14.054","SSID":"@home","PSK":"wii42@steelKEYS","Offset":[0,0,0,0,0,0]}
offsets not available
Boot-Mode: Deep-Sleep Wake

woken from deepsleep, normal mode
Samples:42 min:89.60 max:89.73 time:757
x: -318 y: 16490 z: -106
Tilt: 89.65
Tacc: 23.68
Volt: 4.03
Temp: 22.38
Gravity: 33.39
IP: 192.168.1.116
adding BRICKS params
found token
adding payload...

calling BRICKS
sendBricks called

{"apikey":"dRKrROCbxVIpvQxxIO5e","type":"ispindel","brand":"wemos_d1_mini","version":"7.3.3","chipid":"eb2ce85cc3770513","s_number_wort_0":33.38998413,"s_number_temp_0":22.375,"s_number_voltage_0":4.030239582,"s_number_wifi_0":-66,"s_number_tilt_0":89.65087891}
{"apikey":"dRKrROCbxVIpvQxxIO5e","type":"ispindel","brand":"wemos_d1_mini","version":"1.4.0","chipid":"1c6b34"          ,"s_number_wort_0":25.36      ,"s_number_temp_0":23.00 ,"s_number_voltage_0":4.29       ,"s_number_wifi_0":-77,"s_number_tilt_0":89.262}

insecure
adding headers
serializing json
starting POST, payload: {"apikey":"dRKrROCbxVIpvQxxIO5e","type":"ispindel","brand":"wemos_d1_mini","version":"7.3.3","chipid":"eb2ce85cc3770513","s_number_wort_0":33.38998413,"s_number_temp_0":22.375,"s_number_voltage_0":4.030239582,"s_number_wifi_0":-66,"s_number_tilt_0":89.65087891}
code: 500

Final-sleep: 60s; RT: 2502


{"apikey":"${token}","type":"ispindel","brand":"wemos_d1_mini","version":"${app-ver}","chipid":"${id}","s_number_wort_0":${gravity-plato},"s_number_temp_0":${temp-c},"s_number_voltage_0":${battery},"s_number_wifi_0":${rssi},"s_number_tilt_0":${tilt}}

HTTP POST 1: "https://brewbricks.com/api/iot/v1"
TOKEN: "{your apikey}"
HEADER2: "User-Agent: iSpindel"