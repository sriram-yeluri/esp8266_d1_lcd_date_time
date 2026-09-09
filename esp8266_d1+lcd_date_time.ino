#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // Library handles connection and portal logic
#include <time.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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

  // Initialize WiFiManager
  WiFiManager wm;

  lcd.setCursor(0, 0);
  lcd.print("Checking WiFi...");

  // Optional: Automatically close configuration portal if user does nothing for 3 minutes
  wm.setConfigPortalTimeout(180);

  // This creates an open hotspot named "ESP8266-Clock-Setup" if it can't connect to saved networks
  // The code stays stuck here until your phone configures it
  if (!wm.autoConnect("ESP8266-Clock-Setup")) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Setup Timeout");
    lcd.setCursor(0, 1);
    lcd.print("Restarting...");
    delay(3000);
    ESP.restart();
  }

  // If you reach here, you are connected to Wi-Fi!
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected!");
  lcd.setCursor(0, 1);
  lcd.print("Syncing Time...");

  // Initialize native ESP8266 NTP client config
  configTime(timezoneOffsetInSeconds, daylightOffsetInSeconds, "pool.ntp.org", "time.nist.gov");

  // Wait until time is successfully fetched from the internet
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }
  lcd.clear();
}

void loop() {
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

  // --- TASK 2: Alternate Day and Date on Row 1 ---
  if (currentMillis - lastDisplayCycle >= cycleInterval) {
    lastDisplayCycle = currentMillis;
    
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    lcd.setCursor(0, 1);

    if (showDayToggle) {
      lcd.print(daysOfWeek[ptm->tm_wday]);
    } else {
      char dateBuffer[16];
      snprintf(dateBuffer, sizeof(dateBuffer), "%02d %s %d", ptm->tm_mday, monthsOfYear[ptm->tm_mon], ptm->tm_year + 1900);
      lcd.print(dateBuffer);
    }

    showDayToggle = !showDayToggle; 
  }
}
