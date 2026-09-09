#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <DoubleResetDetector.h> // Handles detecting rapid hardware reset presses
#include <time.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Reset Detector Config ---
// Number of seconds to look out for a second button press
#define DRD_TIMEOUT 2.0
// RTC Memory Address block used to track the tracking flag
#define DRD_ADDRESS 0
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

// --- Timezone Configuration ---
const long timezoneOffsetInSeconds = 19800; 
const int daylightOffsetInSeconds = 0; 

// --- LCD Settings ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* daysOfWeek[]  = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char* monthsOfYear[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// --- Timers (Non-blocking) ---
unsigned long lastDisplayCycle = 0;
const unsigned long cycleInterval = 4000; 

unsigned long lastClockTime = 0;
const unsigned long clockInterval = 500;  

bool showDayToggle = true;

void setup() {
  Wire.begin(4, 5);
  lcd.init();
  lcd.backlight();

  WiFiManager wm;

  // Check if this boot was caused by a rapid double-reset
  if (drd.detectDoubleReset()) {
    lcd.setCursor(0, 0);
    lcd.print("Reset Triggered!");
    lcd.setCursor(0, 1);
    lcd.print("Clearing WiFi...");
    
    wm.resetSettings(); // Wipes saved credentials out of the internal flash memory
    delay(3000);
    lcd.clear();
  }

  lcd.setCursor(0, 0);
  lcd.print("Checking WiFi...");
  wm.setConfigPortalTimeout(180);

  // If credentials were wiped, this will automatically launch the setup portal
  if (!wm.autoConnect("ESP8266-Clock-Setup")) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Setup Timeout");
    lcd.setCursor(0, 1);
    lcd.print("Restarting...");
    delay(3000);
    ESP.restart();
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected!");
  lcd.setCursor(0, 1);
  lcd.print("Syncing Time...");

  configTime(timezoneOffsetInSeconds, daylightOffsetInSeconds, "pool.ntp.org", "time.nist.gov");

  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }
  lcd.clear();

  // Deactivate the tracker flag once setup finishes successfully so normal runs don't trigger it
  drd.stop();
}

void loop() {
  // Keep monitoring the detector status loop internally
  drd.loop();

  unsigned long currentMillis = millis();
  time_t now = time(nullptr);
  struct tm* ptm = localtime(&now); 

  // --- TASK 1: Update Clock (Row 0) ---
  if (currentMillis - lastClockTime >= clockInterval) {
    lastClockTime = currentMillis;
    
    char timeBuffer[17];
    strftime(timeBuffer, sizeof(timeBuffer), "Time: %H:%M:%S", ptm);
    
    lcd.setCursor(0, 0);
    lcd.print(timeBuffer); 
  }

  // --- TASK 2: Alternate Day and Date on Row 1 ---
  if (currentMillis - lastDisplayCycle >= cycleInterval) {
    lastDisplayCycle = currentMillis;
    
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    lcd.setCursor(0, 1);

    if (showDayToggle) {
      lcd.print(daysOfWeek[ptm->tm_wday]);
    } else {
      char dateBuffer[17];
      snprintf(dateBuffer, sizeof(dateBuffer), "%02d %s %d", ptm->tm_mday, monthsOfYear[ptm->tm_mon], ptm->tm_year + 1900);
      lcd.print(dateBuffer);
    }

    showDayToggle = !showDayToggle; 
  }
}
