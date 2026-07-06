/*
 * Common mock server functions for espframework
 *
 * (c) 2023-2024 Magnus Persson
 */

import { createRequire } from 'module'
import { Buffer } from 'buffer'
import { configData, featureData, statusData } from './data.js'
const require = createRequire(import.meta.url)
const multer = require('multer')
const path = require('path')
const upload = multer({ dest: './' })

var wifiScanRunning = false

export function registerEspFwk(app) {
  // Helper function to validate Bearer token
  function validateBearerToken(req) {
    const authHeader = req.headers['authorization'] || req.headers['authorization']
    if (authHeader && authHeader.startsWith('Bearer ')) {
      const token = authHeader.substring(7)
      console.log('Bearer token for validation:', token)
      return token === statusData.id
    }
    console.log('No or invalid Bearer token', authHeader)
    return false
  }

  app.get('/', function (req, res) {
    console.log('GET: /')
    const options = {
      root: path.join('.')
    }
    res.type('html')
    res.sendFile('mock-server/index.html', options)
  })

  app.get('/index.js', function (req, res) {
    console.log('GET: /index.js')
    const options = {
      root: path.join('.')
    }
    res.type('js')
    res.sendFile('dist/assets/index.js', options)
  })

  app.get('/style.css', function (req, res) {
    console.log('GET: /style.css')
    const options = {
      root: path.join('.')
    }
    res.type('css')
    res.sendFile('dist/assets/style.css', options)
  })

  app.get('/api/ping', function (req, res) {
    console.log('GET: /api/ping')
    res.send({ status: true })
  })

  app.post('/api/firmware', upload.single('file'), function (req, res) {
    const title = req.body.title
    const file = req.file

    console.log(title)
    console.log(file)

    res.sendStatus(200)
  })

  app.post('/api/filesystem/upload', upload.single('file'), function (req, res) {
    const title = req.body.title
    const file = req.file

    console.log(title)
    console.log(file)

    res.sendStatus(200)
  })

  app.get('/api/auth', (req, res) => {
    console.log('GET: /api/auth')
    /*
     * Description:    Perform device authentication and receive access token
     * Authentication: Yes, user and password
     * Limitation:     -
     * Return:         200 OK, 401 Access Denied
     * Request body:   None
     */
    var data = { token: statusData.id }

    console.log(req.headers['authorization'])

    // Decode and print authentication header (Basic auth only)
    const authHeader = req.headers['authorization'] || req.headers['Authorization']
    if (authHeader) {
      if (authHeader.startsWith('Basic ')) {
        const base64 = authHeader.substring(6)
        try {
          const decoded = Buffer.from(base64, 'base64').toString()
          console.log('Basic auth decoded:', decoded)
        } catch (e) {
          console.log('Failed to decode Basic auth:', e.message)
        }
      } else {
        console.log('Unsupported auth scheme:', authHeader.split(' ')[0])
      }
    } else {
      console.log('No Authorization header')
    }

    res.send(data)
  })

  app.get('/api/config', (req, res) => {
    console.log('GET: /api/config')
    /*
     * Description:    Return configuration data as json document.
     * Authentication: Required
     * Limitation:     Don't include format templates due to their size.
     *                 Wifi passwords are removed due to security considerations (no encrypted transfer).
     * Note:           -
     * Return:         200 OK, 401 Access Denied
     */

    // Verify Bearer token
    if (!validateBearerToken(req)) {
      res.status(401).send('Access Denied')
      return
    }

    res.type('application/json')
    res.send(configData)
  })

  app.get('/api/restart', (req, res) => {
    console.log('GET: /api/restart')
    /*
     * Description:    Perform a restart of the device
     * Authentication: Required
     * Limitation:     -
     * Note:           Simulator will wait 2 seconds before returning result.
     * Return:         200 OK, 401 Access Denied
     */
    setTimeout(() => {
      var data = {
        status: true,
        success: true,
        message: 'Device is restarting...'
      }
      res.type('application/json')
      res.send(data)
    }, 2000)
  })

  app.get('/api/factory', (req, res) => {
    console.log('GET: /api/factory')
    /*
     * Description:    Simualate a factory reset.
     * Authentication: Required
     * Limitation:     -
     * Note:           -
     * Return:         200 OK, 401 Access Denied
     */
    setTimeout(() => {
      var data = {
        success: true,
        message: 'Factory settings restored'
      }
      res.type('application/json')
      res.send(data)
    }, 2000)
  })

  app.get('/api/status', (req, res) => {
    console.log('GET: /api/status')
    /*
     * Description:    Return status data as json document.
     * Authentication: None
     * Limitation:     -
     * Note:           -
     * Return:         200 OK
     */
    res.type('application/json')
    res.send(statusData)
  })

  app.get('/api/feature', (req, res) => {
    console.log('GET: /api/feature')
    /*
     * Description:    Return feature data as json document.
     * Authentication: None
     * Limitation:     -
     * Note:           -
     * Return:         200 OK
     */
    res.type('application/json')
    res.send(featureData)
  })

  app.post('/api/config', (req, res) => {
    console.log('POST: /api/config')
    /*
     * Description:    Update the configuration data that is in body
     * Authentication: Required
     * Limitation:     -
     * Note:           See config read for options.
     * Return:         200 OK, 401 Access Denied
     */
    console.log(req.body)
    for (var o in req.body) {
      configData[o] = req.body[o]
    }
    var data = {
      success: true,
      message: 'Configuration stored'
    }
    res.type('application/json')
    res.send(data)
  })

  app.get('/api/wifi', (req, res) => {
    console.log('GET: /api/wifi')
    /*
     * Description:    Do a wifi scan for avaialble networks
     * Authentication: Required
     * Limitation:     -
     * Note:           Use /api/wifi/scan/status to check for completion
     * Return:         200 OK, 401 Access Denied
     */
    wifiScanRunning = true
    setTimeout(() => {
      wifiScanRunning = false
    }, 8000)
    var data = {
      success: true,
      message: 'Wifi scan started.'
    }
    res.type('application/json')
    res.send(data)
  })

  app.get('/api/wifi/status', (req, res) => {
    console.log('GET: /api/wifi/status')
    /*
     * Description:    Return status of the current wifi scan process.
     * Authentication: Required
     * Limitation:     -
     * Note:           -
     * Return:         200 OK, 401 Access Denied
     */
    var data = {}
    if (wifiScanRunning) {
      data = {
        status: wifiScanRunning,
        success: false,
        message: 'Wifi scan running...'
      }
    } else {
      data = {
        status: false,
        success: true,
        message: 'Wifi scan completed...',
        networks: [
          { wifi_ssid: 'network A', rssi: -20, channel: 10, encryption: 2 },
          { wifi_ssid: 'network B', rssi: -70, channel: 12, encryption: 3 },
          { wifi_ssid: 'network C', rssi: -50, channel: 6, encryption: 0 }
        ]
      }
    }
    res.type('application/json')
    res.send(data)
  })

  app.post('/api/filesystem', (req, res) => {
    console.log('POST: /api/filesystem')
    /* 
     * Description:    Interact with local file system on the device
     * Authentication: Required
     * Limitation:     - 
     * Return:         200 OK, 401 Access Denied, 400 Faulty request
     * Request body:
       {
         command: "dir|get|del",
         file: "name of file for get and del"
       }
     */
    console.log('Command:', req.body.command)

    if (req.body.command == 'dir') {
      var data = {
        total: 1000,
        used: 900,
        free: 100,
        files: [
          { file: '/error.log', size: 10 },
          { file: '/error2.log', size: 10 },
          { file: '/config.json', size: 10 },
          { file: '/gravitymon.json', size: 10 },
          { file: '/data.csv', size: 10 },
          { file: '/mqtt.tpl', size: 10 },
          { file: '/formdata.tpl', size: 10 },
          { file: '/bin.dat', size: 10 }
        ]
      }
      res.type('application/json')
      res.send(data)
      return
    } else if (req.body.command == 'del') {
      console.log(req.body.file)
      setTimeout(() => {
        res.sendStatus(200)
      }, 2000)
      return
    } else if (req.body.command == 'get') {
      console.log(req.body.file)
      if (req.body.file == '/error.log') {
        setTimeout(() => {
          res.send('Log entry 5\nLog entry 4\nLog entry 3\nLog entry 2\nLog entry 1\n')
        }, 200)
      } else if (req.body.file == '/error2.log') {
        setTimeout(() => {
          res.send('Log entry 9\nLog entry 8\nLog entry 7\nLog entry 6\n')
        }, 500)
      } else if (req.body.file == '/config.json') {
        var ispindel = {
          Name: '',
          Token: '',
          Sleep: 900,
          Server: '',
          API: 0,
          Port: 80,
          Channel: 0,
          URI: '',
          Username: '',
          Password: '',
          Job: 'ispindel',
          Instance: '000',
          Hassio: false,
          UseHTTPS: false,
          Vfact: 191.8000031,
          TS: 0,
          OWpin: 12,
          POLY: '-0.00031*tilt^2+0.557*tilt-14.054',
          SSID: 'my-wifi',
          PSK: 'wifi-pass',
          Offset: [614, 628, 740, 96, -60, 3]
        }
        res.send(ispindel)
      } else if (req.body.file == '/gravitymon.json') {
        var gravmon = {
          mdns: 'gravitymone18798',
          id: 'e18798',
          'ota-url': 'http://ota',
          'wifi-ssid': '@home',
          'wifi-pass': '',
          'wifi-ssid2': '',
          'wifi-pass2': '',
          ble: 'pink',
          'temp-format': 'C',
          token: 'token',
          token2: 'token2',
          'http-push': 'http://push1',
          'http-push-h1': 'Content-Type: application/json1',
          'http-push-h2': 'c: d',
          'http-push2': 'http://push2',
          'http-push2-h1': 'Content-Type: application/json2',
          'http-push2-h2': 'a: b',
          'http-push3': 'http://push3',
          'influxdb2-push': 'push4',
          'influxdb2-org': 'org',
          'influxdb2-bucket': 'bucket',
          'influxdb2-auth': 'auth',
          'mqtt-push': 'push5',
          'mqtt-port': 1883,
          'mqtt-user': 'user',
          'mqtt-pass': 'pass',
          'sleep-interval': 900,
          'voltage-factor': 0.59,
          'voltage-config': 4.15,
          'gravity-formula': 'tilt*2+til*3+1.0334',
          'gravity-format': 'G',
          'temp-adjustment-value': 0,
          'gravity-temp-adjustment': false,
          'gyro-temp': false,
          'storage-sleep': false,
          'gravitymon-ble': false,
          'gyro-calibration-data': {
            ax: 0,
            ay: 1,
            az: 2,
            gx: 3,
            gy: 4,
            gz: 5
          },
          'formula-calculation-data': {
            a1: 1,
            a2: 2,
            a3: 3,
            a4: 4,
            a5: 5,
            a6: 6,
            a7: 7,
            a8: 8,
            a9: 9,
            a10: 10,
            g1: 1.01,
            g2: 1.02,
            g3: 1.03,
            g4: 1.04,
            g5: 1.05,
            g6: 1.06,
            g7: 1.07,
            g8: 1.08,
            g9: 1.09,
            g10: 1.1
          },
          'app-ver': '1.4.0',
          'app-build': 'beta-2',
          angle: 89.943,
          gravity: 0,
          battery: 1.95,
          'runtime-average': 0,
          platform: 'esp32s3'
        }
        res.send(gravmon)
      } else if (req.body.file == '/data.csv') {
        var csv = 'data,data2,data3,data4,data5,data6,data7,data8,data9,data0\ndata,data2,data3,data4,data5,data6,data7,data8,data9,data0\ndata,data2,data3,data4,data5,data6,data7,data8,data9,data0\ndata,data2,data3,data4,data5,data6,data7,data8,data9,data0\n'
        res.send(csv)
      } else if (req.body.file == '/mqtt.tpl') {
        var mqtt = 'ispindel/${mdns}/tilt:${angle}|ispindel/${mdns}/temperature:${temp}|ispindel/${mdns}/temp_units:${temp-unit}|ispindel/${mdns}/battery:${battery}|ispindel/${mdns}/gravity:${gravity}|ispindel/${mdns}/interval:${sleep-interval}|ispindel/${mdns}/RSSI:${rssi}|'
        res.send(mqtt)
      } else if (req.body.file == '/formdata.tpl') {
        var formdata = '?value=data&value2=data2&value3=data3&value4=data4'
        res.send(formdata)
      } else if (req.body.file == '/bin.dat') {
        var bin = Buffer.from([0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x01, 0x02, 0x03, 0xFF,0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x01, 0x02, 0x03, 0xFF,0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x01, 0x02, 0x03, 0xFF,0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x01, 0x02, 0x03, 0xFF,0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x01, 0x02, 0x03, 0xFF])
        res.send(bin)
      }
      return
    }

    res.sendStatus(400)
  })
}
