This firmware runs on the Othernet Dreamcatcher satellite data receiver. The receiver's primary component's include the ESP32/ESP32-S2 from Espressif Systems, Semtech SX1281 LoRa transceiver, Texas Instruments TPS-65235 13V regulator, and Microchip MIC5353 3.3V regulator.

This firmware has been tested on both the ESP32 and ESP32-S2 wifi microntrollers. Due to high memory usage only wrover modules/boards will work (with SPIRAM). The Dreamcatcher 4 should be available for purchase in April of 2021. 

This firmware is used to control the voltage to a standard, commercially-available satellite LNB. The LNB takes a 12 GHz signal and downconverts it to a range that can be received by the SX1281. The SX1281 demodulates the downconverted LoRa carrier and sends packets to the ESP32 over SPI. Downloaded packets are turned into files and stored on the microSD. 

The ESP32 can create a wifi access point and also connect to an existing wifi network. By defaul, Dreamcatcher operates in access point (AP) mode. Changes to wifi settings can be made through the web interface. Configuration of the SX1281 (frequency, spreading factor, coding rate) is also done through the web interface.

 

<h2>Getting started</h2>

This application is prepared to build with arduino as component. Easiest way is to clone with `git clone --recursive`.
Because most recent arduino release (v4.x) is in beta stage this application was prepared and tested with particular esp-idf commit. Curently it is https://github.com/espressif/esp-idf/commit/b0150615dff529662772a60dcb57d5b559f480e2

Some settings can be changed in `customize.h`.

<h2>Build options</h2>

When esp-idf environment is prepared there is 2 options to build binaries:
* as usual with idf.py build, for each chip model there is prepared sdkconfig file to use as template,
* with build.sh scipt (linux version only), which will build for esp32 and esp32-S2, both version at the same time.

<h2>Possible runtime issues</h2>

* there is change in esp-idf i2c and arduino-esp32 has not been updated yet, this requires to update `esp32-hal-i2c.c` line 1819 with code `i2c_config_t conf = {0};`
* some changes may be required in https://github.com/espressif/arduino-esp32/blob/idf-release/v4.2/CMakeLists.txt, in this repository is included file with fixed content:
https://github.com/chegewara/dreamcatcher4/blob/master/CMakeLists.txt_arduino-esp32
