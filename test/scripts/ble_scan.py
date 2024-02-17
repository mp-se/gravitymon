import asyncio, logging, json
from uuid import UUID

from construct import Array, Byte, Const, Int8sl, Int16ub, Struct, Int32ub
from construct.core import ConstError

from bleak import BleakScanner, BleakClient
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

logger = logging.getLogger(__name__)

ibeacon_format = Struct(
    "type_length" / Const(b"\x02\x15"),
    "uuid" / Array(16, Byte),
    "major" / Int16ub,
    "minor" / Int16ub,
    "power" / Int8sl,
)

eddystone_format = Struct(
    "type_length" / Const(b"\x20\x00"),
    "battery" / Int16ub,
    "temp" / Int16ub,
    "gravity" / Int16ub,
    "angle" / Int16ub,
    "chipid" / Int32ub,
)

class tilt:
    def __init__(self, color, uuid):
        self.color = color
        self.uuid = uuid

tilts = []

def init():
    tilts.append(tilt("red", UUID("A495BB10-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("green", UUID("A495BB20-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("black", UUID("A495BB30-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("purple", UUID("A495BB40-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("orange", UUID("A495BB50-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("blue", UUID("A495BB60-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("yellow", UUID("A495BB70-C5B1-4B44-B512-1370F02D74DE")))
    tilts.append(tilt("pink", UUID("A495BB80-C5B1-4B44-B512-1370F02D74DE")))

def contains(list, filter):
    for x in list:
        if filter(x):
            return True
    return False

def first(iterable, default=None):
  for item in iterable:
    return item
  return default

async def parse_gravitymon(device: BLEDevice):
    try:      
        async with BleakClient(device) as client:
            for service in client.services:
                if service.uuid.startswith("0000180a-"):
                    for char in service.characteristics:
                        if "read" in char.properties and char.uuid.startswith("00002ac4-"):
                            try:
                                value = await client.read_gatt_char(char.uuid)
                                data = json.loads( value.decode() )
                                logger.info( "Data received: %s", json.dumps(data) )
                            except Exception as e:
                                logger.error( "Failed to read data, Error: %s", e)          
            await client.disconnect()           
    except Exception as e:
        logger.error( "Failed to connect, Error: %s", e)                
        pass

def parse_tilt(device: BLEDevice, advertisement_data: AdvertisementData):
    try:      
        apple_data = advertisement_data.manufacturer_data[0x004C]
        ibeacon = ibeacon_format.parse(apple_data)
        uuid = UUID(bytes=bytes(ibeacon.uuid))
        tilt = first(x for x in tilts if x.uuid == uuid)
        if tilt is not None:
            tempF = ibeacon.major
            gravitySG = ibeacon.minor/1000

            if ibeacon.minor > 5000: # pro detected
                tempF = ibeacon.major/10
                gravitySG = ibeacon.minor/10000

            data = {
                "color": tilt.color,
                "gravity": gravitySG,
                "temperature": tempF,
                "RSSI": advertisement_data.rssi,
            }
            logger.info( "Data received: %s %s", json.dumps(data), device.address )
    except KeyError as e:
        pass
    except ConstError as e:
        pass

def parse_eddystone(device: BLEDevice, advertisement_data: AdvertisementData):
   
    try:      
        uuid = advertisement_data.service_uuids[0]
        data = advertisement_data.service_data.get(uuid)
        eddy = eddystone_format.parse(data)

        data = {
            "battery": eddy.battery/1000,
            "gravity": eddy.gravity/10000,
            "temperature": eddy.temp/1000, 
            "angle": eddy.angle/100,
            "chipid": hex(eddy.chipid)[2:],
        }
        logger.info( "Data received: %s %s", json.dumps(data), device.address )
    except ConstError as e:
        pass

async def main():
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)-15s %(name)-8s %(levelname)s: %(message)s",
    )

    init()
    #scanner = BleakScanner(scanning_mode="passive")
    scanner = BleakScanner(scanning_mode="active")

    while True:
        logger.info("Scanning for tilt/gravitymon BLE devices...")
        results = await scanner.discover(timeout=3,return_adv=True)     
        for d, a  in results.values():
            if d.name == "gravitymon" and any("0000feaa-" in s for s in a.service_uuids):
                parse_eddystone(d,a)
            elif d.name == "gravitymon":
                await parse_gravitymon(d)
            else:
                parse_tilt(d,a)

asyncio.run(main())