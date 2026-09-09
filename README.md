# esp8266_d1_lcd_date_time
Date and Time display on LCD with esp8266 D1 mini

## Wiring Connections
* GND: Connect to the GND pin on the D1 Mini.
* VCC: Connect to the 5V or VIN pin on the D1 Mini.
* SDA: Connect to D2 (GPIO4) on the D1 Mini.
* SCL: Connect to D1 (GPIO5) on the D1 Mini.

## Software Setup
* Open Arduino IDE and Go to Sketch > Include Library > Manage Libraries.
* Install LiquidCrystal_I2C by Frank de Brabander.
* Install NTPClient by Fabrice Weinberg
* Install WiFiManager by tablatronix.
* Install DoubleResetDetector by Stephen Dent

## Automated hotspot setup wizard
How to use it on your phone:
1. Upload this code. Because there are no credentials saved in the flash memory yet, the LCD will immediately read Checking WiFi... before launching the portal.
2. Open your phone's Wi-Fi menu and connect to the network named ESP8266-Clock-Setup.
3. A login page should instantly pop up (Captive Portal). If it does not, open your mobile browser and type in 192.168.4.1.
4. Tap Configure WiFi, select your home network from the scanned list, type your Wi-Fi password, and hit Save.
5. The D1 Mini will save these details permanently into its internal EEPROM configuration block, shut down the temporary hotspot, and connect directly to your home router. Next time you turn it on, it will automatically connect without launching the setup portal.

## How to use the reset trigger:

1. Turn on your clock normally. It will display the time using your current network credentials.
2. To clear the memory, press the physical black RST button on the side of your D1 Mini board once, wait a fraction of a second, and immediately press it a second time.
3. The display will read Reset Triggered! Clearing WiFi..., drop its old configurations, and spin back up into its open ESP8266-Clock-Setup access point mode so you can configure it from your phone again.


