<p align="center">
  <img src="Photos\ESP32_PCB_vis01.png" title="ESP32 Radio PCB vis1">
  <img src="Photos\ESP32_PCB_vis02.png" title="ESP32 Radio PCB vis2">
</p>

# ESP32 Web Radio Evo3

## Overview

This is project of Internet Radio Streamer called "Evo". Hardware was buid using EPS32-S3 and DAC codec PCM5102A. Construction allows to listen varius music station from all aroudn world.
working properly with streams coded in MP3, AAC, vorbis and flac (up to 1Mbit). Support all operations (volume control, stations changes, memory bank change) on single rotary 
encoder and aslo cooeprate with infreared remote controls working on NEC standars (38kHz).


- Internet radio stream station from bank files (can be stored on SD card or downloaded from GitHub)
- I2S simple PCM5102A decoder support
- Full web server integrated for controling from desktop computer or mobile phone
- OLED Display
- Single rotary encoder operations, 
- OTA updates directlly from web page
- SD card content review by web page
- VUmeters add as some visualization on OLED
- 16 Banks with 99 station per each Bank support
- Polish fonts add for coorrect display station strings
- 3-point Equalizer
- Resistance keyboard based on single ADC input
- 3 display modes with fullscreen clock option
- Configuration stored on SD card. Minimum function keep as a "hardcoded"


## Hardware

- ESP32-S3 dev. module with internal antena or external antena connector
- SD or micro SD card reader
- 256x64 OLED display based on SSD1322 or SH1122 driver IC
- Rotary encoder
- IR 38KHz receiver (Vishay TSOP2238)
- PCB design stored in repository
- Housing avalivle as ready to print STL files

 
## Software

Project was coded in Arduino platform but ready main.cpp for Platformio avalable and also tested

How to compile (Arduino):
- Install fresh Arduino (currently 2.3.4)
- After instalation add ESP32 board package by Espressif (this operation will take some time)
- Copy from GitHub library folder libraries: 
  ESP32-audioI2S, ezButton, U8g2, WiFiManager to local Arduino library space:
  typically:C:\Users\YOUR USER NAME\AppData\Local\Arduino15\libraries

For Platformio look into src folder -> Platformio folder and Platformio.ini file.


## Software Dependencies

[ESP Supressif libs]
- WiFi 3.0.7 
- Networking 3.0.7 
- NetworkClientSecure 3.0.7
- SD 3.0.7 
- FS 3.0.7 
- SPI 3.0.7
- SD_MMC 3.0.7
- SPIFFS w wersji 3.0.7
- HTTPClient w wersji 3.0.7
- FFat w wersji 3.0.7
- Update w wersji 3.0.7
- WebServer w wersji 3.0.7
- DNSServer w wersji 3.0.7
- ESP32 Async UDP w wersji 3.0.7
- EEPROM w wersji 3.0.7
- Wire 3.0.7
- Ticker 3.0.7

[3rd-party libs]
- ESP32-audioI2S-master 3.0.13
- U8g2 2.36.5
- ezButton 1.0.4
- WiFiManager w wersji 2.0.17
- ESP Async WebServer w wersji 3.6.0
- Async TCP 3.3.2 

## Usage
Radio can be build only with ESP32-S3 dev. module and PCM5102A. In this configartion last station, memory bank number and volume will be stored in EEPROM. Control is possible via web page or IR remote control RC-406.
For full experience connect SD card (look on schematic or source code), encoder, IR receiver.

NEC standard codes for remote controler are stored on SD card in "remote.txt" file. Example files you can find inside "SD card content" folder.

### Radio Station URLs
The radio streams URL addresses are defined in banks files, each bank is downloaded at first run and stored on SD card as separate txt file: Bank01.txt, Bank02.txt etc.
Each of banks can store 99 station. You can modify the station by changing the URLs in the code. Bank files are stored here:

STATIONS_URL1 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank01.txt"
STATIONS_URL2 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank02.txt"
STATIONS_URL3 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank03.txt"
...
STATIONS_URL16 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank16.txt"


## License

This project is open-source and licensed under the [MIT License](https://opensource.org/licenses/MIT). Feel free to contribute and improve the code.
