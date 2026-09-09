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
* Install WiFiManager by tablatronix
