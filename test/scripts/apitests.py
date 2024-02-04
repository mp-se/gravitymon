import unittest, requests, json, time, urllib3

ver  = "1.4.0"

# wokwi
#host = "localhost:8180"
#id = "000110"

# Physical device
host = "192.168.1.160"
id = "e18798"
skipFactory = True # enable when coverate is collected
debugResult = False
headers = { "Authorization": "Bearer " + id, "Content-Type": "application/json"}

# pytest apitest.py
# python3 -m unittest -v apitests
# python3 -m unittest -v apitests.API.test_02_config_default
# python3 -m unittest -v apitests.API.test_bug_79

def call_api_post( path, json ):
    url = "http://" + host + path
    return requests.post( url, json=json, headers=headers)

def call_api_get( path ):
    url = "http://" + host + path
    return requests.get( url, headers=headers )

def do_factory_reset():
    try:
        r = call_api_get( "/api/factory?id=" + id)
    except urllib3.exceptions.ProtocolError:
        pass # Normal when device resets with async server
    except requests.exceptions.ConnectionError:
        pass # Normal when device resets with async server


class API(unittest.TestCase):

    def test_01_factory(self):
        return
        do_factory_reset() # Do this to ensure default settings for other tests
        time.sleep(4)

    def test_02_config_default(self):
        if not skipFactory: 
            do_factory_reset()
            time.sleep(4)
        r = call_api_get( "/api/config" )
        if debugResult: print(r.text)
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)

        # Assume most values are factory default
        self.assertNotEqual(j["mdns"], "empty")
        self.assertEqual(j["id"], "e18798")
        self.assertEqual(j["ota_url"], "")
        self.assertNotEqual(j["wifi_ssid"], "empty")
        self.assertEqual(j["wifi_ssid2"], "")
        self.assertEqual(j["ble_tilt_color"], "")
        self.assertEqual(j["ble_format"], 0)
        self.assertEqual(j["temp_format"], "C")
        self.assertEqual(j["token"], "")
        self.assertEqual(j["token2"], "")
        self.assertEqual(j["http_push"], "")
        self.assertEqual(j["http_push_h1"], "Content-Type: application/json")
        self.assertEqual(j["http_push_h2"], "")
        self.assertEqual(j["http_push2"], "")
        self.assertEqual(j["http_push2_h1"], "Content-Type: application/json")
        self.assertEqual(j["http_push2_h2"], "")
        self.assertEqual(j["http_push3"], "")
        self.assertEqual(j["influxdb2_push"], "")
        self.assertEqual(j["influxdb2_org"], "")
        self.assertEqual(j["influxdb2_bucket"], "")
        self.assertEqual(j["influxdb2_auth"], "")
        self.assertEqual(j["mqtt_push"], "")
        self.assertEqual(j["mqtt_port"], 1883)
        self.assertEqual(j["mqtt_user"], "")
        self.assertEqual(j["mqtt_pass"], "")
        self.assertNotEqual(j["sleep_interval"], 0)
        self.assertNotEqual(j["voltage_factor"], 0)
        self.assertNotEqual(j["voltage_config"], 0)
        self.assertEqual(j["gravity_formula"], "")
        self.assertEqual(j["gravity_format"], "G")
        self.assertEqual(j["temp_adjustment_value"], 0)
        self.assertEqual(j["gravity_temp_adjustment"], False)
        self.assertEqual(j["gyro_temp"], False)
        self.assertEqual(j["storage_sleep"], False)
        self.assertEqual(j["gyro_calibration_data"]["ax"], 0)
        self.assertEqual(j["gyro_calibration_data"]["ay"], 0)
        self.assertEqual(j["gyro_calibration_data"]["az"], 0)
        self.assertEqual(j["gyro_calibration_data"]["gx"], 0)
        self.assertEqual(j["gyro_calibration_data"]["gy"], 0)
        self.assertEqual(j["gyro_calibration_data"]["gz"], 0)
        self.assertEqual(j["gyro_read_count"], 50)
        self.assertEqual(j["gyro_moving_threashold"], 500)
        self.assertEqual(j["formula_max_deviation"], 3)
        self.assertEqual(j["wifi_portal_timeout"], 120)
        self.assertEqual(j["wifi_connect_timeout"], 20)
        self.assertEqual(j["formula_calibration_temp"], 20)
        self.assertEqual(j["http_int"], 0)
        self.assertEqual(j["http_int2"], 0)
        self.assertEqual(j["http_int3"], 0)
        self.assertEqual(j["influxdb2_int"], 0)
        self.assertEqual(j["mqtt_int"], 0)
        self.assertEqual(j["tempsensor_resolution"], 9)
        self.assertEqual(j["ignore_low_angles"], False)
        self.assertEqual(j["battery_saving"], True)
        self.assertEqual(len(j["formula_calculation_data"]), 10)
        self.assertEqual(j["formula_calculation_data"][0]["a"], 0)
        self.assertEqual(j["formula_calculation_data"][0]["g"], 1.0)

    def test_03_config_write_mdns(self):
        j = { "mdns": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["mdns"], "myvalue")

    def test_04_config_write_otaurl(self):
        j = { "ota_url": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["ota_url"], "myvalue")

    def test_05_config_write_blecolor(self):
        j = { "ble_tilt_color": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["ble_tilt_color"], "myvalue")

    def test_06_config_write_bleformat(self):
        j = { "ble_format": 1 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["ble_format"], 1)

    def test_07_config_write_tempformat(self):
        j = { "temp_format": "F" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["temp_format"], "F")

    def test_08_config_write_token(self):
        j = { "token": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["token"], "myvalue")

    def test_09_config_write_token2(self):
        j = { "token2": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["token2"], "myvalue")

    def test_10_config_write_httppush(self):
        j = { "http_push": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["http_push"], "myvalue")

    def test_11_config_write_httppushheader(self):
        j = { "http_push_h1": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["http_push_h1"], "myvalue")

    def test_12_config_write_httppushheader2(self):
        j = { "http_push_h2": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["http_push_h2"], "myvalue")

    def test_13_config_write_httppush2(self):
        j = { "http_push2": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["http_push2"], "myvalue")

    def test_14_config_write_httppush2header(self):
        j = { "http_push2_h1": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["http_push2_h1"], "myvalue")

    def test_15_config_write_httppush2header2(self):
        j = { "http_push2_h2": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["http_push2_h2"], "myvalue")

    def test_16_config_write_httppush3(self):
        j = { "http_push3": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["http_push3"], "myvalue")

    def test_17_config_write_influxdb2push(self):
        j = { "influxdb2_push": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["influxdb2_push"], "myvalue")

    def test_18_config_write_influxdb2org(self):
        j = { "influxdb2_org": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["influxdb2_org"], "myvalue")

    def test_19_config_write_influxdb2bucket(self):
        j = { "influxdb2_bucket": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["influxdb2_bucket"], "myvalue")

    def test_20_config_write_influxdb2auth(self):
        j = { "influxdb2_auth": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["influxdb2_auth"], "myvalue")

    def test_21_config_write_mqttpush(self):
        j = { "mqtt_push": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["mqtt_push"], "myvalue")

    def test_22_config_write_mqttport(self):
        j = { "mqtt_port": 1138 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["mqtt_port"], 1138)

    def test_23_config_write_mqttuser(self):
        j = { "mqtt_user": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["mqtt_user"], "myvalue")

    def test_24_config_write_mqttpass(self):
        j = { "mqtt_pass": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["mqtt_pass"], "myvalue")

    def test_25_config_write_sleepinterval(self):
        j = { "sleep_interval": 450 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["sleep_interval"], 450)

    def test_26_config_write_voltagefactor(self):
        j = { "voltage_factor": 1.99 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["voltage_factor"], 1.99)

    def test_27_config_write_voltageconfig(self):
        j = { "voltage_config": 5.12 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["voltage_config"], 5.12)

    def test_28_config_write_gravityformula(self):
        j = { "gravity_formula": "myvalue" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["gravity_formula"], "myvalue")

    def test_29_config_write_gravityformat(self):
        j = { "gravity_format": "P" }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["gravity_format"], "P")

    def test_30_config_write_tempadjustmentvalue(self):
        j = { "temp_adjustment_value": 1.12 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["temp_adjustment_value"], 1.12)

    def test_31_config_write_gravitytempadjustment(self):
        j = { "gravity_temp_adjustment": True }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["gravity_temp_adjustment"], True)

    def test_32_config_write_gyrotemp(self):
        j = { "gyro_temp": True }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["gyro_temp"], True)

    def test_33_config_write_storagesleep(self):
        j = { "storage_sleep": True }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["storage_sleep"], True)

    def test_34_config_write_gyrocalibrationdata(self):
        j = { "gyro_calibration_data": { "ax": 1, "ay": 1, "az": 1, "gx": 1, "gy": 1, "gz": 1 } }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["gyro_calibration_data"]["ax"], 1)
        self.assertEqual(j["gyro_calibration_data"]["ay"], 1)
        self.assertEqual(j["gyro_calibration_data"]["az"], 1)
        self.assertEqual(j["gyro_calibration_data"]["gx"], 1)
        self.assertEqual(j["gyro_calibration_data"]["gy"], 1)
        self.assertEqual(j["gyro_calibration_data"]["gz"], 1)

    def test_35_config_write_gyroreadcount(self):
        j = { "gyro_read_count": 51 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["gyro_read_count"], 51)

    def test_36_config_write_gyromovingthreashold(self):
        j = { "gyro_moving_threashold": 501 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["gyro_moving_threashold"], 501)

    def test_37_config_write_formulamaxdeviation(self):
        j = { "formula_max_deviation": 5 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["formula_max_deviation"], 5)

    def test_38_config_write_wifiportaltimeout(self):
        j = { "wifi_portal_timeout": 50 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["wifi_portal_timeout"], 50)

    def test_39_config_write_wificonnecttimeout(self):
        j = { "wifi_connect_timeout": 30 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["wifi_connect_timeout"], 30)

    def test_40_config_write_formulacalibrationtemp(self):
        j = { "formula_calibration_temp": 21.2 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(round(j["formula_calibration_temp"], 2), 21.2)

    def test_41_config_write_inthttp(self):
        j = { "http_int": 2 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["http_int"], 2)

    def test_42_config_write_inthttp2(self):
        j = { "http_int2": 3 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["http_int2"], 3)

    def test_43_config_write_inthttp3(self):
        j = { "http_int3": 4 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["http_int3"], 4)

    def test_44_config_write_intinflux(self):
        j = { "influxdb2_int": 5 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["influxdb2_int"], 5)

    def test_45_config_write_intmqtt(self):
        j = { "mqtt_int": 1 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["mqtt_int"], 1)

    def test_46_config_write_tempsensorresolution(self):
        j = { "tempsensor_resolution": 10 }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["tempsensor_resolution"], 10)

    def test_47_config_write_ignorelowangles(self):
        j = { "ignore_low_angles": True }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["ignore_low_angles"], True)

    def test_48_config_write_batterysaving(self):
        j = { "battery_saving": False }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["battery_saving"], False)

    def test_49_config_write_formulacalculationdata(self):
        j = { "formula_calculation_data": [ { "a": 1, "g": 1 }, { "a": 2, "g": 2 }, { "a": 3, "g": 3 }, { "a": 4, "g": 4 }, { "a": 5, "g": 5 }, { "a": 6, "g": 6 }, { "a": 7, "g": 7 }, { "a": 8, "g": 8 }, { "a": 9, "g": 9 }, { "a": 10, "g": 10 } ] }
        r = call_api_post( "/api/config", j )
        self.assertEqual(r.status_code, 200)
        r = call_api_get( "/api/config" )
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(len(j["formula_calculation_data"]), 10)
        self.assertEqual(j["formula_calculation_data"][0]["a"], 1)
        self.assertEqual(round(j["formula_calculation_data"][0]["g"]), 1)
        self.assertEqual(j["formula_calculation_data"][1]["a"], 2)
        self.assertEqual(round(j["formula_calculation_data"][1]["g"]), 2)
        self.assertEqual(j["formula_calculation_data"][2]["a"], 3)
        self.assertEqual(round(j["formula_calculation_data"][2]["g"]), 3)
        self.assertEqual(j["formula_calculation_data"][3]["a"], 4)
        self.assertEqual(round(j["formula_calculation_data"][3]["g"]), 4)
        self.assertEqual(j["formula_calculation_data"][4]["a"], 5)
        self.assertEqual(round(j["formula_calculation_data"][4]["g"]), 5)
        self.assertEqual(j["formula_calculation_data"][5]["a"], 6)
        self.assertEqual(round(j["formula_calculation_data"][5]["g"]), 6)
        self.assertEqual(j["formula_calculation_data"][6]["a"], 7)
        self.assertEqual(round(j["formula_calculation_data"][6]["g"]), 7)
        self.assertEqual(j["formula_calculation_data"][7]["a"], 8)
        self.assertEqual(round(j["formula_calculation_data"][7]["g"]), 8)
        self.assertEqual(j["formula_calculation_data"][8]["a"], 9)
        self.assertEqual(round(j["formula_calculation_data"][8]["g"]), 9)
        self.assertEqual(j["formula_calculation_data"][9]["a"], 10)
        self.assertEqual(round(j["formula_calculation_data"][9]["g"]), 10)

        """
        self.assertNotEqual(j["wifi_ssid"], "empty")
        self.assertEqual(j["wifi_ssid2"], "")
       
        """

    def test_50_status(self):
        #if not skipFactory: 
        #    do_factory_reset()
        #    time.sleep(4)
        r = call_api_get( "/api/status" )
        if debugResult: print(r.text)
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)

        # Vales can vary so we just check for the keys
        self.assertEqual(j["id"], id)
        self.assertNotEqual(j["temp_format"], "X")
        self.assertNotEqual(j["gravity_format"], "X")
        self.assertNotEqual(j["app_ver"], "0.0.0")
        self.assertNotEqual(j["app_build"], "build")
        self.assertNotEqual(j["mdns"], "empty")
        self.assertNotEqual(j["platform"], "empty")
        self.assertEqual(j["hardware"], "ispindel")
        self.assertEqual(j["sleep_mode"], False)
        self.assertNotEqual(j["angle"], 45.45)
        self.assertNotEqual(j["gravity"], 1.003)
        self.assertNotEqual(j["temp"], 29.9)
        self.assertNotEqual(j["battery"], 1.2)
        self.assertNotEqual(j["rssi"], -12)
        self.assertNotEqual(j["wifi_ssid"], 1.2)
        self.assertEqual(j["ispindel_config"], False)
        self.assertNotEqual(j["runtime_average"], 12.2)
        self.assertEqual(j["self_check"]["battery_level"], False)
        self.assertEqual(j["self_check"]["temp_connected"], False)
        self.assertEqual(j["self_check"]["gravity_formula"], False)
        self.assertEqual(j["self_check"]["gyro_calibration"], False)
        self.assertEqual(j["self_check"]["gyro_connected"], False)
        self.assertEqual(j["self_check"]["push_targets"], False)
 
    def test_51_format_read(self):
        r = call_api_get( "/api/config/format" )
        if debugResult: print(r.text)
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)

        self.assertNotEqual(j["http_format"], "empty")
        self.assertNotEqual(j["http_format2"], "empty")
        self.assertNotEqual(j["http_format3"], "empty")
        self.assertNotEqual(j["influxdb2_format"], "empty")
        self.assertNotEqual(j["mqtt_format"], "empty")

    def test_52_format_writehttp(self):
        j = { "http_format": "hello1"}
        r = call_api_post( "/api/config/format", j )
        if debugResult: print(r.text)
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config/format" )
        if debugResult: print(r.text)
        j = json.loads(r.text)
        self.assertEqual(j["http_format"], "hello1")

    def test_53_format_writehttp2(self):
        j = { "http_format2": "hello2"}
        r = call_api_post( "/api/config/format", j )
        if debugResult: print(r.text)
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config/format" )
        if debugResult: print(r.text)
        j = json.loads(r.text)
        self.assertEqual(j["http_format2"], "hello2")

    def test_54_format_writehttp3(self):
        j = { "http_format3": "hello3"}
        r = call_api_post( "/api/config/format", j )
        if debugResult: print(r.text)
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config/format" )
        if debugResult: print(r.text)
        j = json.loads(r.text)
        self.assertEqual(j["http_format3"], "hello3")

    def test_55_format_writeinfluxdb2(self):
        j = { "influxdb2_format": "hello4"}
        r = call_api_post( "/api/config/format", j )
        if debugResult: print(r.text)
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config/format" )
        if debugResult: print(r.text)
        j = json.loads(r.text)
        self.assertEqual(j["influxdb2_format"], "hello4")

    def test_56_format_writemqtt(self):
        j = { "mqtt_format": "hello5"}
        r = call_api_post( "/api/config/format", j )
        if debugResult: print(r.text)
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config/format" )
        if debugResult: print(r.text)
        j = json.loads(r.text)
        self.assertEqual(j["mqtt_format"], "hello5")

    def test_57_calibration(self):
        r = call_api_get( "/api/calibrate" )
        if debugResult: print(r.text)
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["success"], True)
        self.assertNotEqual(j["message"], "")
        time.sleep(4)
        r = call_api_get( "/api/calibrate/status" )
        if debugResult: print(r.text)
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["status"], False)
        self.assertEqual(j["success"], True)
        self.assertNotEqual(j["message"], "")

    def test_58(self):
        pass

    def test_59_config_sleepmode(self):
        j = { "sleep_mode": True }
        r = call_api_post( "/api/config/sleepmode", j )
        if debugResult: print(r.text)
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["sleep_mode"], True)

        j = { "sleep_mode": False }
        r = call_api_post( "/api/config/sleepmode", j )
        if debugResult: print(r.text)
        self.assertEqual(r.status_code, 200)
        j = json.loads(r.text)
        self.assertEqual(j["sleep_mode"], False)

    # TODO: Wifi Scan / WIfi Scan status
        
    # TODO: Test push / Test Push status
        
    # TODO: Filesystem
        
    # TODO: Authenticate
        
    # TODO: Create formula


if __name__ == '__main__':
    unittest.main()
