# Unit testing - Python Script

I have moved my test scripts into this project now. They are mainly based on python scrips and validate the features from the API's.

Create a virtual environment and install the needed dependecies
```
python -m venv .env
pip install -r requirements.txt
```

Before you runt the script you need to update the IP adress to match the device that you have on your network.

Running the ALL tests
```
cd src/test
python3 -m unittest -v apitests.py -v
```

Running the ONE test
```
cd src/test
python3 -m unittest -v apitests.API.test_status -v
```

# Unit testing - Specific build

I've added a specific build that uses the AUnit (https://github.com/bxparks/AUnit) testing framework so that we can test functions or classes on the device itself. I hope this will simplify the release and testing cycle.

1. Select the build target (gravity-unit)
2. Build/upload the code to an iSpindle device. 
3. Check the output from the serial console.
