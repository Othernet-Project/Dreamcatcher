This firmware runs on the Othernet Dreamcatcher satellite data receiver. The receiver's primary component's include the ESP32/ESP32-S2 from Espressif Systems, Semtech SX1281 LoRa transceiver, Texas Instruments TPS-65235 13V regulator, and Microchip MIC5353 3.3V regulator.

This firmware has been tested on both the ESP32 and ESP32-S2 wifi microntrollers. Due to high memory usage only wrover modules/boards will work (with SPIRAM). The Dreamcatcher 4 should be available for purchase in April of 2021. 

This firmware is used to control the voltage to a standard, commercially-available satellite LNB. The LNB takes a 12 GHz signal and downconverts it to a range that can be received by the SX1281. The SX1281 demodulates the downconverted LoRa carrier and sends packets to the ESP32 over SPI. Downloaded packets are turned into files and stored on the microSD. 

The ESP32 can create a wifi access point and also connect to an existing wifi network. By defaul, Dreamcatcher operates in access point (AP) mode. Changes to wifi settings can be made through the web interface. Configuration of the SX1281 (frequency, spreading factor, coding rate) is also done through the web interface.

# Getting started

This application is prepared to build with arduino as component. Easiest way is to clone with `git clone --recursive`.
Because the arduino release only supports ep-idf up to 4.x version please don't use 5.x releases, they don't work yet.

Some settings can be changed in `customize.h`.

## Building on Ubuntu/Debian
To setup everything and build the Dreamcatcher Firmware you can follow the following Steps on Ubuntu/Debian (tested on Ubuntu 18.04 and 20.04).

### Install needed packages & Setup python 3 as default
```
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10 && alias pip=pip3
```
### Install esp-idf 4.4.5

https://docs.espressif.com/projects/esp-idf/en/v4.4.5/esp32/get-started/index.html

```
mkdir -p ~/esp
cd ~/esp
git clone -b v5.1 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
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

First export idf so you can use the idf.py commands: `. ~/esp/esp-idf/export.sh`.

You can build for ESP32s2 (DC4 Board) or ESP32 (if you have one for testing for example), just replace the content of sdkconfig with either sdkconfig-esp32 or sdkconfig-esp32s2.
The Default sdkonfig is equal to ESP32s2, if you setup the sdkconfig you want run `idf.py build`.

There is also a build.sh you can use to build for both ESPs at once, just export idf with `. ~/esp/esp-idf/export.sh` and then run `./build.sh`.

If that is successful you can flash your ESP Board, connect it to your PC and use `dmesg | grep tty` to find your serial device id.

Then use `sudo usermod -a -G dialout $USER` to set the Permissions as needed, you need to restart/relogin to make the changes take affect.

After this you can Flash your board with `idf.py -p /dev/ttyACM0 flash`.
