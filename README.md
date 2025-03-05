<p align="center">
  <img src="Photos\ESP32_PCB_vis01.png" title="ESP32 Radio PCB vis1">
  <img src="Photos\ESP32_PCB_vis02.png" title="ESP32 Radio PCB vis2">
</p>


# ESP32 Web Radio Evo3
current code rev 3.15

- single rotary encoder operations, 
- this compilation is focuesd on radio functions,
- player code still exist, adding 2nd encoder will give chance to operate with player
- no weather option,
- scrolling fucntion fo station string on display
- VUmeters add as some visualization on OLED
- 16 Banks with 99 station per each Bank
- Polish fonts add for coorrect display station strings
- IR receiver for remote control - NEC protocol supported
- 3-point equalizer
- Resistance keyboard based on single ADC input
- WiFi signal level on display
- additional fullscreen clock mode



# Hardware

- ESP32-S3 module
- SD or micro SD card reader
- 256x64 OLED display SSD1322 or SH1122
- IR 38KHz receiver
- rotary encoder (minimum 1)

 
# Software

How to compile:
- Install fresh Arduino (currently 2.3.4)
- After instalation add ESP32 board package by Espressif (this operation will take some time)
- Copy from GitHub library folder libraries: 
  ESP32-audioI2S, ezButton, U8g2, WiFiManager to local Arduino library space:
  typically:C:\Users\YOUR USER NAME\AppData\Local\Arduino15\libraries

