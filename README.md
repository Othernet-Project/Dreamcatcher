This firmware runs on the Othernet Dreamcatcher satellite data receiver. The receiver's primary component's include the ESP32/ESP32-S2 from Espressif Systems, Semtech SX1281 LoRa transceiver, Texas Instruments TPS-65235 13V regulator, and Microchip MIC5353 3.3V regulator.

This firmware has been tested on both the ESP32 and ESP32-S2 wifi microntrollers. Due to high memory usage only wrover modules/boards will work (with SPIRAM). The Dreamcatcher 4 should be available for purchase in April of 2021. 

This firmware is used to control the voltage to a standard, commercially-available satellite LNB. The LNB takes a 12 GHz signal and downconverts it to a range that can be received by the SX1281. The SX1281 demodulates the downconverted LoRa carrier and sends packets to the ESP32 over SPI. Downloaded packets are turned into files and stored on the microSD. 

The ESP32 can create a wifi access point and also connect to an existing wifi network. By defaul, Dreamcatcher operates in access point (AP) mode. Changes to wifi settings can be made through the web interface. Configuration of the SX1281 (frequency, spreading factor, coding rate) is also done through the web interface.

# Getting started

This application is prepared to build with arduino as component. Easiest way is to clone with `git clone --recursive`.
Because most recent arduino release (v4.x) is in beta stage this application was prepared and tested with particular esp-idf commit. Curently it is https://github.com/espressif/esp-idf/commit/b0150615dff529662772a60dcb57d5b559f480e2

Some settings can be changed in `customize.h`.

## Building on Ubuntu/Debian
To setup everything and build the Dreamcatcher 4 Firmware you can follow the following Steps on Ubuntu/Debian (tested on Ubuntu 18.04).

### Install needed packages & Setup python 3 as default
```
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10 && alias pip=pip3
```
### Install esp-idf at Commit b0150615dff529662772a60dcb57d5b559f480e2
```
mkdir -p ~/esp
cd ~/esp
git clone https://github.com/espressif/esp-idf.git
cd esp-idf,
git checkout b0150615dff529662772a60dcb57d5b559f480e2
git submodule update --init --recursive
./install.sh
```
### Prepare Dreamcatcher Firmware

go to a folder you like and do the following commands
```
git clone --recursive https://github.com/Othernet-Project/Dreamcatcher.git
cd Dreamcatcher
cp CMakeLists.txt_arduino-esp32 components/arduino/CMakeLists.txt
```

### Build Firmware for a Target & flash

You can build for ESP32s2 (DC4 Board) or ESP32 (if you have one for testing for example), just replace the content of sdkconfig with either sdkconfig-esp32 or sdkconfig-esp32s2.
Default sdkconfig is for esp32s2, to build run the following:
```
.~/esp/esp-idf/export.sh
idf.py build
```
If that is successful you can flash your ESP Board, connect it to your PC and use `dmesg | grep tty` to find your serial device id.

Then use `sudo chown username /dev/ttyUSB0` to set the Permissions as needed (repalce username with your user and ttyUSB0 with your serial ID).

After this you can Flash your board with `idf.py -p /dev/ttyUSB0 flash`.

## Build options

When esp-idf environment is prepared there is 2 options to build binaries:
* as usual with idf.py build, for each chip model there is prepared sdkconfig file to use as template,
* with build.sh scipt (linux version only), which will build for esp32 and esp32-S2, both version at the same time.

## Possible runtime issues

* there is change in esp-idf i2c and arduino-esp32 has not been updated yet, this requires to update `esp32-hal-i2c.c` line 1819 with code `i2c_config_t conf = {0};`
* some changes may be required in https://github.com/espressif/arduino-esp32/blob/idf-release/v4.2/CMakeLists.txt, in this repository is included file with fixed content:
https://github.com/chegewara/dreamcatcher4/blob/master/CMakeLists.txt_arduino-esp32
