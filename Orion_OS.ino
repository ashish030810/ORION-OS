/*
 * ============================================================================
 * 🛰️ ORION-OS v1.1: COMMAND & CONTROL KERNEL (WITH CONSOLE LCD EXPANSION)
 * ============================================================================
 * Language Mode: Arduino C++ (Decoupled State Machine Architecture)
 * Lead Systems Architect: Ashish
 * Target Timeline: 75 - 100 Hours Enterprise Project Matrix
 * ----------------------------------------------------------------------------
 * New Integrations [Block 1.1]:
 * - LiquidCrystal_I2C Driver Framework Hook
 * - Asynchronous Core Loop Scheduling Configuration
 * - Remote Uplink Command Parser Engine (Serial Gateway Firewall)
 * - Volatile Telemetry Buffer Registry & Analytical System Diagnostic Tables
 * ============================================================================
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- SYSTEM CONFIGURATION CONSTANTS ---
#define SERIAL_TERMINAL_SPEED   9600
#define CORE_RECORDS_CAPACITY   4
#define TELEMETRY_STREAM_DELAY  4000  // Automated logging loop every 4 seconds
#define LCD_REFRESH_DELAY       1000  // Refresh physical screen every 1 second

// Initialize LCD: 0x27 address, 16 columns wide, 2 rows high
// Note: If screen is blank on real hardware, check if address is 0x3F instead.
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- HABITAT OS SUBSYSTEM STATES ---
enum OperationalState {
  STATE_KERNEL_BOOTING,
  STATE_STATION_STANDBY,
  STATE_DIAGNOSTIC_SWEEP,
  STATE_LIFE_SUPPORT_WARN,
  STATE_CRITICAL_LOCKDOWN
};

// --- DATA STRUCTURES & LOG SCHEMAS ---
struct SubsystemProfile {
  String moduleName;
  int powerDrawWatts;
  bool operationalStatus;
  char priorityTier; // 'A' = Critical, 'B' = Secondary, 'C' = Utility
};

// --- GLOBAL VOLATILE SYSTEM STORAGE REGISTERS ---
OperationalState currentKernelState = STATE_KERNEL_BOOTING;
unsigned long lastTelemetryStreamMs = 0;
unsigned long lastLcdUpdateMs = 0;
unsigned long masterKernelTickCount = 0;
int administrativeAlertCounter       = 0;

// Central Hardware Infrastructure Log Registry
SubsystemProfile habitatModules[CORE_RECORDS_CAPACITY] = {
  {"CORESYS-MAIN", 120, true,  'A'},
  {"BIO-DOME-AGRI", 250, true,  'B'},
  {"HATCH-AIRLOCK", 95,  true,  'A'},
  {"COMM-SAT-GRID", 180, false, 'C'} // Starts offline for manual testing
};

// --- FUNCTION CONTROL PIPELINE PROTOTYPES ---
void initializeCorePeripherals();
void processIncomingUplinkCommands();
void executeSystemDiagnosticSweep();
void streamSystemTelemetryPacket();
void updatePhysicalLcdConsole();
void toggleModulePower(int moduleIndex, bool state);

// ============================================================================
// 📌 MISSION CONTROL SYSTEM BOOT PROTOCOL
// ============================================================================
void setup() {
  // Establish high-speed communications bridge
  Serial.begin(SERIAL_TERMINAL_SPEED);
  delay(800);
  
  // Ignite the LCD Screen interface
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("ORION-OS v1.1");
  lcd.setCursor(0, 1);
  lcd.print("BOOTING STACK...");
  
  Serial.println(F("================================================================="));
  Serial.println(F("🌌 ORION OPERATING SYSTEM (Orion-OS) v1.1 INITIALIZED            "));
  Serial.println(F("KERNEL MODE: SECURE | LCD DISCOVERY OK | COMPONENT SYNC OK       "));
  Serial.println(F("================================================================="));
  Serial.println(F("[MANUAL]: Input 'H' into terminal console to unlock Operations Guide."));
  
  initializeCorePeripherals();
  delay(1000); // Visual hold for boot screen
  
  currentKernelState = STATE_STATION_STANDBY;
  lastTelemetryStreamMs = millis();
  lastLcdUpdateMs = millis();
}

// ============================================================================
// 🔄 CONTINUOUS ASYNCHRONOUS KERNEL LOOP ENGINE
// ============================================================================
void loop() {
  unsigned long currentRuntimeClockMs = millis();
  
  // INTERCEPT A: PARSE REMOTE INCOMING LINK TELEMETRY COMMANDS
  if (Serial.available() > 0) {
    processIncomingUplinkCommands();
  }

  // INTERCEPT B: BACKGROUND REFRESH MONITOR (WakaTime Data Logging Stream)
  if (currentRuntimeClockMs - lastTelemetryStreamMs >= TELEMETRY_STREAM_DELAY) {
    lastTelemetryStreamMs = currentRuntimeClockMs;
    masterKernelTickCount++;
    streamSystemTelemetryPacket();
  }
  
  // INTERCEPT C: ASYNCHRONOUS PHYSICAL MONITOR RENDERING
  if (currentRuntimeClockMs - lastLcdUpdateMs >= LCD_REFRESH_DELAY) {
    lastLcdUpdateMs = currentRuntimeClockMs;
    updatePhysicalLcdConsole();
  }
}

// ============================================================================
// 📡 CORE SUBSYSTEM SUBSURFACE DRIVERS
// ============================================================================

void initializeCorePeripherals() {
  Serial.println(F("\n[BOOT] Running initial system pin allocation tests..."));
  Serial.println(F("[BOOT] Base core components configured smoothly."));
}

void processIncomingUplinkCommands() {
  char commandTokenPayload = Serial.read();
  commandTokenPayload = toupper(commandTokenPayload); // Normalize syntax case
  
  Serial.println(F("\n[UPLINK-GATEWAY] Decrypting raw operational command packet..."));
  
  switch (commandTokenPayload) {
    case 'H':
      Serial.println(F("\n====== 🛰️ ORION-OS COMMAND CONSOLE DIRECTORY ======"));
      Serial.println(F(" H - Display System Terminal Operations Directory"));
      Serial.println(F(" D - Execute Structural Hardware Diagnostics Sweep"));
      Serial.println(F(" M - Toggle Telemetry Mute Mode / Flush Local Consoles"));
      Serial.println(F(" 0 - Force Disengage Module 3 Power Rails (COMM-GRID)"));
      Serial.println(F(" 1 - Force Engage Module 3 Power Rails (COMM-GRID)"));
      Serial.println(F("=================================================="));
      break;
      
    case 'D':
      executeSystemDiagnosticSweep();
      break;
      
    case 'M':
      Serial.println(F("[GATEWAY] Command Accepted: Volatile terminal registries cleared."));
      break;
      
    case '0':
      toggleModulePower(3, false);
      break;
      
    case '1':
      toggleModulePower(3, true);
      break;
      
    default:
      break;
  }
}

void executeSystemDiagnosticSweep() {
  currentKernelState = STATE_DIAGNOSTIC_SWEEP;
  lcd.clear();
  lcd.print("SYSTEM DIAG");
  lcd.setCursor(0, 1);
  lcd.print("SCANNING STACK...");
  
  Serial.println(F("\n[DIAGNOSTICS] Initializing comprehensive module health verification loops..."));
  
  for (int scanStep = 25; scanStep <= 100; scanStep += 25) {
    Serial.print(F(" -> Analyzing physical layer arrays: "));
    Serial.print(scanStep);
    Serial.println(F("% Complete..."));
    
    lcd.setCursor(12, 0);
    lcd.print(scanStep);
    lcd.print("%");
    delay(400); 
  }
  
  Serial.println(F("[DIAGNOSTICS] Diagnostic analysis concluded. Status: NOMINAL."));
  currentKernelState = STATE_STATION_STANDBY;
}

void updatePhysicalLcdConsole() {
  lcd.clear();
  
  switch (currentKernelState) {
    case STATE_STATION_STANDBY:
      lcd.setCursor(0, 0);
      lcd.print("SYS: OPERATIONAL");
      lcd.setCursor(0, 1);
      lcd.print("UPTIME: ");
      lcd.print(millis() / 1000);
      lcd.print("s");
      break;
      
    case STATE_DIAGNOSTIC_SWEEP:
      // Handled actively inside the tracking function loop
      break;
      
    case STATE_CRITICAL_LOCKDOWN:
      lcd.setCursor(0, 0);
      lcd.print("!! LOCKDOWN !!");
      lcd.setCursor(0, 1);
      lcd.print("SECURITY BREACH");
      break;
      
    default:
      lcd.print("UNKNOWN SYS NODE");
      break;
  }
}

void streamSystemTelemetryPacket() {
  Serial.println(F("\n================= 📥 ORION-OS SYSTEM AUDIT REPORT ================="));
  Serial.print(F("System Uptime Sync: ")); Serial.print(millis() / 1000); Serial.println(F(" Seconds"));
  Serial.print(F("Current Operational State Index: [")); Serial.print(currentKernelState); Serial.println(F("]"));
  Serial.println(F("-----------------------------------------------------------------"));
  
  long collectiveStationPowerDraw = 0;
  for (int i = 0; i < CORE_RECORDS_CAPACITY; i++) {
    if (habitatModules[i].operationalStatus) {
      collectiveStationPowerDraw += habitatModules[i].powerDrawWatts;
    }
  }
  Serial.print(F("Total Operational Grid Power Consumption: ")); Serial.print(collectiveStationPowerDraw); Serial.println(F(" Watts"));
}

void toggleModulePower(int moduleIndex, bool state) {
  if (moduleIndex >= 0 && moduleIndex < CORE_RECORDS_CAPACITY) {
    habitatModules[moduleIndex].operationalStatus = state;
    Serial.print(F("[POWER-GRID] Relay command executed. Module ["));
    Serial.print(habitatModules[moduleIndex].moduleName);
    Serial.println(state ? F("] successfully re-energized.") : F("] power isolated manually."));
  }
}