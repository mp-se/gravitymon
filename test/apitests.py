import unittest, requests, json, time

ver  = "1.4.0"

# wokwi
#host = "localhost:8180"
#id = "000110"

# Physical device
host = "192.168.1.212"
id = "66a1b0"

# python3 -m unittest -v apitests
# python3 -m unittest -v apitests.API.test_config_1
# python3 -m unittest -v apitests.API.test_bug_79

def call_api_post( path, json ):
    url = "http://" + host + path
    return requests.post( url, data=json )

def call_api_get( path ):
    url = "http://" + host + path
    return requests.get( url )

class API(unittest.TestCase):

    # Do factory reset for testing
    def test_factory(self):
        r = call_api_get( "/api/status" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["id"], id)
        r = call_api_get( "/api/factory?id=" + j["id"])
        time.sleep(4)

    # Check that all parameters exist
    def test_status(self):
        call_api_get( "/api/factory?id=" + id)
        time.sleep(4)
        r = call_api_get( "/api/status" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["id"], id)
        self.assertEqual(j["wifi-ssid"], "@home")
        self.assertNotEqual(j["angle"], 0)
        self.assertNotEqual(j["gravity"], 2)
        self.assertNotEqual(j["temp-c"], 0)
        self.assertNotEqual(j["temp-f"], 0)
        self.assertNotEqual(j["battery"], 0)
        self.assertNotEqual(j["temp-format"], "")
        self.assertNotEqual(j["gravity-format"], "")
        self.assertNotEqual(j["sleep-mode"], True)
        self.assertNotEqual(j["rssi"], 0)
        self.assertNotEqual(j["app-ver"], "0.0.0")
        self.assertNotEqual(j["app-build"], "test")
        self.assertNotEqual(j["mdns"], "")
        self.assertNotEqual(j["platform"], "")
        self.assertNotEqual(j["runtime-average"], -1)
        self.assertEqual(j["self-check"]["gyro-connected"], True)
        self.assertEqual(j["self-check"]["gyro-calibration"], False)
        self.assertEqual(j["self-check"]["temp-connected"], True)
        self.assertEqual(j["self-check"]["gravity-formula"], False)
        self.assertEqual(j["self-check"]["battery-level"], True)
        self.assertEqual(j["self-check"]["push-targets"], False)

    # Check that all parameters exist
    def test_config_1(self):
        j = { "id": id, "http-push": "https://push.me", "token": "mytoken", "token2": "mytoken2", "http-push2": "http://push.me", "http-push3": "http://push.me", "influxdb2-push": "http://influx.db", "influxdb2-org": "my-org", 
              "influxdb2-bucket": "my-bucket", "influxdb2-auth": "my-secret", "mqtt-push": "mqtt.com", "mqtt-port": 1883, "mqtt-user": "my-user", 
              "mqtt-pass": "my-pass", "http-push-h1": "header1", "http-push-h2": "header2", "http-push2-h1": "header1(2)", "http-push2-h2": "header2(2)" }
        r = call_api_post( "/api/config/push", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["http-push-h1"], "header1")
        self.assertEqual(j["http-push-h2"], "header2")
        self.assertEqual(j["http-push2-h1"], "header1(2)")
        self.assertEqual(j["http-push2-h2"], "header2(2)")

    # Check that all parameters exist
    def test_config_2(self):
        j = { "id": id, "mdns": "gravmon",  "temp-format": "C", "sleep-interval": 300 }
        r = call_api_post( "/api/config/device", j )
        self.assertEqual(r.status_code, 200)

    def test_config_3(self):
        j = { "id": id, "http-push": "https://push.me", "token": "mytoken", "token2": "mytoken2", "http-push2": "http://push.me", "http-push3": "http://push.me", "influxdb2-push": "http://influx.db", "influxdb2-org": "my-org", 
              "influxdb2-bucket": "my-bucket", "influxdb2-auth": "my-secret", "mqtt-push": "mqtt.com", "mqtt-port": 1883, "mqtt-user": "my-user", 
              "mqtt-pass": "my-pass" }
        r = call_api_post( "/api/config/push", j )
        self.assertEqual(r.status_code, 200)

    def test_config_4(self):
        j = { "id": id, "ota-url": "http://ota.url/path", "voltage-factor": 1.55, "temp-adjustment-value": -2, "gyro-temp": "on", "ble": "color" }
        r = call_api_post( "/api/config/hardware", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["id"], id)
        self.assertEqual(j["ble"], "color")
        self.assertEqual(j["ota-url"], "http://ota.url/path")
        self.assertEqual(j["voltage-factor"], 1.55)
        self.assertEqual(j["temp-adjustment-value"], -2)
        self.assertEqual(j["gyro-temp"], True)
        # These are read only here, just checking them.
        self.assertNotEqual(j["gyro-calibration-data"]["ax"], 0.0001)
        self.assertNotEqual(j["gyro-calibration-data"]["ay"], 0.0001)
        self.assertNotEqual(j["gyro-calibration-data"]["az"], 0.0001)
        self.assertNotEqual(j["gyro-calibration-data"]["gx"], 0.0001)
        self.assertNotEqual(j["gyro-calibration-data"]["gy"], 0.0001)
        self.assertNotEqual(j["gyro-calibration-data"]["gz"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["a1"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["a2"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["a3"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["a4"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["a5"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["a6"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["a7"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["a8"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["a9"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["a10"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["g1"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["g2"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["g3"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["g4"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["g5"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["g6"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["g7"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["g8"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["g9"], 0.0001)
        self.assertNotEqual(j["formula-calculation-data"]["g10"], 0.0001)
        self.assertNotEqual(j["angle"], 0)
        self.assertNotEqual(j["gravity"], -10)
        self.assertNotEqual(j["battery"], 0)

    def test_config_5(self):
        j = { "id": id, "gravity-formula": "my-formula", "gravity-temp-adjustment": "on", "gravity-format": "G" }
        r = call_api_post( "/api/config/gravity", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["id"], id)
        self.assertEqual(j["gravity-format"], "G")
        self.assertEqual(j["gravity-formula"], "my-formula")
        self.assertEqual(j["gravity-temp-adjustment"], True)

    def test_config_6(self):
        j = { "id": id, "mdns": "gravmon",  "temp-format": "F", "sleep-interval": 300 }
        r = call_api_post( "/api/config/device", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["id"], id)
        self.assertEqual(j["mdns"], "gravmon")
        self.assertEqual(j["wifi-ssid"], "@home")
        self.assertEqual(j["wifi-pass"], "") # Should not be displayed in API
        self.assertEqual(j["temp-format"], "F")
        self.assertEqual(j["sleep-interval"], 300)

    def test_config_7(self):
        j = { "id": id, "ota-url": "", "voltage-factor": 1.55, "temp-adjustment-value": -2, "gyro-temp": "off", "ble": "blue" }
        r = call_api_post( "/api/config/hardware", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["id"], id)
        self.assertEqual(j["ble"], "blue")
        self.assertEqual(j["ota-url"], "")
        self.assertEqual(j["voltage-factor"], 1.55)
        self.assertEqual(j["temp-adjustment-value"], -2)
        self.assertEqual(j["gyro-temp"], False)

    def test_config_8(self):
        j = { "id": id, "gravity-formula": "", "gravity-temp-adjustment": "off", "gravity-format": "P" }
        r = call_api_post( "/api/config/gravity", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["gravity-format"], "P")
        self.assertEqual(j["gravity-temp-adjustment"], False)
        self.assertEqual(j["gravity-formula"], "")

    def test_config_9(self):
        j = { "id": id, "gravity-temp-adjustment": "on" }
        r = call_api_post( "/api/config/gravity", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["gravity-temp-adjustment"], True)

        j = { "id": id } # No checkbox tag should set it to false.
        r = call_api_post( "/api/config/gravity", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["gravity-temp-adjustment"], False)

    def test_config_A(self):
        j = { "id": id, "gyro-temp": "on" }
        r = call_api_post( "/api/config/hardware", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["gyro-temp"], True)

        j = { "id": id } # No checkbox tag should set it to false.
        r = call_api_post( "/api/config/hardware", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["gyro-temp"], False)

    # Check formula api (sg mode)
    def test_formula_sg_1(self):
        # Ensure we have SG defined as gravity
        j = { "id": id, "gravity-formula": "", "gravity-temp-adjustment": "off", "gravity-format": "G" } 
        r = call_api_post("/api/config/gravity", j)
        self.assertEqual(r.status_code, 200)

    def test_formula_sg_2(self):
        # Fails due to wrong id
        j = { "id": "wrong", "g1": 0, "g2": 1, "g3": 1.02, "g4": 1.0333, "g5": 1.00011, "g6": 1, "g7": 1, "g8": 1, "g9": 1, "g10": 1, "a1": 0, "a2": 25, "a3": 25.5, "a4": 25.55, "a5": 25.555, "a6": 0, "a7": 0, "a8": 0, "a9": 0, "a10": 0, "gravity-formula": "ThisShouldChange" }
        r = call_api_post("/api/formula", j)
        self.assertNotEqual(r.status_code, 200)

        # Fails due to to few values
        j = { "id": id, "g1": 0, "g2": 1, "g3": 1.02, "g4": 1.0333, "g5": 1.00011, "g6": 1, "g7": 1, "g8": 1, "g9": 1, "g10": 1, "a1": 0, "a2": 25, "a3": 0, "a4": 0, "a5": 0, "a6": 0, "a7": 0, "a8": 0, "a9": 0, "a10": 0, "gravity-formula": "ThisShouldChange" }
        r = call_api_post("/api/formula", j)
        self.assertEqual(r.status_code, 200) 

        # Checks that values from last call was stored
        r = call_api_get( "/api/formula" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["gravity-formula"], "")
        self.assertEqual(j["error"], "Not enough values to create formula, need at least 3 angles.")

    def test_formula_sg_3(self):
        # Check a simple formula
        j = { "id": id, "g1": 1.0, "g2": 1.01, "g3": 1.02, "g4": 1.03, "g5": 1.04, "g6": 1.05, "g7": 1.06, "g8": 1.07, "g9": 1.08, "g10": 1.1, "a1": 25, "a2": 30, "a3": 35, "a4": 40, "a5": 45, "a6": 50, "a7": 55, "a8": 60, "a9": 65, "a10": 70, "gravity-formula": "ThisShouldChange" }
        r = call_api_post("/api/formula", j)
        self.assertEqual(r.status_code, 200) 

        r = call_api_get( "/api/formula" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["gravity-formula"], "0.00000909*tilt^2+0.00124545*tilt+0.96445455")

    # Check formula api (plato mode)
    def test_formula_plato_1(self):
        # Ensure we have Plato defined as gravity
        j = { "id": id, "gravity-formula": "", "gravity-temp-adjustment": "off", "gravity-format": "P" } 
        r = call_api_post("/api/config/gravity", j)
        self.assertEqual(r.status_code, 200)

    def test_formula_plato_2(self):
        # Fails due to to few values
        j = { "id": id, "g1": 0, "g2": 3, "g3": 5.3, "g4": 7.44, "g5": 8.555, "g6": 9, "g7": 9.1, "g8": 9.2, "g9": 9.3, "g10": 9.4, "a1": 0, "a2": 25, "a3": 25.5, "a4": 25.55, "a5": 25.5555, "a6": 35, "a7": 36, "a8": 37, "a9": 38, "a10": 39, "gravity-formula": "ThisShouldChange" }
        r = call_api_post("/api/formula", j)
        self.assertEqual(r.status_code, 200) 

        # Checks that values from last call was stored
        r = call_api_get( "/api/formula" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["id"], id)
        self.assertEqual(j["g1"], 0)
        self.assertEqual(j["g2"], 3)
        self.assertEqual(j["g3"], 5.3)
        self.assertEqual(j["g4"], 7.44)
        self.assertEqual(j["g5"], 8.55)
        self.assertEqual(j["g6"], 9)
        self.assertEqual(j["g7"], 9.1)
        self.assertEqual(j["g8"], 9.2)
        self.assertEqual(j["g9"], 9.3)
        self.assertEqual(j["g10"], 9.4)
        self.assertEqual(j["a1"], 0)
        self.assertEqual(j["a2"], 25)
        self.assertEqual(j["a3"], 25.5)
        self.assertEqual(j["a4"], 25.55)
        self.assertEqual(j["a5"], 25.556)
        self.assertEqual(j["a6"], 35)
        self.assertEqual(j["a7"], 36)
        self.assertEqual(j["a8"], 37)
        self.assertEqual(j["a9"], 38)
        self.assertEqual(j["a10"], 39)
        self.assertEqual(j["gravity-format"], "P")
        self.assertEqual(j["gravity-formula"], "-0.00012160*tilt^2+0.00875074*tilt+0.87998845")
        self.assertEqual(j["error"], "")

    def test_formula_plato_3(self):
        j = { "id": id, "g1": 0, "g2": 3, "g3": 0, "g4": 0, "g5": 0, "g6": 0, "g7": 0, "g8": 0, "g9": 0, "g10": 0, "a1": 0, "a2": 25, "a3": 0, "a4": 0, "a5": 0, "a6": 0, "a7": 0, "a8": 0, "a9": 0, "a10": 0, "gravity-formula": "ThisShouldChange" }
        r = call_api_post("/api/formula", j)
        self.assertEqual(r.status_code, 200) 

        # Checks that values from last call was stored
        r = call_api_get( "/api/formula" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["error"], "Not enough values to create formula, need at least 3 angles.")

    def test_formula_plato_4(self):
        # Ensure we have Plato defined as gravity
        j = { "id": id, "gravity-formula": "", "gravity-temp-adjustment": "off", "gravity-format": "P" } 
        r = call_api_post("/api/config/gravity", j)
        self.assertEqual(r.status_code, 200)

        # Check a simple formula
        j = { "id": id, "g1": 1.0, "g2": 1.1, "g3": 1.2, "g4": 1.3, "g5": 1.4, "g6": 1.5, "g7": 1.6, "g8": 1.7, "g9": 1.8, "g10": 1.9, "a1": 25, "a2": 30, "a3": 35, "a4": 40, "a5": 45, "a6": 50, "a7": 55, "a8": 60, "a9": 65, "a10": 70, "gravity-formula": "ThisShouldChange" }
        r = call_api_post("/api/formula", j)
        self.assertEqual(r.status_code, 200) 

        r = call_api_get( "/api/formula" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        #self.assertEqual(j["gravity-formula"], "0.00000001*tilt^2+0.00007752*tilt+1.00193428")  # 1.6 max deviation
        self.assertEqual(j["gravity-formula"], "-0.00000352*tilt^2+0.00045454*tilt+0.99231483")  # 3.0 max deviation
       
            # Check format api
    def test_pushtest_1(self):
        j = { "id": id, "http-push": "http://unknown.push.me", "token": "mytoken", "token2": "mytoken2", "http-push2": "http://unknown.push.me", "http-push3": "http://unknown.push.me", "brewfather-push": "http://unknown.push.me", "influxdb2-push": "http://influx.db", "influxdb2-org": "my-org", 
              "influxdb2-bucket": "my-bucket", "influxdb2-auth": "my-secret", "mqtt-push": "mqtt.com", "mqtt-port": 1883, "mqtt-user": "my-user", 
              "mqtt-pass": "my-pass", "http-push-h1": "header1", "http-push-h2": "header2", "http-push2-h1": "header1(2)", "http-push2-h2": "header2(2)" }
        r = call_api_post( "/api/config/push", j )
        self.assertEqual(r.status_code, 200)

    def test_pushtest_2(self):
        r = call_api_get( "/api/test/push?id=" + id + "&format=http-1" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["success"], False)
        self.assertEqual(j["enabled"], True)
        self.assertEqual(j["code"], -1)

    def test_pushtest_3(self):
        r = call_api_get( "/api/test/push?id=" + id + "&format=http-2" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["success"], False)
        self.assertEqual(j["enabled"], True)
        self.assertEqual(j["code"], -1)

    def test_pushtest_4(self):
        r = call_api_get( "/api/test/push?id=" + id + "&format=http-3" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["success"], False)
        self.assertEqual(j["enabled"], True)
        self.assertEqual(j["code"], -1)

    def test_pushtest_5(self):
        r = call_api_get( "/api/test/push?id=" + id + "&format=influxdb" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["success"], False)
        self.assertEqual(j["enabled"], True)
        self.assertEqual(j["code"], -1)

    def test_pushtest_6(self):
        r = call_api_get( "/api/test/push?id=" + id + "&format=mqtt" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["success"], False)
        self.assertEqual(j["enabled"], True)
        self.assertEqual(j["code"], -3)

    # Check format api
    def test_push_1(self):
        r = call_api_get( "/api/factory?id=" + id)
        time.sleep(4)

        # Note: The endpoint test.php does not validate the payload, it only accepts the request and return 200.
        j = { "id": id, "http-push": "http://www.allerum.net/test.php", "http-push2": "http://www.allerum.net/test.php", "http-push3": "http://www.allerum.net/test.php", "mqtt-push": "192.168.1.16", "mqtt-port": 1883, "mqtt-user": "", "mqtt-pass": "" }
        r = call_api_post( "/api/config/push", j )
        self.assertEqual(r.status_code, 200)

    def test_push_2(self):
        r = call_api_get( "/api/test/push?id=" + id + "&format=http-1" )
        j = json.loads(r.text)
        print(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["success"], True)

    def test_push_3(self):
        r = call_api_get( "/api/test/push?id=" + id + "&format=http-2" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["success"], True)

    def test_push_4(self):
        r = call_api_get( "/api/test/push?id=" + id + "&format=http-3" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["success"], True)

    def test_push_5(self):
        r = call_api_get( "/api/test/push?id=" + id + "&format=mqtt" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["success"], True)

    def test_push_6(self):
        r = call_api_get( "/api/test/push?id=" + id + "&format=influx" )
        # TODO: Figure out how to test the influx db setup. Create my own mockup ?

    # Check format api
    def test_format_1(self):
        j = { "id": id, "http-1": "one" }
        r = call_api_post( "/api/config/format", j )
        self.assertEqual(r.status_code, 200)

    def test_format_2(self):
        j = { "id": id, "http-2": "two" }
        r = call_api_post( "/api/config/format", j )
        self.assertEqual(r.status_code, 200)

    def test_format_3(self):
        j = { "id": id, "http-3": "five" }
        r = call_api_post( "/api/config/format", j )
        self.assertEqual(r.status_code, 200)

    def test_format_4(self):
        j = { "id": id, "influxdb": "three" }
        r = call_api_post( "/api/config/format", j )
        self.assertEqual(r.status_code, 200)

    def test_format_5(self):
        j = { "id": id, "mqtt": "four" }
        r = call_api_post( "/api/config/format", j )
        self.assertEqual(r.status_code, 200)

    def test_format_6(self):
        r = call_api_get( "/api/config/format" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["id"], id)
        self.assertEqual(j["http-1"], "one")
        self.assertEqual(j["http-2"], "two")
        self.assertEqual(j["http-3"], "five")
        self.assertEqual(j["influxdb"], "three")
        self.assertEqual(j["mqtt"], "four")

    def test_format_7(self):
        j = { "id": id, "http-1": "" }
        r = call_api_post( "/api/config/format", j )
        self.assertEqual(r.status_code, 200)

    def test_format_8(self):
        j = { "id": id, "http-2": "" }
        r = call_api_post( "/api/config/format", j )
        self.assertEqual(r.status_code, 200)

    def test_format_9(self):
        j = { "id": id, "http-3": "" }
        r = call_api_post( "/api/config/format", j )
        self.assertEqual(r.status_code, 200)

    def test_format_A(self):
        j = { "id": id, "influxdb": "" }
        r = call_api_post( "/api/config/format", j )
        self.assertEqual(r.status_code, 200)

    def test_format_B(self):
        j = { "id": id, "mqtt": "" }
        r = call_api_post( "/api/config/format", j )
        self.assertEqual(r.status_code, 200)

    def test_format_C(self):
        r = call_api_get( "/api/config/format" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        #print(j["http-1"])
        #print(j["http-2"])
        #print(j["http-3"])
        #print(j["influxdb"])
        #print(j["mqtt"])
        self.assertEqual(j["http-1"], "%7B%22name%22%20%3A%20%22%24%7Bmdns%7D%22%2C%20%22ID%22%3A%20%22%24%7Bid%7D%22%2C%20%22token%22%20%3A%20%22%24%7Btoken%7D%22%2C%20%22interval%22%3A%20%24%7Bsleep%2Dinterval%7D%2C%20%22temperature%22%3A%20%24%7Btemp%7D%2C%20%22temp%5Funits%22%3A%20%22%24%7Btemp%2Dunit%7D%22%2C%20%22gravity%22%3A%20%24%7Bgravity%7D%2C%20%22angle%22%3A%20%24%7Bangle%7D%2C%20%22battery%22%3A%20%24%7Bbattery%7D%2C%20%22RSSI%22%3A%20%24%7Brssi%7D%2C%20%22corr%2Dgravity%22%3A%20%24%7Bcorr%2Dgravity%7D%2C%20%22gravity%2Dunit%22%3A%20%22%24%7Bgravity%2Dunit%7D%22%2C%20%22run%2Dtime%22%3A%20%24%7Brun%2Dtime%7D%20%7D")
        self.assertEqual(j["http-2"], "%7B%22name%22%20%3A%20%22%24%7Bmdns%7D%22%2C%20%22ID%22%3A%20%22%24%7Bid%7D%22%2C%20%22token%22%20%3A%20%22%24%7Btoken%7D%22%2C%20%22interval%22%3A%20%24%7Bsleep%2Dinterval%7D%2C%20%22temperature%22%3A%20%24%7Btemp%7D%2C%20%22temp%5Funits%22%3A%20%22%24%7Btemp%2Dunit%7D%22%2C%20%22gravity%22%3A%20%24%7Bgravity%7D%2C%20%22angle%22%3A%20%24%7Bangle%7D%2C%20%22battery%22%3A%20%24%7Bbattery%7D%2C%20%22RSSI%22%3A%20%24%7Brssi%7D%2C%20%22corr%2Dgravity%22%3A%20%24%7Bcorr%2Dgravity%7D%2C%20%22gravity%2Dunit%22%3A%20%22%24%7Bgravity%2Dunit%7D%22%2C%20%22run%2Dtime%22%3A%20%24%7Brun%2Dtime%7D%20%7D")
        self.assertEqual(j["http-3"], "%3Fname%3D%24%7Bmdns%7D%26id%3D%24%7Bid%7D%26token%3D%24%7Btoken2%7D%26interval%3D%24%7Bsleep%2Dinterval%7D%26temperature%3D%24%7Btemp%7D%26temp%2Dunits%3D%24%7Btemp%2Dunit%7D%26gravity%3D%24%7Bgravity%7D%26angle%3D%24%7Bangle%7D%26battery%3D%24%7Bbattery%7D%26rssi%3D%24%7Brssi%7D%26corr%2Dgravity%3D%24%7Bcorr%2Dgravity%7D%26gravity%2Dunit%3D%24%7Bgravity%2Dunit%7D%26run%2Dtime%3D%24%7Brun%2Dtime%7D")
        self.assertEqual(j["influxdb"], "measurement%2Chost%3D%24%7Bmdns%7D%2Cdevice%3D%24%7Bid%7D%2Ctemp%2Dformat%3D%24%7Btemp%2Dunit%7D%2Cgravity%2Dformat%3D%24%7Bgravity%2Dunit%7D%20gravity%3D%24%7Bgravity%7D%2Ccorr%2Dgravity%3D%24%7Bcorr%2Dgravity%7D%2Cangle%3D%24%7Bangle%7D%2Ctemp%3D%24%7Btemp%7D%2Cbattery%3D%24%7Bbattery%7D%2Crssi%3D%24%7Brssi%7D%0A")
        self.assertEqual(j["mqtt"], "ispindel%2F%24%7Bmdns%7D%2Ftilt%3A%24%7Bangle%7D%7Cispindel%2F%24%7Bmdns%7D%2Ftemperature%3A%24%7Btemp%7D%7Cispindel%2F%24%7Bmdns%7D%2Ftemp%5Funits%3A%24%7Btemp%2Dunit%7D%7Cispindel%2F%24%7Bmdns%7D%2Fbattery%3A%24%7Bbattery%7D%7Cispindel%2F%24%7Bmdns%7D%2Fgravity%3A%24%7Bgravity%7D%7Cispindel%2F%24%7Bmdns%7D%2Finterval%3A%24%7Bsleep%2Dinterval%7D%7Cispindel%2F%24%7Bmdns%7D%2FRSSI%3A%24%7Brssi%7D%7C")

    # Toggle sleep mode
    def toggle_sleepmode_1(self):
        j = { "id": id, "sleep-mode": "on" }
        r = call_api_post( "/api/status/sleepmode" )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/status" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["sleep-mode"], True)

    def toggle_sleepmode_2(self):
        j = { "id": id, "sleep-mode": "off" }
        r = call_api_post( "/api/status/sleepmode" )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/status" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["sleep-mode"], False)

    # Clear setting
    def default_settings_1(self):
        j = { "id": id, "mdns": "gravmon",  "temp-format": "C", "sleep-interval": 300 }
        r = call_api_post( "/api/config/device", j )
        self.assertEqual(r.status_code, 200)

    def default_settings_2(self):
        j = { "id": id, "token": "", "token2": "", "http-push": "", "http-push2": "", "http-push3": "", "influxdb2-push": "", "influxdb2-org": "", "influxdb2-bucket": "", 
              "influxdb2-auth": "", "mqtt-push": "", "mqtt-port": 1883, "mqtt-user": "", "mqtt-pass": "" }
        r = call_api_post( "/api/config/push", j )
        self.assertEqual(r.status_code, 200)

    def default_settings_3(self):
        j = { "id": id, "ota-url": "", "voltage-factor": 1.55, "temp-adjustment-value": -2, "gyro-temp": "off" }
        r = call_api_post( "/api/config/hardware", j )
        self.assertEqual(r.status_code, 200)

    def default_settings_4(self):
        j = { "id": id, "gravity-formula": "", "gravity-temp-adjustment": "off", "gravity-format": "G" }
        r = call_api_post( "/api/config/gravity", j )
        self.assertEqual(r.status_code, 200)

    # Check advanced
    def test_advanced_config_1(self):
        r = call_api_get( "/api/status" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["id"], id)
        r = call_api_get( "/api/factory?id=" + j["id"])
        time.sleep(4)

        r = call_api_get( "/api/config/advanced" )
        j = json.loads(r.text)
        #print(j)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["gyro-read-count"], 50)
        self.assertEqual(j["tempsensor-resolution"], 9)
        self.assertEqual(j["gyro-moving-threashold"], 500)
        self.assertEqual(j["formula-max-deviation"], 3)
        self.assertEqual(j["formula-calibration-temp"], 20)
        self.assertEqual(j["wifi-portal-timeout"], 120)
        self.assertEqual(j["wifi-connect-timeout"], 20)
        self.assertEqual(j["ignore-low-angles"], False)
        self.assertEqual(j["battery-saving"], True)
        self.assertEqual(j["formula-calibration-temp"], 20)
        self.assertEqual(j["int-http1"], 0)
        self.assertEqual(j["int-http2"], 0)
        self.assertEqual(j["int-http3"], 0)
        self.assertEqual(j["int-influx"], 0)
        self.assertEqual(j["int-mqtt"], 0)

    def test_advanced_config_2(self):
        j = { "id": id, "gyro-read-count": 51, "tempsensor-resolution": 10, "gyro-moving-threashold": 501, "formula-max-deviation": 1.7, "ignore-low-angles": "on",
              "formula-calibration-temp": 21, "wifi-portal-timeout": 121, "wifi-connect-timeout": 21, "int-http1": 1, "int-http2": 2, "int-http3": 3, "int-influx": 4, "int-mqtt": 5, "battery-saving": "off"  }
        r = call_api_post( "/api/config/advanced", j )
        self.assertEqual(r.status_code, 200)
    
        r = call_api_get( "/api/config/advanced" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["gyro-read-count"], 51)
        self.assertEqual(j["tempsensor-resolution"], 10)
        self.assertEqual(j["gyro-moving-threashold"], 501)
        self.assertEqual(j["formula-max-deviation"], 1.7)
        self.assertEqual(j["formula-calibration-temp"], 21)
        self.assertEqual(j["wifi-portal-timeout"], 121)
        self.assertEqual(j["wifi-connect-timeout"], 21)
        self.assertEqual(j["ignore-low-angles"], True)
        self.assertEqual(j["battery-saving"], False)
        self.assertEqual(j["int-http1"], 1)
        self.assertEqual(j["int-http2"], 2)
        self.assertEqual(j["int-http3"], 3)
        self.assertEqual(j["int-influx"], 4)
        self.assertEqual(j["int-mqtt"], 5)

    def test_advanced_config_3(self):
        j = { "id": id, "ignore-low-angles": "on" }
        r = call_api_post( "/api/config/advanced", j )
        self.assertEqual(r.status_code, 200)
    
        r = call_api_get( "/api/config/advanced" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["ignore-low-angles"], True)

        j = { "id": id } # Skipping checkbox variable should set it to false
        r = call_api_post( "/api/config/advanced", j )
        self.assertEqual(r.status_code, 200)
    
        r = call_api_get( "/api/config/advanced" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["ignore-low-angles"], False)

    def test_bug_71(self):
        format = "gm%3A%7B%22name%22%3A%22%24%7Bmdns%7D%22%2C%20%22ID%22%3A%22%24%7Bid%7D%22%2C%20%22temperature%22%3A%20%24%7Btemp%7D%2C%20%22gravity%22%3A%24%7Bgravity%7D%2C%22angle%22%3A%20%24%7Bangle%7D%2C%20%22battery%22%3A%24%7Bbattery%7D%2C%20%22rssi%22%3A%20%24%7Brssi%7D%7D"
        j = { "id": id, "mqtt": format }
        r = call_api_post( "/api/config/format", j )
        self.assertEqual(r.status_code, 200)

        r = call_api_get( "/api/config/format" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["mqtt"], format)

    def test_bug_79(self):
        j = { "id": id, "formula-max-deviation": 1.7 }
        r = call_api_post( "/api/config/advanced", j )
        self.assertEqual(r.status_code, 200)

        j = { "id": id, "g1": 1.0, "g2": 1.009, "g3": 1.014, "g4": 1.027, "g5": 1.037, "g6": 1.042, "g7": 1.051, "g8": 1.060, "g9": 1.073, "g10": 1.078, "a1": 23.52, "a2": 26.47, "a3": 29.87, "a4": 33.43, "a5": 38.16, "a6": 40.6, "a7": 45.85, "a8": 50.12, "a9": 56.55, "a10": 59.078, "gravity-formula": "" }
        r = call_api_post("/api/formula", j)
        self.assertEqual(r.status_code, 200) 

        r = call_api_get( "/api/formula" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["error"], 'Unable to find an accurate formula based on input, check error log and graph below.')

        j = { "id": id, "formula-max-deviation": 4 }
        r = call_api_post( "/api/config/advanced", j )
        self.assertEqual(r.status_code, 200)

        j = { "id": id, "g1": 1.0, "g2": 1.009, "g3": 1.014, "g4": 1.027, "g5": 1.037, "g6": 1.042, "g7": 1.051, "g8": 1.060, "g9": 1.073, "g10": 1.078, "a1": 23.52, "a2": 26.47, "a3": 29.87, "a4": 33.43, "a5": 38.16, "a6": 40.6, "a7": 45.85, "a8": 50.12, "a9": 56.55, "a10": 59.078, "gravity-formula": "" }
        r = call_api_post("/api/formula", j)
        self.assertEqual(r.status_code, 200) 

        r = call_api_get( "/api/formula" )
        j = json.loads(r.text)
        self.assertEqual(r.status_code, 200)
        self.assertEqual(j["error"], '')

    def test_wifi(self):
        j = { "id": id, "wifi-ssid": "ssid1", "wifi-ssid2": "ssid2", "wifi-pass": "pass1", "wifi-pass2": "pass2",   }
        r = call_api_post( "/api/config/wifi", j )
        self.assertEqual(r.status_code, 200)


if __name__ == '__main__':
    unittest.main()
