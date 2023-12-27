import unittest, time, logging

from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.select import Select
from selenium.webdriver.support.wait import WebDriverWait
from selenium.webdriver.remote.remote_connection import LOGGER
#
# Note! UI tests might fail on a smaller monitor since all 
# elements needs to be visible for the automations to work.
#

# pytesy uitests.py -k test_02_index

url = "http://192.168.1.160"

LOGGER.setLevel(logging.FATAL)

class GM(unittest.TestCase):

    def setUp(self):
        self.baseUrl = url

        options = webdriver.EdgeOptions()
        #options.add_argument("--headless=new")
        options.add_argument('log-level=3')
        service = webdriver.EdgeService(service_args=['--log-level=OFF'], log_output="./log")

        self.driver = webdriver.Edge(options=options, service=service)
        self.wait = WebDriverWait(self.driver, timeout=20)
        self.driver.maximize_window()

    def tearDown(self):
        self.driver.quit()
  
    def find_element(self, id):
        return self.driver.find_element(by=By.ID, value=id)

    def wait_for_alert_success(self):
        alert = self.find_element("alert")
        alertmsg = self.find_element("alert-msg")
        self.wait.until(lambda d : alert.get_dom_attribute("class").find("show") )      
        assert "alert-success" in alert.get_dom_attribute("class").split(' '), f"Failed message=. {alertmsg.text}"

    def set_textfield(self, field, value):
        f = self.find_element(field)
        f.clear()
        f.send_keys(value)

    def set_selectfield(self, field, array):
        f = Select(self.find_element(field))
        for a in array:
            f.select_by_value(a)

    def get_textfield(self, field):
        return self.find_element(field).get_attribute("value")

    def print(self, field):
        e = self.find_element(field)
        print(f"Displayed={e.is_displayed()}, Enabled={e.is_enabled()}" )

    def test_01_factory(self):
        print("test_01_factory()")
        self.driver.get(self.baseUrl + "/config.htm")     
        assert self.driver.current_url == self.baseUrl + "/config.htm"
        time.sleep(1)
        self.find_element("factory-btn").click()
        self.wait.until(lambda d : self.find_element("factory-save-btn").is_displayed() == True )
        self.find_element("factory-save-btn").click()
        self.wait_for_alert_success()
        time.sleep(5) # Wait for device to reboot
        self.wait.until(lambda d : self.find_element("sleep-interval").is_enabled() == True )
        sleep = self.get_textfield("sleep-interval")
        assert sleep == "900", print(f"sleep-interval={sleep}")

    def test_02_index(self):
        print("test_02_index()")
        self.driver.get(self.baseUrl + "/")     
        assert self.driver.current_url == self.baseUrl + "/"
        self.driver.get(self.baseUrl + "/index.htm")     
        assert self.driver.current_url == self.baseUrl + "/index.htm"

        self.find_element("clear-btn")
        self.find_element("log-btn")
        self.find_element("issue-btn")
        self.find_element("support-btn")


    def test_10_config(self):
        print("test_10_config()")
        self.driver.get(self.baseUrl + "/config.htm")     
        assert self.driver.current_url == self.baseUrl + "/config.htm"
        time.sleep(1)

        # Testing device section ################################################################
        self.find_element("headingDevice-btn")
        self.set_textfield("mdns", "my_mdns")
        self.find_element("temp-format-c").click()
        self.find_element("temp-format-f").click()
        self.set_textfield("sleep-interval", "256")
        self.find_element("device-btn").click()
        self.wait.until(lambda d : self.find_element("device-btn").is_enabled() == True )
        self.find_element("restart-btn")

        mdns = self.get_textfield("mdns")
        assert mdns == "my_mdns", print(f"mdns={mdns}")
        assert self.find_element("temp-format-c").is_selected() == False
        assert self.find_element("temp-format-f").is_selected() == True
        sleep = self.get_textfield("sleep-interval")
        assert sleep == "256", print(f"sleep-interval={sleep}")

    def test_11_config(self):
        print("test_11_config()")
        self.driver.get(self.baseUrl + "/config.htm#collapsePush")     
        assert self.driver.current_url == self.baseUrl + "/config.htm#collapsePush"
        time.sleep(1)

        # Testing push 1 section ################################################################

        self.wait.until(lambda d : self.find_element("http-push").is_displayed() == True )
        self.set_textfield("http-push", "http://push1.me")
        self.set_textfield("http-push2", "http://push2.me")
        self.set_textfield("http-push3", "http://push3.me")
        self.set_textfield("token", "my_token")
        self.set_textfield("token2", "my_token2")

        self.find_element("push-btn").click()
        self.wait.until(lambda d : self.find_element("push-btn").is_enabled() == True )
        self.find_element("format-btn")
        self.find_element("test-btn")

        push = self.get_textfield("http-push")
        assert push == "http://push1.me", print(f"http-push={push}")
        push2 = self.get_textfield("http-push2")
        assert push2 == "http://push2.me", print(f"http-push2={push2}")
        push3 = self.get_textfield("http-push3")
        assert push3 == "http://push3.me", print(f"http-push3={push3}")
        token = self.get_textfield("token")
        assert token == "my_token", print(f"token={token}")
        token2 = self.get_textfield("token2")
        assert token2 == "my_token2", print(f"token2={token2}")

    def test_12_config(self):
        print("test_12_config()")
        self.driver.get(self.baseUrl + "/config.htm#collapsePush2")     
        assert self.driver.current_url == self.baseUrl + "/config.htm#collapsePush2"
        time.sleep(1)

        # Testing push 2 section ################################################################

        self.wait.until(lambda d : self.find_element("influxdb2-push").is_displayed() == True )
        self.set_textfield("influxdb2-push", "http://push4.me")
        self.set_textfield("influxdb2-org", "home")
        self.set_textfield("influxdb2-bucket", "spann")
        self.set_textfield("influxdb2-auth", "autz")

        self.set_textfield("mqtt-push", "192.168.1.1")
        self.set_textfield("mqtt-port", "1884")
        self.set_textfield("mqtt-user", "user")
        self.set_textfield("mqtt-pass", "pass")

        self.find_element("push-btn2").click()
        self.wait.until(lambda d : self.find_element("push-btn2").is_enabled() == True )
        self.find_element("format-btn2")
        self.find_element("test-btn2")

        push = self.get_textfield("influxdb2-push")
        assert push == "http://push4.me", print(f"influxdb-push={push}")
        org = self.get_textfield("influxdb2-org")
        assert org == "home", print(f"influxdb-org={org}")
        bucket = self.get_textfield("influxdb2-bucket")
        assert bucket == "spann", print(f"influxdb-bucket={bucket}")
        auth = self.get_textfield("influxdb2-auth")
        assert auth == "autz", print(f"influxdb-auth={auth}")

        mqtt = self.get_textfield("mqtt-push")
        assert mqtt == "192.168.1.1", print(f"mqtt-push={mqtt}")
        port = self.get_textfield("mqtt-port")
        assert port == "1884", print(f"mqtt-port={port}")
        user = self.get_textfield("mqtt-user")
        assert user == "user", print(f"mqtt-user={user}")
        pwd = self.get_textfield("mqtt-pass")
        assert pwd == "pass", print(f"mqtt-pass={pwd}")

    def test_13_config(self):
        print("test_13_config()")
        self.driver.get(self.baseUrl + "/config.htm#collapseGravity")     
        assert self.driver.current_url == self.baseUrl + "/config.htm#collapseGravity"
        self.driver.execute_script("window.scrollTo(0, 8000);")
        time.sleep(1)

        # Testing gravity section ###############################################################

        self.wait.until(lambda d : self.find_element("gravity-formula").is_displayed() == True )
        self.find_element("gravity-format-g").click()
        self.find_element("gravity-format-p").click()
        self.find_element("gravity-format-g").click()
        self.find_element("formula-btn").click()
        self.set_textfield("gravity-formula", "tilt*2")
        self.find_element("gravity-temp-adjustment").click()
        self.find_element("gravity-btn").click()
        self.wait.until(lambda d : self.find_element("gravity-btn").is_enabled() == True )

        assert self.find_element("gravity-format-g").is_selected() == True
        assert self.find_element("gravity-format-p").is_selected() == False
        formula = self.get_textfield("gravity-formula")
        assert formula == "tilt*2", print(f"gravity-formula={formula}")
        assert self.find_element("gravity-temp-adjustment").is_selected() == True

    def test_14_config(self):
        print("test_14_config()")
        self.driver.get(self.baseUrl + "/config.htm#collapseHardware")     
        assert self.driver.current_url == self.baseUrl + "/config.htm#collapseHardware"
        self.driver.execute_script("window.scrollTo(0, 8000);")
        time.sleep(1)

        # Testing hardware section ##############################################################

        self.wait.until(lambda d : self.find_element("voltage-factor").is_displayed() == True )
        self.set_textfield("voltage-factor", "2,1")
        self.set_textfield("temp-adjustment-value", "-1,1")
        self.find_element("gyro-temp").click()
        self.find_element("storage-sleep").click()
        self.set_selectfield("ble-format", ["0","1","2","3","4"])
        self.set_selectfield("ble", ["red","green","black","purple","orange","blue","yellow","pink"])
        self.set_textfield("ota-url", "http://ota.com")
        self.find_element("hardware-btn").click()
        self.wait.until(lambda d : self.find_element("hardware-btn").is_enabled() == True )

        factor = self.get_textfield("voltage-factor")
        assert factor == "2.1", print(f"voltage-factor={factor}")
        adj = self.get_textfield("temp-adjustment-value")
        assert adj == "-1.1", print(f"temp-adjustment-value={adj}")
        assert self.find_element("gyro-temp").is_selected() == True
        assert self.find_element("storage-sleep").is_selected() == True
        ble = self.get_textfield("ble")
        assert ble == "pink", print(f"ble={ble}")
        ota = self.get_textfield("ota-url")
        assert ota == "http://ota.com", print(f"ota-url={ota}")

        self.find_element("gravitymon-com").click()
        self.find_element("volt-factor-btn")
        self.find_element("firmware-btn")

    def test_15_config(self):
        print("test_15_config()")
        self.driver.get(self.baseUrl + "/config.htm#collapseAdvanced")     
        assert self.driver.current_url == self.baseUrl + "/config.htm#collapseAdvanced"
        self.driver.execute_script("window.scrollTo(0, 8000);")
        time.sleep(1)

        # Testing advanced section ##############################################################

        self.wait.until(lambda d : self.find_element("adv-config").is_displayed() == True )
        self.find_element("adv-config").click()
        self.wait.until(lambda d : self.find_element("gyro-read-count").is_enabled() == True )
        self.set_textfield("gyro-read-count", "100")
        self.set_textfield("gyro-moving-threashold", "1000")
        self.set_textfield("formula-max-deviation", "6")
        self.find_element("ignore-low-angles").click()
        self.find_element("battery-saving").click()
        self.set_textfield("formula-calibration-temp", "69")
        self.set_textfield("tempsensor-resolution", "11")
        self.driver.execute_script("window.scrollTo(0, 8000);")
        time.sleep(1)
        self.set_textfield("wifi-connect-timeout", "12")
        self.set_textfield("wifi-portal-timeout", "14")
        self.set_textfield("int-http1", "1")
        self.set_textfield("int-http2", "2")
        self.set_textfield("int-http3", "3")
        self.set_textfield("int-influx", "4")
        self.set_textfield("int-mqtt", "5")
        self.driver.execute_script("window.scrollTo(0, 8000);")

        self.find_element("advanced-btn").click()
        self.wait.until(lambda d : self.find_element("advanced-btn").is_enabled() == True )

        gyro_read = self.get_textfield("gyro-read-count")
        assert gyro_read == "100", print(f"gyro-read-count={gyro_read}")
        gyro_moving = self.get_textfield("gyro-moving-threashold")
        assert gyro_moving == "1000", print(f"gyro-moving-threashold={gyro_moving}")
        formula_dev = self.get_textfield("formula-max-deviation")
        assert formula_dev == "6", print(f"formula-max-deviation={formula_dev}")
        assert self.find_element("ignore-low-angles").is_selected() == True
        assert self.find_element("battery-saving").is_selected() == False
        formula_temp = self.get_textfield("formula-calibration-temp")
        assert formula_temp == "69", print(f"formula-calibration-temp={formula_temp}")
        temp_res = self.get_textfield("tempsensor-resolution")
        assert temp_res == "11", print(f"tempsensor-resolution={temp_res}")
        wifi_connect = self.get_textfield("wifi-connect-timeout")
        assert wifi_connect == "12", print(f"wifi-connect-timeout={wifi_connect}")
        wifi_portal = self.get_textfield("wifi-portal-timeout")
        assert wifi_portal == "14", print(f"wifi-portal-timeout={wifi_portal}")
        http1 = self.get_textfield("int-http1")
        assert http1 == "1", print(f"int-http1={http1}")
        http2 = self.get_textfield("int-http2")
        assert http2 == "2", print(f"int-http2={http2}")
        http3 = self.get_textfield("int-http3")
        assert http3 == "3", print(f"int-http3={http3}")
        influx = self.get_textfield("int-influx")
        assert influx == "4", print(f"int-influx={influx}")
        mqtt = self.get_textfield("int-mqtt")
        assert mqtt == "5", print(f"int-mqtt={mqtt}")

    def test_16_headers(self):
        print("test_16_headers()")
        self.driver.get(self.baseUrl + "/config.htm#collapsePush")     
        assert self.driver.current_url == self.baseUrl + "/config.htm#collapsePush"
        time.sleep(1)

        self.find_element("header1-btn").click()
        self.wait.until(lambda d : self.find_element("header-close-btn").is_displayed() == True )
        self.wait.until(lambda d : self.find_element("header-close-btn").is_enabled() == True )
        self.find_element("header-close-btn").click()

        #self.driver.implicitly_wait(10)

    def test_16b_headers(self):
        print("test_16b_headers()")
        self.driver.get(self.baseUrl + "/config.htm#collapsePush")     
        assert self.driver.current_url == self.baseUrl + "/config.htm#collapsePush"
        time.sleep(1)

        self.find_element("header2-btn").click()
        self.wait.until(lambda d : self.find_element("header-close-btn").is_displayed() == True )
        self.wait.until(lambda d : self.find_element("header-close-btn").is_enabled() == True )
        self.find_element("header-close-btn").click()

    def test_17_wifi(self):
        print("test_17_wifi()")
        self.driver.get(self.baseUrl + "/config.htm")     
        assert self.driver.current_url == self.baseUrl + "/config.htm"
        time.sleep(1)
        self.wait.until(lambda d : self.find_element("wifi-btn").is_enabled() == True )
        self.find_element("wifi-btn").click()
        time.sleep(1)
        self.wait.until(lambda d : self.find_element("wifi-save-btn").is_enabled() == True )
        self.find_element("wifi-close-btn").click()

    def test_18_calibrate(self):
        print("test_18_calibrate()")
        self.driver.get(self.baseUrl + "/config.htm")     
        assert self.driver.current_url == self.baseUrl + "/config.htm"
        time.sleep(1)
        self.wait.until(lambda d : self.find_element("calibrate-btn").is_enabled() == True )
        self.find_element("calibrate-btn").click()
        self.wait_for_alert_success()

    def test_20_about(self):
        print("test_20_about()")
        self.driver.get(self.baseUrl + "/about.htm")     
        assert self.driver.current_url == self.baseUrl + "/about.htm"

    def test_30_backup(self):
        print("test_30_backup()")
        self.driver.get(self.baseUrl + "/backup.htm")     
        assert self.driver.current_url == self.baseUrl + "/backup.htm"
        
    def test_40_calibration(self):
        print("test_40_calibration()")
        self.driver.get(self.baseUrl + "/calibration.htm")     
        assert self.driver.current_url == self.baseUrl + "/calibration.htm"
        time.sleep(1)

        self.set_textfield("a1", "25")
        self.set_textfield("a2", "35")
        self.set_textfield("a3", "45")
        self.set_textfield("a4", "55")
        self.set_textfield("a5", "")
        self.set_textfield("a6", "")
        self.set_textfield("a7", "")
        self.set_textfield("a8", "")
        self.set_textfield("a9", "")
        self.set_textfield("a10", "")

        #self.set_textfield("g1", "25")
        self.set_textfield("g2", "1.010")
        self.set_textfield("g3", "1.020")
        self.set_textfield("g4", "1.030")
        self.set_textfield("g5", "")
        self.set_textfield("g6", "")
        self.set_textfield("g7", "")
        self.set_textfield("g8", "")
        self.set_textfield("g9", "")
        self.set_textfield("g10", "")

        self.driver.execute_script("window.scrollTo(0, 8000);")
        time.sleep(1)

        self.find_element("calculate-btn").click()
        self.wait.until(lambda d : self.find_element("formula").text == "-0.00000000*tilt^2+0.00100000*tilt+0.97500000" )      

        self.set_textfield("a5", "60")
        self.set_textfield("g5", "1.040")

        self.find_element("calculate-btn").click()
        self.wait.until(lambda d : self.find_element("formula").text == "0.00000872*tilt^2+0.00035289*tilt+0.98615385" )      

    def test_50_format(self):
        print("test_50_format()")
        self.driver.get(self.baseUrl + "/format.htm")     
        assert self.driver.current_url == self.baseUrl + "/format.htm"

        self.wait.until(lambda d : self.find_element("push-target").is_enabled() == True)

        self.set_selectfield("push-target", [ "http-1", "http-2", "http-3", "influxdb", "mqtt"])
        self.set_selectfield("predefined", [ "iSpindle-Post", "GravityMon-Post", "iSpindle-Mqtt", "HomeAssistant-Mqtt", "HomeAssistant-Mqtt2", "UBIDots-Post", "BrewFatherCustom-Post", "iSpindle-Post", "GravityMon-Get", "Brewblox-Mqtt"])

        self.find_element("copy-btn").click()
        self.find_element("test-btn").click()
        self.find_element("clear-btn").click()

        self.find_element("format-btn").click()
        self.wait_for_alert_success()

    def test_60_firmware(self):
        print("test_60_firmware()")
        self.driver.get(self.baseUrl + "/firmware.htm")     
        assert self.driver.current_url == self.baseUrl + "/firmware.htm"

    def test_70_test(self):
        print("test_70_test()")
        self.driver.get(self.baseUrl + "/test.htm")     
        assert self.driver.current_url == self.baseUrl + "/test.htm"
        
        self.find_element("test-btn").click()
        self.wait.until(lambda d : self.find_element("preview").text.find("All tests completed") )      
        
if __name__ == '__main__':
    unittest.main()
