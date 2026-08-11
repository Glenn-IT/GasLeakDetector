# SIM800L & Gas Leak Detector Troubleshooting Guide

This document summarizes all hardware, electrical, and software issues encountered during the build of the **Gas Leak Detector with SIM800L SMS Alert**, along with their exact root causes and verified solutions.

---

## 📋 Quick Problem & Solution Summary

| Issue / Symptom | Root Cause | Verified Fix |
| :--- | :--- | :--- |
| **SIM800L reboots during SMS transmission** | **Overvoltage Shutdown (OVP)**. SIM800L max rating is 4.4V. 5.2V from Buck causes instant reboot on 2A RF burst. | Insert a **1N4007 Diode** in series on `VCC` (Anode to 5.2V Buck, Cathode/silver stripe to SIM800L `VCC`). Drops voltage to safe **4.5V**. |
| **`GSM Not Responding` / Serial Silence** | **Missing Common Ground** or swapped TX/RX wires. | 1. Connect SIM `GND` / Buck `OUT-` to **Arduino `GND`**.<br>2. Ensure SIM `TXD` ──► Arduino **D8**, SIM `RXD` ──► Arduino **D7**. |
| **`Did not receive '>' prompt from SIM800L`** | Leftover serial buffer characters from `AT+CMGF=1` or 2A voltage sag. | Code flushes serial buffer before `AT+CMGS`, extends prompt timeout to 5s, and uses **Y-style / double jumper wires** for `VCC`/`GND`. |
| **Garbled text (`!#!!#...`) on Serial Monitor** | Baud rate mismatch (SIM800L at 115200 vs SoftwareSerial at 9600). | Code automatically probes baud rates at boot and locks SIM800L permanently to 9600 baud via `AT+IPR=9600` & `AT&W`. |
| **`Access is denied on COM8` (Arduino Upload Error)** | Serial Monitor tab in Arduino IDE is locking the COM port. | Close the Serial Monitor tab in Arduino IDE, unplug/replug USB cable, and re-upload. |

---

## 🛠️ Detailed Troubleshooting & Fix Breakdown

### 1. SIM800L Overvoltage Reboot Issue
* **Symptom**: SIM800L stays powered on while idle, but as soon as an gas alarm occurs and it tries to send an SMS, the module reboots or flashes fast continuously.
* **Electrical Cause**: The raw red SIM800L module has a operating voltage range of **3.4V to 4.4V**. When powered directly from a 5.2V Buck converter, the internal RF power amplifier triggers Overvoltage Protection (OVP) during the 2A transmit burst.
* **Solution**:
  - Insert a **1N4007 Diode** in series on the positive `VCC` wire:
    - **Anode (Black end without stripe)** ──► Buck Converter `OUT+` (5.2V)
    - **Cathode (End WITH silver/white stripe)** ──► SIM800L `VCC` pin
  - This creates a **0.7V forward voltage drop**, bringing 5.2V down to **~4.5V**, keeping the SIM800L safe and stable.

```text
Buck Converter 5.2V OUT(+) ───► [ ANODE ──► 1N4007 DIODE ──► CATHODE (Silver Stripe) ] ───► SIM800L VCC
```

---

### 2. High Current Jumper Wire Resistance (Y-Harness Solution)
* **Symptom**: Voltage sag on thin DuPont jumper wires during 2A bursts.
* **Solution**: Create a **Y-Style (Dual Wire) Harness**:
  - Insert **two Red wires in parallel** from Buck `OUT+` merging into the SIM800L `VCC` pin (via diode).
  - Insert **two Black wires in parallel** from Buck `OUT-` merging into the SIM800L `GND` pin.
  - This cuts line resistance in half and prevents current bottlenecks.

---

### 3. Missing Common Ground
* **Symptom**: Serial monitor prints `[GSM ERROR] GSM Not Responding!` even though SIM module LEDs are blinking.
* **Electrical Cause**: Transmit and receive data lines (D8/D7) require a shared ground reference path between the Arduino microcontroller and the SIM800L.
* **Solution**: Connect a jumper wire between **SIM800L `GND` / Buck `OUT-`** and the **Arduino `GND` pin**.

---

### 4. Software Serial Buffer & Prompt Handling
* **Symptom**: `[GSM ERROR] Did not receive '>' prompt from SIM800L. SMS failed.`
* **Software Cause**: When issuing `AT+CMGS="+63..."`, leftover `OK` characters from previous `AT+CMGF=1` commands in the SoftwareSerial buffer prevented matching the `>` prompt.
* **Software Solution in `GasLeakDetector.ino`**:
  - Explicitly wait for `AT+CMGF=1` `OK` response.
  - Flush the incoming buffer: `while (sim800.available()) sim800.read();` before sending `AT+CMGS`.
  - Extend prompt timeout to 5000ms.

---

### 5. USB Upload "Access is Denied" on COM Port
* **Symptom**: Arduino IDE displays `Error: cannot open port \\.\COM8: Access is denied.`
* **Cause**: Windows COM ports can only be opened by one process at a time. The Serial Monitor tab is holding COM8 open.
* **Solution**:
  1. Close the **Serial Monitor tab** in Arduino IDE.
  2. Unplug USB cable for 3 seconds and plug back in.
  3. Upload sketch (Ctrl + U).

---

## ⚡ Verified Working Power & Wiring Architecture

```text
                     ┌─────────────────────────────────────────┐
                     │ 5.2V / 5A High-Power Buck Converter     │
                     └────────────────────┬────────────────────┘
                                          │
                   ┌──────────────────────┴──────────────────────┐
                   │ Star Branch 1                               │ Star Branch 2
                   ▼                                             ▼
  ┌─────────────────────────────────┐                 ┌─────────────────────┐
  │ 1N4007 Diode (Anode to 5.2V,    │                 │ Breadboard +5V Rail │
  │ Cathode stripe to SIM VCC)      │                 │ (Arduino, LCD, MQ2) │
  └────────────────┬────────────────┘                 └─────────────────────┘
                   │ 4.5V Safe Power
                   ▼
  ┌─────────────────────────────────┐
  │ SIM800L Module                  │
  │ (VCC & GND via Dual Y-Harness)  │
  └────────────────┬────────────────┘
                   │
                   ▼ (Common GND Tie)
  ┌─────────────────────────────────┐
  │ Arduino GND Pin                 │
  └─────────────────────────────────┘
```
