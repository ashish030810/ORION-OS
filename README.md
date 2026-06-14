# 🛰️ Orion-OS v1.1: Enterprise Multi-Module Command & Control Kernel

Welcome to the **Orion-OS** ecosystem. This repository contains a production-grade, multi-layered hardware control kernel designed to simulate and manage a deep-space habitat command console. This codebase acts as a 100-hour milestone engineering project for the Hack Club Stardance challenge.

---

## 🧠 Core Architecture & System Logic
Orion-OS runs on a highly decoupled **Finite State Machine (FSM)** paired with an asynchronous scheduler loop. Instead of relying on blocking execution utilities (`delay()`) which freeze processor execution, the system continuously tracks background time deltas using non-blocking millisecond offsets (`millis()`). This layout ensures the uplink telemetry firewall remains fully active while concurrently driving background screen refreshes.

### Operational State Matrix:
* `STATE_KERNEL_BOOTING` - Runs diagnostic self-tests and hardware peripheral mapping.
* `STATE_STATION_STANDBY` - Default background loop, listening for incoming serial commands.
* `STATE_DIAGNOSTIC_SWEEP` - Iterative physical layer monitoring sequence scanning individual module pins.
* `STATE_CRITICAL_LOCKDOWN` - System isolation mode following an unauthorized hardware/software access attempt.

---

## 📦 Hardware Component Matrix

| Qty | Component Name | Functional System Purpose |
| :--- | :--- | :--- |
| 1 | **Seeed Studio XIAO RP2040** | High-performance dual-core micro-processing core. |
| 1 | **I2C 16x2 Character LCD** | Cockpit terminal matrix display for live parameter feedback. |
| 1 | **I2C Backpack (PCF8574)** | Pins expander to reduce screen interface routing to 2 data lines. |
| 4 | **Tactile Push Buttons** | Manual hardware override switches for localized isolation. |
| 1 | **Active Piezo Buzzer** | High-frequency audio indicator for safety threshold alerts. |

---

## 🎛️ Terminal Command Interface Operations

When connected via the serial gateway interface at **9600 Baud**, the system decrypts input payloads via the command processor pipeline:

* `H` - Accesses the internal diagnostic systems operations manual directory.
* `D` - Triggers a sequential 4-stage automated component integrity evaluation scan.
* `0` - Disengages power routing relays to Module 3 (COMM-SAT-GRID).
* `1` - Safely energizes power lines to Module 3 (COMM-SAT-GRID).

---

## 🚀 Future Roadmap & Iterations
* **v2.0 (Planned):** Life Support & Environmental Fluid Dynamics Matrix integration.
* **v3.0 (Planned):** EEPROM-backed crew biometric registration database logs.
* **v4.0 (Planned):** Custom data packet compression algorithm for deep-space telemetry arrays.
