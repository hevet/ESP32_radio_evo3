# ESP32 Web Radio Evo3 (Evolution 3)

<p align="center">
  <img src="Photos\WebRadio1.png" title="ESP32 Radio">
  <img src="Photos\RadioAsm1.png" title="ESP32 Radio">
  <img src="Photos\RadioAsm2.png" title="ESP32 Radio">        
  <img src="Photos\ESP_radio_rev1.0_PCBvis1.png" title="ESP32 Radio">
  <img src="Photos\ESP32_PCB_vis02.png" title="ESP32 Radio">
  <img src="Photos\DisplayMode0.png" title="ESP32 Radio" width="420" height="160">
  <img src="Photos\DisplayMode1.png" title="ESP32 Radio" width="420" height="160">
  <img src="Photos\DisplayMode2.png" title="ESP32 Radio" width="420" height="160">  
  <img src="Photos\DisplayMode3.png" title="ESP32 Radio" width="420" height="160">
  <img src="Photos\DisplayMode4.png" title="ESP32 Radio" width="420" height="160">
</p>


## Overview
This is project of Internet Radio Streamer called "Evo". Hardware was buid using EPS32-S3 and DAC codec PCM5102A. Construction allows to listen varius music station from all aroudn world.
working properly with streams coded in MP3, AAC, VORBIS and FLAC (up to 1.5Mbit). Support all operations (volume control, stations changes, memory bank change, power on/off) on single rotary 
encoder and aslo cooeprate with infrared remote controls working on NEC standars (38kHz).

Project is NOT based on yoRadio.

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
- Power on/off from remote controller
- Sleep timer

## Display Modes


## Hardware
- ESP32-S3 dev. module with internal antena or external antena connector
- SD or micro SD card reader
- 256x64 OLED display based on SSD1322 or SH1122 driver IC
- 1x Rotary encoder
- IR 38KHz receiver (Vishay TSOP2238)
- PCB design stored in repository
- Housing avalivle as ready to print STL files

 
## Software
Project was coded in Arduino platform but ready main.cpp for Platformio avalable and also tested

How to compile (Arduino):
- Install fresh Arduino (currently 2.3.6)
- After instalation add ESP32 board package by Espressif (this operation will take some time)
- Install bellow libs inside Arduino (library manager): 
  ESP32-audioI2S, 
  ezButton, 
  U8g2, 
  WiFiManager
  
For Platformio look into src folder -> Platformio folder and Platformio.ini file.


## Software Dependencies
[ESP Espressif 3.2.0 libs]
- WiFi 3.2.0 
- Networking 3.2.0 
- NetworkClientSecure 3.2.0 
- SD 3.2.0 
- FS 3.2.0 
- SPI 3.2.0 
- SD_MMC 3.2.0 
- SPIFFS w wersji 3.2.0 
- HTTPClient w wersji 3.2.0 
- FFat w wersji 3.2.0 
- Update w wersji 3.2.0 
- WebServer w wersji 3.2.0 
- DNSServer w wersji 3.2.0 
- ESP32 Async UDP w wersji 3.2.0 
- EEPROM w wersji 3.2.0 
- Wire 3.2.0 
- Ticker 3.2.0 

[3rd-party libs]
- ESP32-audioI2S-master 3.4.3
- U8g2 2.36.5
- ezButton 1.0.4
- WiFiManager w wersji 2.0.17
- ESP Async WebServer w wersji 3.7.2
- Async TCP 3.3.6 

## Web server
Radio has buit in small web server with simple webpage for list stations, bank select, volume control and access to settings menu
<p><img src="Photos\RadioWeb1.png" title="ESP32 Radio" width="750" height="580"></p>

## Remote Control
There is pre-coded Remote Controler RC-406 (Kenwood clone). 
You can change it by editing remote.txt file on SD card.
Key mapping:
<p><img src="Photos\RemoteControlMap_RC406_rev1.10.png" title="ESP32 Radio" width="500" height="300"></p>


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
This project is open-source and licensed under the [MIT License](https://opensource.org/licenses/MIT). 
Feel free to contribute and improve the code.

MIT License
Copyright (c) 2025 [Grzegorz / Dzikakuna / Robgold ]

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


## Others
- This project was inspired by https://github.com/sarunia/ESP32_radio_player_v2
- More informations you can read here: https://www.elektroda.pl/rtvforum/topic4041603.html