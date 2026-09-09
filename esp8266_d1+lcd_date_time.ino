#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <DoubleResetDetector.h> 
#include <time.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DRD_TIMEOUT 2.0
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
const unsigned long cycleInterval = 4000; // Switch screens every 4 seconds

unsigned long lastClockTime = 0;
const unsigned long clockInterval = 500;  

// --- Display Screen State Tracker ---
// 0 = Day Name, 1 = Date Stamp, 2 = WiFi Strength
int displayScreenIdx = 0; 

// Helper function to convert raw dBm to a text string
const char* getSignalQuality(long rssi) {
  if (rssi >= -50) return "Excellent";
  if (rssi >= -65) return "Good     ";
  if (rssi >= -75) return "Fair     ";
  return "Poor     ";
}

void setup() {
  Wire.begin(4, 5);
  lcd.init();
  lcd.backlight();

  WiFiManager wm;

  if (drd.detectDoubleReset()) {
    lcd.setCursor(0, 0);
    lcd.print("Reset Triggered!");
    lcd.setCursor(0, 1);
    lcd.print("Clearing WiFi...");
    wm.resetSettings(); 
    delay(3000);
    lcd.clear();
  }

  lcd.setCursor(0, 0);
  lcd.print("Checking WiFi...");
  wm.setConfigPortalTimeout(180);

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
  drd.stop();
}

void loop() {
  drd.loop();

  unsigned long currentMillis = millis();
  time_t now = time(nullptr);
  struct tm* ptm = localtime(&now); 

  // --- TASK 1: Update Clock (Row 0) ---
  if (currentMillis - lastClockTime >= clockInterval) {
    lastClockTime = currentMillis;
    
    char timeBuffer[16];
    strftime(timeBuffer, sizeof(timeBuffer), "Time: %H:%M:%S", ptm);
    
    lcd.setCursor(0, 0);
    lcd.print(timeBuffer); 
  }

  // --- TASK 2: Cycle Through Day, Date, and WiFi (Row 1) ---
  if (currentMillis - lastDisplayCycle >= cycleInterval) {
    lastDisplayCycle = currentMillis;
    
    // Clear row 1 cleanly before adding new text
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    lcd.setCursor(0, 1);

    switch (displayScreenIdx) {
      case 0:
        // Screen 1: Day Name (e.g., "Wednesday")
        lcd.print(daysOfWeek[ptm->tm_wday]);
        break;

      case 1: {
        // Screen 2: Date (e.g., "09 Sep 2026")
        char dateBuffer[16];
        snprintf(dateBuffer, sizeof(dateBuffer), "%02d %s %d", ptm->tm_mday, monthsOfYear[ptm->tm_mon], ptm->tm_year + 1900);
        lcd.print(dateBuffer);
        break;
      }

      case 2: {
        // Screen 3: WiFi Strength (e.g., "WiFi: Good -61dB")
        long rssi = WiFi.RSSI();
        char wifiBuffer[16];
        // Prints both readable rating and exact dBm value to fit 16 characters
        snprintf(wifiBuffer, sizeof(wifiBuffer), "WF:%s %ddBm", getSignalQuality(rssi), rssi);
        lcd.print(wifiBuffer);
        break;
      }
    }

    // Step to the next screen index loop (0 -> 1 -> 2 -> 0)
    displayScreenIdx = (displayScreenIdx + 1) % 3; 
  }
}
