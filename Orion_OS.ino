/*
 * ============================================================================
 * 🛰️ ORION-OS v1.1: COMPLETE COMMAND & CONTROL KERNEL (PRODUCTION READY)
 * ============================================================================
 * Language Mode: Arduino C++ (Decoupled State Machine Architecture)
 * Lead Systems Architect: Ashish
 * Target Hardware: Arduino MCU with 16x2 I2C LCD Display
 * ----------------------------------------------------------------------------
 * Features:
 * - Full 4-person Crew Manifest Rotation ('C' to trigger, 'M' to standby)
 * - Complete Error-Free Serial Gateway Parser
 * - Hardware Diagnostics Sweep Engine ('D' to trigger)
 * - Power Grid Isolation Simulation ('0' and '1' to trigger)
 * ============================================================================
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- SYSTEM CONFIGURATION CONSTANTS ---
#define SERIAL_TERMINAL_SPEED   9600
#define CORE_RECORDS_CAPACITY   4
#define CREW_SIZE               4
#define TELEMETRY_STREAM_DELAY  4000  // Automated logging loop every 4 seconds
#define LCD_REFRESH_DELAY       1000  // Refresh physical screen every 1 second
#define CREW_ROTATION_DELAY     2000  // Switch names on LCD every 2 seconds in Crew Mode

// Initialize LCD: 0x27 address, 16 columns wide, 2 rows high
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- HABITAT OS SUBSYSTEM STATES ---
enum OperationalState {
  STATE_KERNEL_BOOTING,
  STATE_STATION_STANDBY,
  STATE_DIAGNOSTIC_SWEEP,
  STATE_CREW_MANIFEST,
  STATE_CRITICAL_LOCKDOWN
};

// --- DATA STRUCTURES & LOG SCHEMAS ---
struct SubsystemProfile {
  String moduleName;
  int powerDrawWatts;
  bool operationalStatus;
  char priorityTier; 
};

// --- GLOBAL VOLATILE SYSTEM STORAGE REGISTERS ---
OperationalState currentKernelState = STATE_KERNEL_BOOTING;
unsigned long lastTelemetryStreamMs = 0;
unsigned long lastLcdUpdateMs = 0;
unsigned long lastCrewRotateMs = 0;
unsigned long masterKernelTickCount = 0;

// Track changes to avoid screen flickering
unsigned long lastDisplayedUptime = 0;
OperationalState lastDisplayedState = STATE_KERNEL_BOOTING;
int currentCrewDisplayIndex = 0;

// Hardcoded Crew Directory Matrix (4 Personnel Saved)
String stationCrew[CREW_SIZE] = {
  "1. ASHISH (LEAD)",
  "2. ALEX (ENG)",
  "3. SAM (BIO)",
  "4. ELENA (PILOT)"
};

// Central Hardware Infrastructure Log Registry
SubsystemProfile habitatModules[CORE_RECORDS_CAPACITY] = {
  {"CORESYS-MAIN", 120, true,  'A'},
  {"BIO-DOME-AGRI", 250, true,  'B'},
  {"HATCH-AIRLOCK", 95,  true,  'A'},
  {"COMM-SAT-GRID", 180, false, 'C'} 
};

// --- FUNCTION CONTROL PIPELINE PROTOTYPES ---
void initializeCorePeripherals();
void processIncomingUplinkCommands();
void executeSystemDiagnosticSweep();
void streamSystemTelemetryPacket();
void updatePhysicalLcdConsole(bool forceRefresh);
void toggleModulePower(int moduleIndex, bool state);

// ============================================================================
// 📌 MISSION CONTROL SYSTEM BOOT PROTOCOL
// ============================================================================
void setup() {
  Serial.begin(SERIAL_TERMINAL_SPEED);
  delay(800);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("ORION-OS v1.2");
  lcd.setCursor(0, 1);
  lcd.print("BOOTING STACK...");
  
  Serial.println(F("================================================================="));
  Serial.println(F("🌌 ORION OPERATING SYSTEM (Orion-OS) v1.2 INITIALIZED            "));
  Serial.println(F("KERNEL MODE: SECURE | CREW COMPILATION OK | REGISTRY COMPACT     "));
  Serial.println(F("================================================================="));
  Serial.println(F("[MANUAL]: Input 'H' into terminal console to unlock Operations Guide."));
  
  initializeCorePeripherals();
  delay(1000); 
  
  currentKernelState = STATE_STATION_STANDBY;
  lastTelemetryStreamMs = millis();
  lastLcdUpdateMs = millis();
  
  updatePhysicalLcdConsole(true);
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

  // INTERCEPT B: BACKGROUND REFRESH MONITOR
  if (currentRuntimeClockMs - lastTelemetryStreamMs >= TELEMETRY_STREAM_DELAY) {
    lastTelemetryStreamMs = currentRuntimeClockMs;
    masterKernelTickCount++;
    streamSystemTelemetryPacket();
  }
  
  // INTERCEPT C: ASYNCHRONOUS PHYSICAL MONITOR RENDERING
  if (currentRuntimeClockMs - lastLcdUpdateMs >= LCD_REFRESH_DELAY) {
    lastLcdUpdateMs = currentRuntimeClockMs;
    updatePhysicalLcdConsole(false);
  }
}

// ============================================================================
// 📡 CORE SUBSYSTEM SUBSURFACE DRIVERS
// ============================================================================

void initializeCorePeripherals() {
  Serial.println(F("\n[BOOT] Synchronizing onboard personnel databases..."));
  Serial.println(F("[BOOT] 4 Active Crew Manifest Profiles loaded successfully."));
}

void processIncomingUplinkCommands() {
  char commandTokenPayload = Serial.read();
  commandTokenPayload = toupper(commandTokenPayload); 
  
  while (Serial.available() > 0 && (Serial.peek() == '\n' || Serial.peek() == '\r')) {
    Serial.read();
  }
  
  if (commandTokenPayload == '\n' || commandTokenPayload == '\r') {
    return;
  }
  
  Serial.println(F("\n[UPLINK-GATEWAY] Decrypting raw operational command packet..."));
  
  switch (commandTokenPayload) {
    case 'H':
      Serial.println(F("\n====== 🛰️ ORION-OS COMMAND CONSOLE DIRECTORY ======"));
      Serial.println(F(" H - Display System Terminal Operations Directory"));
      Serial.println(F(" D - Execute Structural Hardware Diagnostics Sweep"));
      Serial.println(F(" C - Render Onboard Crew Manifest Logs to LCD Display"));
      Serial.println(F(" M - Reset Engine / Return to Default Standby Screens"));
      Serial.println(F(" 0 - Force Disengage Module 3 Power Rails (COMM-GRID)"));
      Serial.println(F(" 1 - Force Engage Module 3 Power Rails (COMM-GRID)"));
      Serial.println(F("=================================================="));
      break;
      
    case 'D':
      executeSystemDiagnosticSweep();
      break;

    case 'C':
      currentKernelState = STATE_CREW_MANIFEST;
      currentCrewDisplayIndex = 0;
      lastCrewRotateMs = millis();
      Serial.println(F("[GATEWAY] Command Accepted: Displaying Crew List on LCD screen monitor."));
      updatePhysicalLcdConsole(true);
      break;
      
    case 'M':
      currentKernelState = STATE_STATION_STANDBY;
      Serial.println(F("[GATEWAY] Command Accepted: Returning system to Standby Operational state."));
      updatePhysicalLcdConsole(true);
      break;
      
    case '0':
      toggleModulePower(3, false);
      break;
      
    case '1':
      toggleModulePower(3, true);
      break;
      
    default:
      Serial.print(F("[ALERT] Unidentified Vector Token Received: '"));
      Serial.print(commandTokenPayload);
      Serial.println(F("'"));
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
  updatePhysicalLcdConsole(true);
}

void updatePhysicalLcdConsole(bool forceRefresh) {
  unsigned long currentSeconds = millis() / 1000;
  unsigned long currentMs = millis();
  
  // Handle automatic rotation of names when in Crew Manifest Mode
  if (currentKernelState == STATE_CREW_MANIFEST) {
    if (currentMs - lastCrewRotateMs >= CREW_ROTATION_DELAY) {
      lastCrewRotateMs = currentMs;
      currentCrewDisplayIndex = (currentCrewDisplayIndex + 1) % CREW_SIZE;
      forceRefresh = true; 
    }
  }

  // Anti-flicker checkpoint
  if (!forceRefresh && (currentKernelState == lastDisplayedState) && (currentKernelState != STATE_CREW_MANIFEST) && (currentSeconds == lastDisplayedUptime)) {
    return; 
  }
  
  lastDisplayedState = currentKernelState;
  lastDisplayedUptime = currentSeconds;
  
  lcd.clear();
  switch (currentKernelState) {
    case STATE_STATION_STANDBY:
      lcd.setCursor(0, 0);
      lcd.print("SYS: OPERATIONAL");
      lcd.setCursor(0, 1);
      lcd.print("UPTIME: ");
      lcd.print(currentSeconds);
      lcd.print("s");
      break;
      
    case STATE_DIAGNOSTIC_SWEEP:
      // Loop sequence directly updates frames natively
      break;

    case STATE_CREW_MANIFEST:
      lcd.setCursor(0, 0);
      lcd.print("ORION CREW MANIF:");
      lcd.setCursor(0, 1);
      lcd.print(stationCrew[currentCrewDisplayIndex]);
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
