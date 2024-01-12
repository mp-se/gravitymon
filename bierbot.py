import requests

host = "https://brewbricks.com/api/iot/v1"
apikey = "<your key>"

headers = {
    "Content-Type": "application/json",
    "User-Agent": "iSpindel",
}

data = {
    "apikey": apikey,
    "type":"ispindel",
    "brand":"wemos_d1_mini",
    "version":"1.4.0",
    "chipid":"AABBCC",
    "s_number_wort_0":2.345,
    "s_number_temp_0":34.56,
    "s_number_voltage_0":4.34,
    "s_number_wifi_0":-54,
    "s_number_tilt_0":33.456
}

ret = requests.post( url=host, json=data, headers=headers )
print(ret.status_code, ret.text)
