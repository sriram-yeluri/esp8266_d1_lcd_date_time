#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Network Settings ---
const char* ssid     = "xxxxxx";
const char* password = "xxxxxx";

// --- Timezone Configuration ---
// Adjust the offset for your Time Zone in seconds.
// Example: UTC +5:30 = 5.5 * 3600 = 19800
// Example: UTC -5:00 (EST) = -5 * 3600 = -18000
const long timezoneOffsetInSeconds = 19800; 
const int daylightOffsetInSeconds = 0; // Set to 3600 if your area uses Daylight Savings

// --- LCD Settings ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* daysOfWeek[]  = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char* monthsOfYear[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// --- Timers (Non-blocking) ---
unsigned long lastDisplayCycle = 0;
const unsigned long cycleInterval = 4000; // Swap between Day and Date every 4 seconds

unsigned long lastClockTime = 0;
const unsigned long clockInterval = 500;  // Update clock every half second

bool showDayToggle = true;

void setup() {
  Wire.begin(4, 5);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Connecting Wi-Fi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Syncing Time...");

  // Initialize native ESP8266 NTP client config
  configTime(timezoneOffsetInSeconds, daylightOffsetInSeconds, "pool.ntp.org", "time.nist.gov");

  // Wait until time is successfully fetched from the internet (year won't be 1970 anymore)
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    now = time(nullptr);
  }
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Fetch current time variables from internal clock
  time_t now = time(nullptr);
  struct tm* ptm = localtime(&now); // Native internal timezone conversion

  // --- TASK 1: Update Clock (Row 0) ---
  if (currentMillis - lastClockTime >= clockInterval) {
    lastClockTime = currentMillis;
    
    char timeBuffer[16];
    // Automatically formats to "Time: HH:MM:SS"
    strftime(timeBuffer, sizeof(timeBuffer), "Time: %H:%M:%S", ptm);
    
    lcd.setCursor(0, 0);
    lcd.print(timeBuffer); 
  }

  // --- TASK 2: Alternate Day and Date on Row 1 ---
  if (currentMillis - lastDisplayCycle >= cycleInterval) {
    lastDisplayCycle = currentMillis;
    
    // Clear row 1 cleanly before adding new text
    lcd.setCursor(0, 1);
    lcd.print("                "); 
    lcd.setCursor(0, 1);

    if (showDayToggle) {
      // Screen A: Day Name (e.g., "Wednesday")
      lcd.print(daysOfWeek[ptm->tm_wday]);
    } else {
      // Screen B: Date (e.g., "09 Sep 2026")
      char dateBuffer[16];
      // %d=Day, strings fetched from abbreviation array, %Y=4-digit Year
      snprintf(dateBuffer, sizeof(dateBuffer), "%02d %s %d", ptm->tm_mday, monthsOfYear[ptm->tm_mon], ptm->tm_year + 1900);
      lcd.print(dateBuffer);
    }

    showDayToggle = !showDayToggle; 
  }
}
