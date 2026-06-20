/*
 * =========================================================================
 * 🌿 ASTRO-FLORA / ORION-OS : UNIFIED CONTROL KERNEL (ESP32 SIMULATED)
 * =========================================================================
 * Lead Systems Architect: ASHISH
 * Target Hardware Platform: ESP32 DevKit v1 (Wokwi Virtualization Layer)
 * =========================================================================
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize LCD Display Panel: I2C Address 0x27, 16 columns wide, 2 rows high
// Note: On ESP32, Wire automatically connects to Pins 21 (SDA) and 22 (SCL)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// =========================================================================
// 🛠️ REGISTER PIN ASSIGNMENTS (MATCHING YOUR ESP32 BOARD CODES)
// =========================================================================
// Analog Inputs (Simulated Matrix Thresholds)
const int PIN_SENSOR_MOISTURE_ZONE_A = 34;  // GPIO 34 (Analog Read Only)
const int PIN_SENSOR_MOISTURE_ZONE_B = 35;  // GPIO 35 (Analog Read Only)
const int PIN_SENSOR_THERMISTOR_AIR  = 32;  // GPIO 32
const int PIN_SENSOR_FLUID_RESERVOIR = 33;  // GPIO 33

// Digital Inputs (Buttons using Internal Pull-Up Resistors)
const int PIN_BUTTON_SECURITY_SCAN   = 4;   // GPIO 4 (Security Access Scan Button)
const int PIN_BUTTON_DIAGNOSTICS    = 12;  // GPIO 12 (Diagnostics 'D' Button)
const int PIN_BUTTON_CREW_MANIFEST   = 13;  // GPIO 13 (Crew Rotation 'C' Button)

// Digital Outputs (Actuators, Buzzers, and Display Signal LEDs)
const int PIN_AUDIO_ALARM_BUZZER     = 17;  // GPIO 17 (Piezo Sound Elements)
const int PIN_RELAY_THERMAL_HEATER   = 5;   // GPIO 5 (Red System LED Control)
const int PIN_RELAY_EXHAUST_FAN      = 16;  // GPIO 16 (Green System LED Control)

// =========================================================================
// ⚙️ SYSTEM SETPOINTS AND TIMING PARAMETERS
// =========================================================================
const int SETPOINT_DRY_THRESHOLD       = 450;
const int SETPOINT_MIN_RESERVOIR_LEVEL = 200;
const int SETPOINT_CRITICAL_LOW_TEMP   = 16;
const int SETPOINT_CRITICAL_HIGH_TEMP  = 30;

const unsigned long DELAY_TELEMETRY_REFRESH = 4000;  // Serial update clock speed
const unsigned long DELAY_SOLAR_CYCLE_SHIFT = 20000; // Simulated day duration
const unsigned long DELAY_LCD_REFRESH       = 1000;  // Anti-flicker delay clock
const unsigned long CREW_ROTATION_DELAY     = 2000;  // Crew names swap timing

// =========================================================================
// 📊 STATE ENGINE LOG SCHEMAS & VARIABLES
// =========================================================================
enum OperationalState {
  STATE_STATION_STANDBY,
  STATE_DIAGNOSTIC_SWEEP,
  STATE_CREW_MANIFEST
};

OperationalState currentKernelState = STATE_STATION_STANDBY;

unsigned long timeTrackerLastTelemetry   = 0;
unsigned long timeTrackerLastSolarShift  = 0;
unsigned long timeTrackerLastLcdRefresh  = 0;
unsigned long timeTrackerLastCrewRotate  = 0;
unsigned long timeTrackerCurrentExecution = 0;

bool statusFlagDaytimeCycleActive = true;
int activeCrewDatabasePointer      = 0;
const int TOTAL_AUTHORIZED_CREW    = 4;

unsigned long securityDisplayWindowMs = 0;
bool showSecurityScreenOverlay = false;
String dynamicSecurityName = "";
String dynamicSecurityAction = "";

// Database Arrays
String crewNameRegistry[TOTAL_AUTHORIZED_CREW] = {
  "1. ASHISH (LEAD)", "2. ALEX (ENG)", "3. SAM (BIO)", "4. ELENA (PILOT)"
};
bool crewInsideStatus[TOTAL_AUTHORIZED_CREW] = {false, false, false, false};

// Custom Hexadecimal LCD Glyphs 
uint8_t graphicIconSprout[8]  = { 0b00100, 0b00110, 0b01100, 0b00100, 0b00100, 0b01110, 0b11111, 0b00000 };
uint8_t graphicIconSun[8]     = { 0b00100, 0b10101, 0b01110, 0b11011, 0b01110, 0b10101, 0b00100, 0b00000 };
uint8_t graphicIconMoon[8]    = { 0b00110, 0b01100, 0b11000, 0b11000, 0b11000, 0b01100, 0b00110, 0b00000 };
uint8_t graphicIconWarning[8] = { 0b00000, 0b00100, 0b01010, 0b01010, 0b11111, 0b11111, 0b00000, 0b00000 };

// Function Prototypes
void executeCrewAccessTransaction();
void executeSystemDiagnosticSweep();
void executeAcousticPulse(int parameterFrequencyHz, int parameterDurationMs);

// =========================================================================
// 📌 SETUP INITIALIZATION SEQUENCE
// =========================================================================
void setup() {
  Serial.begin(9600);
  
  // Power up and flash the liquid crystal matrix panel
  lcd.init();
  lcd.backlight();
  
  // Inject specialized pixel icon vectors into CGRAM chip storage
  lcd.createChar(0, graphicIconSprout);
  lcd.createChar(1, graphicIconSun);
  lcd.createChar(2, graphicIconMoon);
  lcd.createChar(3, graphicIconWarning);
  
  // Register pin operational modes across hardware map
  pinMode(PIN_BUTTON_SECURITY_SCAN, INPUT_PULLUP);
  pinMode(PIN_BUTTON_DIAGNOSTICS, INPUT_PULLUP);
  pinMode(PIN_BUTTON_CREW_MANIFEST, INPUT_PULLUP);
  
  pinMode(PIN_RELAY_THERMAL_HEATER, OUTPUT);
  pinMode(PIN_RELAY_EXHAUST_FAN, OUTPUT);
  pinMode(PIN_AUDIO_ALARM_BUZZER, OUTPUT);
  
  // Set starting values
  digitalWrite(PIN_RELAY_THERMAL_HEATER, LOW);
  digitalWrite(PIN_RELAY_EXHAUST_FAN, LOW);
  
  executeAcousticPulse(880, 200);
  
  lcd.clear();
  lcd.print("ORION-OS v1.2");
  lcd.setCursor(0, 1);
  lcd.print("ESP32 HUB ACTIVE");
  
  Serial.println(F("================================================================="));
  Serial.println(F("🌌 ORION CONTROL KERNEL OPERATIONAL FOR STARDANCE SIMULATION"));
  Serial.println(F("================================================================="));
  
  delay(2000);
}

// =========================================================================
// 🔄 CORE ASYNCHRONOUS ENGINE LOOP
// =========================================================================
void loop() {
  timeTrackerCurrentExecution = millis();
  
  // 1. Read Data Sensors (Simulated Inputs via Wokwi ADC channels)
  int rawValueMoistureZoneA  = analogRead(PIN_SENSOR_MOISTURE_ZONE_A);
  int rawValueMoistureZoneB  = analogRead(PIN_SENSOR_MOISTURE_ZONE_B);
  int rawValueAtmosphereTemp = analogRead(PIN_SENSOR_THERMISTOR_AIR);
  int rawValueWaterReservoir = analogRead(PIN_SENSOR_FLUID_RESERVOIR);
  
  int computedPercentMoistureA = map(rawValueMoistureZoneA, 0, 4095, 0, 100);
  int computedPercentMoistureB = map(rawValueMoistureZoneB, 0, 4095, 0, 100);
  int computedCelsiusDegree     = map(rawValueAtmosphereTemp, 0, 4095, -10, 50);

  // 2. Hardware Interrupt Button Scans (Non-Blocking Inputs)
  if (digitalRead(PIN_BUTTON_SECURITY_SCAN) == LOW) {
    executeCrewAccessTransaction();
    delay(250); // Simple debounce protection window
  }
  
  if (digitalRead(PIN_BUTTON_DIAGNOSTICS) == LOW && currentKernelState != STATE_DIAGNOSTIC_SWEEP) {
    executeSystemDiagnosticSweep();
  }
  
  if (digitalRead(PIN_BUTTON_CREW_MANIFEST) == LOW) {
    currentKernelState = (currentKernelState == STATE_CREW_MANIFEST) ? STATE_STATION_STANDBY : STATE_CREW_MANIFEST;
    lcd.clear();
    executeAcousticPulse(523, 100);
    delay(250);
  }

  // 3. Solar Timing Grid System
  if (timeTrackerCurrentExecution - timeTrackerLastSolarShift >= DELAY_SOLAR_CYCLE_SHIFT) {
    statusFlagDaytimeCycleActive = !statusFlagDaytimeCycleActive;
    timeTrackerLastSolarShift = timeTrackerCurrentExecution;
  }
  
  // 4. Climate HVAC Automatic LED Outputs
  if (computedCelsiusDegree < SETPOINT_CRITICAL_LOW_TEMP) {
    digitalWrite(PIN_RELAY_THERMAL_HEATER, HIGH);  // Red LED Turns ON
    digitalWrite(PIN_RELAY_EXHAUST_FAN, LOW);
  } else if (computedCelsiusDegree > SETPOINT_CRITICAL_HIGH_TEMP) {
    digitalWrite(PIN_RELAY_THERMAL_HEATER, LOW);
    digitalWrite(PIN_RELAY_EXHAUST_FAN, HIGH);    // Green LED Turns ON
  } else {
    digitalWrite(PIN_RELAY_THERMAL_HEATER, LOW);
    digitalWrite(PIN_RELAY_EXHAUST_FAN, LOW);
  }
  
  // 5. Serial Logging Output Telemetry System
  if (timeTrackerCurrentExecution - timeTrackerLastTelemetry >= DELAY_TELEMETRY_REFRESH) {
    timeTrackerLastTelemetry = timeTrackerCurrentExecution;
    Serial.println(F("\n🛰️ --- ORION-OS SYSTEM AUDIT DATALOG ---"));
    Serial.print(F("Uptime Sync: ")); Serial.print(timeTrackerCurrentExecution / 1000); Serial.println(F("s"));
    Serial.print(F("Zone A Soil Matrix: ")); Serial.print(computedPercentMoistureA); Serial.println(F("%"));
    Serial.print(F("Atmosphere Module Heat: ")); Serial.print(computedCelsiusDegree); Serial.println(F(" C"));
    Serial.println(F("--------------------------------------------------"));
  }
  
  // 6. Non-Blocking Screen Refresh Panel Engine (Stops flickering)
  if (timeTrackerCurrentExecution - timeTrackerLastLcdRefresh >= DELAY_LCD_REFRESH) {
    timeTrackerLastLcdRefresh = timeTrackerCurrentExecution;
    
    if (showSecurityScreenOverlay && (timeTrackerCurrentExecution - securityDisplayWindowMs >= 2500)) {
      showSecurityScreenOverlay = false;
      lcd.clear();
    }

    if (showSecurityScreenOverlay) {
      lcd.setCursor(0, 0);
      lcd.print(dynamicSecurityName);
      lcd.setCursor(0, 1);
      lcd.print(dynamicSecurityAction);
    } 
    else if (currentKernelState == STATE_DIAGNOSTIC_SWEEP) {
      // Handled actively inside the sweep loop method
    }
    else if (currentKernelState == STATE_CREW_MANIFEST) {
      if (timeTrackerCurrentExecution - timeTrackerLastCrewRotate >= CREW_ROTATION_DELAY) {
        timeTrackerLastCrewRotate = timeTrackerCurrentExecution;
        activeCrewDatabasePointer = (activeCrewDatabasePointer + 1) % TOTAL_AUTHORIZED_CREW;
      }
      lcd.setCursor(0, 0);
      lcd.print("CREW REGISTRY:");
      lcd.setCursor(0, 1);
      lcd.print(crewNameRegistry[activeCrewDatabasePointer]);
    }
    else { // Default Standby Screen Display Layer
      lcd.setCursor(0, 0);
      lcd.write(0); // Display sprout glyph
      lcd.print(" A:"); lcd.print(computedPercentMoistureA);
      lcd.print("% B:"); lcd.print(computedPercentMoistureB); lcd.print("%");
      
      lcd.setCursor(0, 1);
      if (statusFlagDaytimeCycleActive) { lcd.write(1); lcd.print(" DAY "); }
      else { lcd.write(2); lcd.print(" NIGHT"); }
      lcd.print(" T:"); lcd.print(computedCelsiusDegree); lcd.print("C");
    }
  }
  
  delay(20); // Small cycle governor
}

// =========================================================================
// 📡 INTERFACE SUBROUTINES
// =========================================================================
void executeCrewAccessTransaction() {
  crewInsideStatus[activeCrewDatabasePointer] = !crewInsideStatus[activeCrewDatabasePointer];
  
  dynamicSecurityName = crewNameRegistry[activeCrewDatabasePointer];
  bool isInside = crewInsideStatus[activeCrewDatabasePointer];
  dynamicSecurityAction = isInside ? "-> STATUS: ENTRY" : "-> STATUS: EXIT";
  
  showSecurityScreenOverlay = true;
  securityDisplayWindowMs = millis();
  lcd.clear();
  
  if (isInside) { executeAcousticPulse(523, 100); executeAcousticPulse(659, 100); } 
  else { executeAcousticPulse(659, 100); executeAcousticPulse(523, 100); }
}

void executeSystemDiagnosticSweep() {
  currentKernelState = STATE_DIAGNOSTIC_SWEEP;
  Serial.println(F("\n[DIAGNOSTICS] Starting system sweep verification..."));
  
  for (int progress = 25; progress <= 100; progress += 25) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SYSTEM DIAGNOSTICS");
    lcd.setCursor(0, 1);
    lcd.write(3); // Warning sign
    lcd.print(" STATUS: "); lcd.print(progress); lcd.print("%");
    executeAcousticPulse(440, 80);
    delay(400); 
  }
  
  Serial.println(F("[DIAGNOSTICS] Concluded. Status: NOMINAL"));
  currentKernelState = STATE_STATION_STANDBY;
  lcd.clear();
}

void executeAcousticPulse(int parameterFrequencyHz, int parameterDurationMs) {
  tone(PIN_AUDIO_ALARM_BUZZER, parameterFrequencyHz, parameterDurationMs);
}
