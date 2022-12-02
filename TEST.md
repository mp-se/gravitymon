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

Ive added a specific build that uses the AUnit testing framework so that we can test functions or classed on the device itself. 

Select the build target and build/upload the code to an iSpindle device. Check the output from the serial console.
