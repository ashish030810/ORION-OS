/*
 * =========================================================================
 * ORION-OS v1.2 – ESP32 Wokwi (Stable Overlay)
 * =========================================================================
 * Button press → shows crew name on LCD for 10 seconds, beeps after 4s.
 * No flicker, no premature changes.
 * =========================================================================
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ========== PINS ==========
const int PIN_BUTTON_SECURITY   = 4;
const int PIN_BUTTON_DIAG       = 12;
const int PIN_BUTTON_CREW       = 13;
const int PIN_BUZZER            = 17;
const int PIN_LED_RED           = 5;
const int PIN_LED_GREEN         = 16;

// ========== TIMING ==========
const unsigned long BEEP_DELAY_MS    = 4000;   // 4 seconds
const unsigned long DISPLAY_DURATION = 10000;  // 10 seconds (changed from 7 for clarity)

// ========== CREW DATA ==========
const int TOTAL_CREW = 4;
String crewNames[TOTAL_CREW] = {"1. ASHISH", "2. ALEX", "3. SAM", "4. ELENA"};
bool crewInside[TOTAL_CREW] = {false, false, false, false};
int currentCrewIndex = 0;

// ========== STATE ==========
enum State { STANDBY, DIAGNOSTIC, CREW_MANIFEST };
State systemState = STANDBY;

// ========== VARIABLES ==========
unsigned long lastTelemetry = 0;
unsigned long lastLcdRefresh = 0;
unsigned long lastCrewRotate = 0;
unsigned long currentMillis = 0;

// Overlay control
bool overlayActive = false;
unsigned long overlayStartTime = 0;
String overlayName = "";
String overlayAction = "";

// Delayed beep
bool beepPending = false;
unsigned long beepTime = 0;

// Button edge detection
bool lastButtonState = HIGH;

// ========== CUSTOM ICONS ==========
uint8_t sprout[8] = {0b00100,0b00110,0b01100,0b00100,0b00100,0b01110,0b11111,0b00000};
uint8_t sun[8]    = {0b00100,0b10101,0b01110,0b11011,0b01110,0b10101,0b00100,0b00000};
uint8_t moon[8]   = {0b00110,0b01100,0b11000,0b11000,0b11000,0b01100,0b00110,0b00000};
uint8_t warn[8]   = {0b00000,0b00100,0b01010,0b01010,0b11111,0b11111,0b00000,0b00000};

// ========== PROTOTYPES ==========
void handleSecurityPress();
void runDiagnostics();
void playBeep(int freq, int duration);
void updateStandbyScreen(int moistureA, int moistureB, int temp, bool isDay);

// ========== SETUP ==========
void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, sprout);
  lcd.createChar(1, sun);
  lcd.createChar(2, moon);
  lcd.createChar(3, warn);

  pinMode(PIN_BUTTON_SECURITY, INPUT_PULLUP);
  pinMode(PIN_BUTTON_DIAG, INPUT_PULLUP);
  pinMode(PIN_BUTTON_CREW, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, LOW);

  lcd.print("ORION-OS v1.2");
  lcd.setCursor(0, 1);
  lcd.print("ESP32 READY");
  delay(2000);
  lcd.clear();

  Serial.println(F("========================================"));
  Serial.println(F("🌌 ORION-OS STARTED"));
  Serial.println(F("========================================"));
}

// ========== LOOP ==========
void loop() {
  currentMillis = millis();

  // ----- 1. Read Sensors (simulated) -----
  int moistureA = map(analogRead(34), 0, 4095, 0, 100);
  int moistureB = map(analogRead(35), 0, 4095, 0, 100);
  int tempC     = map(analogRead(32), 0, 4095, -10, 50);
  bool isDay    = (currentMillis / 20000) % 2 == 0;

  // ----- 2. Security Button (Edge Detection) -----
  bool currentButton = digitalRead(PIN_BUTTON_SECURITY);
  if (currentButton == LOW && lastButtonState == HIGH) {
    handleSecurityPress();
  }
  lastButtonState = currentButton;

  // ----- 3. Diagnostics Button -----
  if (digitalRead(PIN_BUTTON_DIAG) == LOW && systemState != DIAGNOSTIC) {
    runDiagnostics();
    delay(200);
  }

  // ----- 4. Crew Manifest Toggle -----
  if (digitalRead(PIN_BUTTON_CREW) == LOW) {
    if (systemState == CREW_MANIFEST) systemState = STANDBY;
    else systemState = CREW_MANIFEST;
    delay(200);
  }

  // ----- 5. Handle Delayed Beep -----
  if (beepPending && currentMillis >= beepTime) {
    beepPending = false;
    playBeep(523, 200);
    Serial.println(F("🔊 BEEP! (4s delay completed)"));
  }

  // ----- 6. Check Overlay Timeout (10 seconds) -----
  if (overlayActive && (currentMillis - overlayStartTime >= DISPLAY_DURATION)) {
    overlayActive = false;
    lcd.clear();
    Serial.println(F("⏹️ Overlay ended. Back to standby."));
  }

  // ----- 7. Serial Telemetry (every 4s) -----
  if (currentMillis - lastTelemetry >= 4000) {
    lastTelemetry = currentMillis;
    Serial.print(F("📊 Uptime: ")); Serial.print(currentMillis/1000); Serial.println("s");
  }

  // ----- 8. LCD Refresh (non-blocking, every 1s) -----
  // ⚠️ CRITICAL: If overlay is active, we SKIP everything else!
  if (currentMillis - lastLcdRefresh >= 1000) {
    lastLcdRefresh = currentMillis;

    // --- OVERLAY ACTIVE: Show the crew name/action (and do NOTHING else) ---
    if (overlayActive) {
      // The LCD was already set in handleSecurityPress().
      // We do NOT clear or redraw here – that would cause flicker.
      // Just skip to the end.
      // (We don't need to do anything here because the text stays perfectly.)
    }
    // --- DIAGNOSTIC MODE ---
    else if (systemState == DIAGNOSTIC) {
      // Handled inside runDiagnostics()
    }
    // --- CREW MANIFEST MODE ---
    else if (systemState == CREW_MANIFEST) {
      if (currentMillis - lastCrewRotate >= 2000) {
        lastCrewRotate = currentMillis;
        currentCrewIndex = (currentCrewIndex + 1) % TOTAL_CREW;
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CREW MANIFEST");
      lcd.setCursor(0, 1);
      lcd.print(crewNames[currentCrewIndex]);
    }
    // --- STANDBY MODE (Main Dashboard) ---
    else {
      updateStandbyScreen(moistureA, moistureB, tempC, isDay);
    }
  }

  delay(20);
}

// ========== SECURITY PRESS ==========
void handleSecurityPress() {
  // Toggle entry/exit
  crewInside[currentCrewIndex] = !crewInside[currentCrewIndex];
  bool inside = crewInside[currentCrewIndex];

  overlayName = crewNames[currentCrewIndex];
  overlayAction = inside ? "STATUS: ENTRY" : "STATUS: EXIT";

  // Show immediately on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(overlayName);
  lcd.setCursor(0, 1);
  lcd.print(overlayAction);

  // Activate overlay (this will block standby updates for 10 seconds)
  overlayActive = true;
  overlayStartTime = millis();

  // Schedule the beep for 4 seconds later
  beepPending = true;
  beepTime = millis() + BEEP_DELAY_MS;

  // Advance to next crew member for next press
  currentCrewIndex = (currentCrewIndex + 1) % TOTAL_CREW;

  // Debug log
  Serial.println(F("🔒 BUTTON PRESSED"));
  Serial.print(F("   Showing: ")); Serial.println(overlayName);
  Serial.print(F("   Overlay active for ")); Serial.print(DISPLAY_DURATION/1000); Serial.println("s");
  Serial.print(F("   Beep scheduled in ")); Serial.print(BEEP_DELAY_MS/1000); Serial.println("s");
}

// ========== DIAGNOSTICS ==========
void runDiagnostics() {
  systemState = DIAGNOSTIC;
  Serial.println(F("[DIAG] Starting sweep..."));
  for (int p = 25; p <= 100; p += 25) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DIAGNOSTICS");
    lcd.setCursor(0, 1);
    lcd.write(3); lcd.print(" "); lcd.print(p); lcd.print("%");
    playBeep(440, 80);
    delay(400);
  }
  Serial.println(F("[DIAG] Complete."));
  systemState = STANDBY;
  lcd.clear();
}

// ========== STANDBY SCREEN ==========
void updateStandbyScreen(int mA, int mB, int temp, bool isDay) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(0); lcd.print(" A:"); lcd.print(mA); lcd.print("% B:"); lcd.print(mB); lcd.print("%");
  lcd.setCursor(0, 1);
  if (isDay) { lcd.write(1); lcd.print(" DAY "); } 
  else { lcd.write(2); lcd.print(" NIGHT"); }
  lcd.print(" T:"); lcd.print(temp); lcd.print("C");
}

// ========== BUZZER ==========
void playBeep(int freq, int duration) {
  tone(PIN_BUZZER, freq, duration);
  delay(10);
}
