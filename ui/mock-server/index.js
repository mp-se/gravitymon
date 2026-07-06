/*
 * Project specific mock server
 *
 * (c) 2023-2024 Magnus Persson
 */
import { createRequire } from "module";
import { registerEspFwk } from './espfwk.js'
import { configData, statusData, formatData } from './data.js'

const require = createRequire(import.meta.url);
const express = require('express')
var cors = require('cors')

const app = express()
const port = 3000

app.use(cors({
  origin: true,
  credentials: true,
  methods: ['GET', 'POST', 'PUT', 'DELETE', 'OPTIONS'],
  allowedHeaders: ['Content-Type', 'Authorization', 'Accept', 'Origin', 'X-Requested-With']
}))
app.use(express.json({ limit: '10mb' }))
app.use(express.urlencoded({ extended: true }))

registerEspFwk(app)

var calibrateRunning = false

app.get('/api/calibrate', (req, res) => {
  console.log('GET: /api/calibrate')
  /* 
   * Description:    Initiate the gyro calibration process. 
   * Authentication: Required
   * Limitation:     - 
   * Note:           Use /api/calibrate/status to check for completion
   * Return:         200 OK, 401 Access Denied
   */
  configData.gyro_calibration_data.gx = 1
  setTimeout(() => { calibrateRunning = false }, 5000)
  calibrateRunning = true
  var data = {
    success: true,
    message: "Gyro calibration started..."
  }
  res.type('application/json')
  res.send(data)
})

app.get('/api/ping', (req, res) => {
  console.log('GET: /api/ping')
  /* 
   * Description:    Check for response from device. 
   * Authentication: Required
   * Limitation:     - 
   * Return:         200 OK
   */
  var data = {
    status: true,
  }
  res.type('application/json')
  res.send(data)
})

app.get('/api/calibrate/status', (req, res) => {
  console.log('GET: /api/calibrate/status')
  /* 
   * Description:    Return status of the current gyro calibration process. 
   * Authentication: Required
   * Limitation:     - 
   * Note:           -
   * Return:         200 OK, 401 Access Denied
   */
  var data = {}
  if (calibrateRunning) {
    data = {
      status: calibrateRunning,
      success: false,
      message: "Gyro calibration running..."
    }
  } else {
    data = {
      status: false,
      success: true,
      message: "Gyro calibration completed..."
    }
  }
  res.type('application/json')
  res.send(data)
})

var testRunning = false

app.post('/api/push', (req, res) => {
  console.log('GET: /api/push')
  /* 
   * Description:    Initiate the push test for a defined target
   * Authentication: Required
   * Limitation:     - 
   * Note:           Use /api/test/push/status to check for completion
   * Return:         200 OK, 401 Access Denied, 422 Content Invalid
   * Request body:
     {
       push_format: "http_format|http_format2|http_format3|influxdb2_format|mqtt_format"
     }
   */
  if(!Object.prototype.hasOwnProperty.call(req.body, "push_format")) {
    res.sendStatus(422)
    return
  }  
  testRunning = true
  setTimeout(() => { testRunning = false }, 5000)
  var data = {
    success: true,
    message: "Test scheduled."
  }
  res.type('application/json')
  res.send(data)
})

app.get('/api/push/status', (req, res) => {
  console.log('GET: /api/push/status')
  /* 
   * Description:    Return status of the current gyro calibration process. 
   * Authentication: Required
   * Limitation:     - 
   * Note:           -
   * Return:         200 OK, 401 Access Denied
   */
  var data = {}
  if (testRunning) {
    data = {
      status: testRunning,
      success: false,
      message: "Push test running..."
    }
  } else {
    data = {
      status: false,
      success: true,
      message: "Push test completed...",
      push_return_code: 200,
      push_enabled: true
    }
  }
  res.type('application/json')
  res.send(data)
})

app.get('/api/gyro', (req, res) => {
  console.log('GET: /api/gyro')
  /* 
   * Description:    Return status of the current gyro. 
   * Authentication: Required
   * Limitation:     - 
   * Note:           -
   * Return:         200 OK
   */
  var data = {
    angle: 45
  }

  res.type('application/json')
  res.send(data)
})

app.get('/api/formula', (req, res) => {
  console.log('GET: /api/formula')
  /* 
   * Description:    Simualate creation of gravity formula. Assume data has been saved via config.
   * Authentication: Required
   * Limitation:     - 
   * Note:           -
   * Return:         200 OK, 401 Access Denied
   */
  setTimeout(() => {
    var data = {
      success: true,
      message: "Hello world",
      gravity_formula: "0.0*tilt^3+0.0*tilt^2+0.0017978*tilt+0.9436"
    }
    res.type('application/json')
    res.send(data)
  }, 2000)
})

app.get('/api/format', (req, res) => {
  console.log('GET: /api/format')
  /* 
   * Description:    Return format data as json document. 
   * Authentication: Required
   * Limitation:     -
   * Note:           -
   * Return:         200 OK, 401 Access Denied
   */
  res.type('application/json')
  res.send(formatData)
})

app.post('/api/format', (req, res) => {
  console.log('POST: /api/format')
  /* 
   * Description:    Update the format data that is in body
   * Authentication: Required
   * Limitation:     - 
   * Note:           See format read for options.
   * Return:         200 OK, 401 Access Denied
   */
  console.log(req.body);
  for (var o in req.body) {
    formatData[o] = req.body[o]
  }
  var data = {
    success: true,
    message: "Format stored",
  }
  res.type('application/json')
  res.send(data)
})

app.post('/api/sleepmode', (req, res) => {
  console.log('POST: /api/sleepmode')
  /* 
   * Description:    Toggle the sleep mode (from index page)
   * Authentication: Required
   * Limitation:     - 
   * Note:           Returns current sleep_mode
   * Return:         200 OK, 401 Access Denied
   * Request body:
     {
       sleep_mode: true|false
     }
   */
  statusData.sleep_mode = req.body.sleep_mode
  var data = { 
    sleep_mode: req.body.sleep_mode 
  }
  res.type('application/json')
  res.send(data)
})

// Registration endpoints for testing
app.get('/api/v1/device/check/:chipid', (req, res) => {
  console.log('GET: /api/v1/device/check/:chipid', req.params)
  /* 
   * Description:    Check if device is already registered
   * Authentication: Bearer token required
   * Limitation:     - 
   * Note:           Used to determine if registration modal should be shown
   * Return:         200 OK with { exists: true|false, software?: string }
   * Path param:     chipid
   */
  const chipid = req.params.chipid
  // Simulate different scenarios based on chipid
  let response = { exists: false }
  
  if (chipid === 'b1a9e561106ea030330ac272e9f446130d42887aa4596fbf9a1dc4bd2144ed4a') { // anonymized 'registered' version
    response = { exists: true, software: 'Gravitymon' }
  } else {
    response = { exists: false }
  }
  
  console.log('GET: /api/device/check', response)
  res.type('application/json')
  res.send(response)
})

app.post('/api/v1/device/add', (req, res) => {
  console.log('POST: /api/v1/device/add', req.body)
  /* 
   * Description:    Register a new device
   * Authentication: Bearer token required
   * Limitation:     - 
   * Note:           Test different error scenarios using request body
   * Return:         200 OK, 401 Unauthorized, 429 Too Many Requests, 500 Internal Server Error
   * Request body:   { software, chipid, version, platform, board }
   */
  const body = req.body
  
  // Simulate different error scenarios based on chipid
  if (body.chipid === 'b0d6dd8a42dbadd0a1ada3b250c999cc9b1051c715e9e9ad4375f2779c077a6e') { // anonymized 'test_401' version
    console.log('Returning 401 error')
    res.status(401)
    return res.json({ error: 'API key revoked' })
  } else if (body.chipid === '20f9647ad4bebb51fe1758953a452af985c6352f52a494c3abf585d39996c7c6') { // anonymized 'test_429' version
    console.log('Returning 429 error')
    res.status(429)
    return res.json({ error: 'Too many requests' })
  } else if (body.chipid === '1eac306cebbc9ddf0654033e3fed489688f8c288236b344b5fa960469b970f8c') { // anonymized 'test_500' version
    console.log('Returning 500 error')
    res.status(500)
    return res.json({ error: 'Internal server error' })
  }
  
  // Success case
  const response = {
    success: true,
    message: 'Data stored successfully',
    id: 1
  }
  
  res.type('application/json')
  res.send(response)
})

app.listen(port, () => {
  console.log(`Gravitymon API simulator port http://localhost:${port}/`)
})