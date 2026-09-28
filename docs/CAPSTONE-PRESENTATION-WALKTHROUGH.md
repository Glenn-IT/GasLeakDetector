# Dual-Zone Gas Leak Detector — Capstone Defense Presentation Walkthrough & Panelist Demonstration Guide
<!-- System: Dual-Zone LPG & Gas Leak Detector with Multi-Recipient SMS Alert, Dual-Tone Siren & Battery Telemetry -->
<!-- Architecture: Microcontroller-Based Embedded IoT Safety System (Arduino Uno ATmega328P + SIM900A/SIM800L + Dual MQ-6 Sensors) -->
<!-- Target Audience: Capstone Defense Panelists, Technical Advisers, Engineering Deans, and Evaluators -->

---

## 🧭 Executive Summary & Timing Strategy

| Phase | Section | Recommended Duration | Primary Interface / Artifact |
| :--- | :--- | :--- | :--- |
| **Phase 1** | Project Rationale, Public Safety Hazard & Institutional Problem Statement | 1.5 mins | Title Slide / Physical Rig Demonstration Bench |
| **Phase 2** | System Technical Architecture, Pin Topology & Embedded Hardware Overview | 1.5 mins | [GasLeakDetector.ino](file:///C:/xampp/htdocs/GasLeakDetector/GasLeakDetector/GasLeakDetector.ino) & [Diagram-Wiring-Visual.html](file:///C:/xampp/htdocs/GasLeakDetector/Diagram-Wiring-Visual.html) |
| **Phase 3** | High-Power Star Topology Power Architecture & 2A GSM RF Burst Protection | 1.0 min | [2S_18650_Buck_Setup/Instructions.md](file:///C:/xampp/htdocs/GasLeakDetector/2S_18650_Buck_Setup/Instructions.md) & [Diagram-Wiring.html](file:///C:/xampp/htdocs/GasLeakDetector/Diagram-Wiring.html) |
| **Phase 4** | Precision Resistor Divider & Dynamic 2S Battery Telemetry Algorithm | 1.0 min | [Diagram-Resistors.html](file:///C:/xampp/htdocs/GasLeakDetector/Diagram-Resistors.html) & Firmware `serviceBatteryMonitor()` |
| **Phase 5** | Dual-Zone MQ Sensing Engine, Sensor Chemistry & 20s Warm-Up Sequence | 1.0 min | LCD 16x2 Display / [GasLeakDetector.ino](file:///C:/xampp/htdocs/GasLeakDetector/GasLeakDetector/GasLeakDetector.ino) `warmUpSensor()` |
| **Phase 6** | Non-Chattering Alarm State Machine with Built-in Hysteresis Deadband | 1.0 min | Serial Monitor Telemetry / Firmware `loop()` Logic |
| **Phase 7** | Multi-Tier Audio-Visual Emergency Siren & Transistor Driver Circuit | 1.0 min | Red Alarm LED (D3) & 4Ω 3W Speaker via 2N2222 Driver (D4) |
| **Phase 8** | Real-Time 16x2 I2C Liquid Crystal Display (LCD) Telemetry Interface | 1.0 min | Physical 16x2 LCD Panel (`A1:nnn A2:nnn` / `Bat:nnn% SAFE`) |
| **Phase 9** | Cellular GSM Subsystem Boot Sequence & AT Command Handshake Pipeline | 1.0 min | Serial Monitor GSM Logs (`AT+CPIN?`, `AT+CSQ`, `AT+CREG?`) |
| **Phase 10** | Multi-Recipient Emergency SMS Broadcast Dispatch Pipeline | 1.5 mins | Target Recipient Smartphones / Firmware `sendSmsToAll()` |
| **Phase 11** | Emergency SMS Rate-Limiting & Anti-Spam Cooldown Engine | 0.5 min | Firmware `SMS_COOLDOWN` (300,000 ms Logic) |
| **Phase 12** | Non-Blocking Alarm Servicing During Synchronous Delays (`delayWithAlarm`) | 1.0 min | Firmware `serviceAlarm()` & `delayWithAlarm()` |
| **Phase 13** | Standalone GSM Diagnostic Suite & Interactive Hardware Documentation | 0.5 min | [GSM_Test/GSM_Test.ino](file:///C:/xampp/htdocs/GasLeakDetector/GSM_Test/GSM_Test.ino) & [Checklist.html](file:///C:/xampp/htdocs/GasLeakDetector/Checklist.html) |
| **Phase 14** | Controlled Live Demonstration: Single-Zone Gas Induction & SMS Reception | 2.0 mins | Unlit Butane Lighter Induction Test / Live Phone Alert Arrival |
| **Phase 15** | Controlled Live Demonstration: Recovery, Hysteresis & System Re-Arming | 1.0 min | Gas Dispersion / Automatic Return to `SAFE` State |
| **Phase 16** | Real-World Deployment Feasibility, Conclusion & Transition to Panel Q&A | 0.5 min | Project Roadmap & Concluding Summary |
| **Total** | **Full System Defense Presentation** | **~17.0 mins** | — |

---

## 🛠️ Pre-Defense Staging & Hardware Rig Setup

Before starting the defense presentation, prepare your demonstration workstation and hardware bench:

1. **Hardware Bench Preparation**:
   * **Power Verification**: Ensure the **2S 18650 Li-ion Battery Pack** is fully charged (**~8.2V to 8.4V** measured at `IN+` / `IN-`).
   * **Buck Step-Down Output**: Verify with a digital multimeter that the Buck Converter `OUT+` is calibrated to **5.00V &plusmn; 0.05V**.
   * **Common Ground Continuity**: Confirm that Buck `OUT-`, Arduino `GND`, GSM `GND`, Sensor `GND`, and LCD `GND` are solidly tied together on the breadboard negative rail.
   * **Decoupling & Diode**: Confirm the **1000 µF 16V electrolytic capacitor** is positioned across the SIM module supply rails (or the **1N4007 diode** is correctly oriented in series on SIM800L `VCC`).
2. **Arduino IDE & Serial Telemetry**:
   * Open the Arduino IDE on the presentation laptop connected to the Arduino Uno via USB.
   * Open the **Serial Monitor** set to **9600 Baud** with **"Both NL & CR"**.
   * Verify clean live telemetry streaming once per second: `A1:nnn  A2:nnn  limit:150  Bat:100% (next in:300s)  [safe]`.
3. **GSM Cellular SIM Card Preparation**:
   * Verify the SIM card installed in the SIM900A / SIM800L module has active prepaid balance (airtime load) and cellular coverage.
   * Verify that the network status LED on the GSM module blinks **slowly (every 3 seconds)** indicating successful registration to the cellular carrier network.
   * Confirm the panelist or demonstration phone number is programmed inside `ALERT_NUMBERS[]` in international format (`+639...`).
4. **Controlled Gas Source**:
   * Have a standard **unlit pocket butane lighter** ready on the demonstration tray.
   * *Safety Protocol*: Do **NOT** ignite any open flame. Only depress the thumb lever for 1–2 seconds to release a controlled stream of butane vapor directly over the sensor grill.
5. **Interactive Documentation Screens (Browser Tabs Ready)**:
   * **Tab 1**: [Diagram-Wiring-Visual.html](file:///C:/xampp/htdocs/GasLeakDetector/Diagram-Wiring-Visual.html) (Photorealistic hardware layout)
   * **Tab 2**: [Diagram-Wiring.html](file:///C:/xampp/htdocs/GasLeakDetector/Diagram-Wiring.html) (Detailed electronic schematic & connection matrix)
   * **Tab 3**: [Checklist.html](file:///C:/xampp/htdocs/GasLeakDetector/Checklist.html) (8-Stage verification protocol)
   * **Tab 4**: [SIM800L_Troubleshooting_Guide.md](file:///C:/xampp/htdocs/GasLeakDetector/SIM800L_Troubleshooting_Guide.md) (Engineering electrical post-mortem)

---

### 👥 System Configuration & Demonstration Numbers

| Configuration Parameter | Hardcoded Value | Description | Purpose in Defense |
| :--- | :--- | :--- | :--- |
| **Target Recipient 1** | `+639606619688` | Designated Primary Emergency Phone (Demonstrator Phone) | Receives immediate SMS with localized zone breakdown |
| **Target Recipient 2** | `+639554097301` | Designated Secondary Emergency Phone (Panelist Phone) | Demonstrates multi-party broadcast delivery |
| **Gas Threshold (`GAS_THRESHOLD`)** | `150` raw ADC | Dynamic trip point (0–1023 scale) | Trips alarm instantly when butane concentration exceeds clean air |
| **Gas Hysteresis (`GAS_HYSTERESIS`)** | `25` raw ADC | Deactivation deadband (Clears only below `125`) | Prevents alarm stuttering/chattering during gas dispersion |
| **Sensor Pre-Heat Countdown** | `20,000 ms` (20s) | Sensor element heating & baseline convergence | Visual LCD splash countdown during boot |
| **SMS Rate-Limit Cooldown** | `300,000 ms` (5 mins) | Minimum spacing between repeated alert broadcasts | Prevents SIM credit depletion and carrier anti-spam bans |
| **Battery Sampling Rate** | Every `2,000 ms` | Background multi-sample ADC acquisition | Gathers noise-free readings while ignoring transient siren loads |
| **Battery LCD Refresh Rate** | Every `300,000 ms` (5 mins) | Stable LCD battery percentage update cycle | Eliminates display flickering caused by instantaneous voltage fluctuations |

---

### 🔌 Physical Pin Assignment & Hardware Interfacing Matrix

| Subsystem | Hardware Component | Board / Pin | Arduino Uno Pin | Interface Type | Electrical Specification |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Sensing Zone 1** | MQ-6 Gas Sensor #1 | AO (Analog Out) | **Pin A0** | Analog Input (0–5V) | 0–1023 ADC; detects LPG, Isobutane, Propane in Kitchen/Tank Area |
| **Sensing Zone 2** | MQ-6 Gas Sensor #2 | AO (Analog Out) | **Pin A1** | Analog Input (0–5V) | 0–1023 ADC; detects LPG in Secondary Enclosure / Piping Route |
| **Battery Sense** | 2S Li-ion Battery Divider | Center Junction | **Pin A2** | Analog Input (0–5V) | 100kΩ / 100kΩ voltage divider scaling 8.4V max down to 4.2V |
| **Visual Alarm** | High-Intensity Red LED | Anode (+) via 220Ω | **Digital Pin D3** | Digital Output (PWM) | Strobe flashing at 4 Hz (250 ms toggle interval) |
| **Audio Siren** | 4Ω 3W Speaker / Piezo | Base via 1kΩ / Pin (+) | **Digital Pin D4** | Digital Output (Tone) | Dual-tone siren (800 Hz & 1200 Hz) via 2N2222 NPN Transistor |
| **Cellular GSM** | SIM900A / SIM800L | Pin 5VT (Module TX) | **Digital Pin D8** | SoftwareSerial RX | Direct TTL serial input from GSM to Arduino |
| **Cellular GSM** | SIM900A / SIM800L | Pin 5VR (Module RX) | **Digital Pin D7** | SoftwareSerial TX | Stepped down via 1kΩ / 2kΩ resistive divider or 5V-tolerant input |
| **Telemetry LCD** | 16x2 Character LCD | SDA (Serial Data) | **Analog Pin A4** | I2C Bus Data | Standard I2C communication protocol (Address `0x27` / `0x3F`) |
| **Telemetry LCD** | 16x2 Character LCD | SCL (Serial Clock) | **Analog Pin A5** | I2C Bus Clock | Standard I2C communication protocol |
| **Power Source** | 2S 18650 Battery Pack | Battery Positive (+) | **Buck IN+** | Raw DC Power | 7.4V nominal (6.4V discharged – 8.4V fully charged) |
| **Voltage Regulator**| LM2596 / MP1584 Buck | Output Positive (+) | **5V Power Rail** | Regulated DC Rail | 5.0V regulated supply feeding Arduino, GSM, LCD, and sensors |
| **System Ground** | Common Negative Rail | Battery Negative (-) | **Arduino GND** | Common Ground | Common return path for power, ADC sensing, and serial bus |

---

## 🎬 Step-by-Step Presentation Script (From First to Last)

---

### Step 1: Project Rationale, Public Safety Hazard & Institutional Problem Statement
* **Screen / Bench Display:** Title Slide / Physical Demonstration Bench showcasing the complete standalone system rig.
* **Estimated Time:** 1.5 minutes
* **Screen Action:** Stand before the panelists with the hardware prototype mounted on the demonstration enclosure or staging breadboard.
* **🗣️ Verbal Script:**
  > *"Good morning, honorable members of the panel, advisers, and distinguished guests. Today, we present our capstone engineering project: the **Dual-Zone Gas Leak Detector with Multi-Recipient SMS Alert, Dual-Tone Siren, and 2S Battery Telemetry**.
  >
  > *Liquefied Petroleum Gas, or LPG, is an indispensable fuel source in residential kitchens, commercial establishments, dormitories, and university food laboratories across the Philippines. However, because LPG is heavier than air, undetected gas leaks settle in low, unventilated pockets. When gas accumulates, even a tiny electrical spark from a light switch or refrigerator compressor can trigger catastrophic explosions and devastating residential fires.
  >
  > *Conventional commercial gas detectors present two severe limitations:
  > 1. **Single-Zone Vulnerability:** Standard consumer detectors cover only one physical point, leaving adjacent piping, regulator manifolds, or multi-room kitchen layouts unmonitored.
  > 2. **Localized Warning Isolation:** Traditional alarms emit only a localized buzzer. If a leak occurs while the occupants are asleep, at work, or off-campus, no one is alerted until an explosion or major fire has already occurred.
  >
  > *Our project resolves these vulnerabilities by introducing a dual-zone sensing architecture that monitors two independent hazard areas simultaneously, coupled with an autonomous GSM cellular broadcast engine that immediately dispatches emergency SMS alerts with exact zone coordinates to multiple designated authorities—even during power grid blackouts."*

---

### Step 2: System Technical Architecture, Pin Topology & Embedded Hardware Overview
* **Screen / Bench Display:** [Diagram-Wiring-Visual.html](file:///C:/xampp/htdocs/GasLeakDetector/Diagram-Wiring-Visual.html) or [GasLeakDetector.ino](file:///C:/xampp/htdocs/GasLeakDetector/GasLeakDetector/GasLeakDetector.ino)
* **Estimated Time:** 1.5 minutes
* **Screen Action:** Point out the primary modules on the hardware rig: Arduino Uno, dual MQ-6 sensors, 16x2 I2C LCD, SIM900A GSM transceiver, 2S battery pack, Buck converter, and alarm transducers.
* **🗣️ Verbal Script:**
  > *"To ensure maximum operational reliability, rapid response, and cost-effectiveness, the system is engineered around a dedicated embedded hardware stack:
  >
  > 1. **Core Microcontroller:** The ATmega328P microcontroller running at 16 MHz on the Arduino Uno platform. It continuously samples analog sensing telemetry, executes our state machine, drives the I2C display bus, and coordinates AT commands with the cellular transceiver.
  > 2. **Dual-Zone Sensing Engine:** Two independent MQ-6 gas sensors deployed on analog pins A0 and A1. While MQ-2 sensors are broad-spectrum, the MQ-6 sensor is specifically chemically calibrated for high sensitivity to LPG, propane, and butane, while exhibiting low cross-sensitivity to airborne alcohol and natural cooking vapors.
  > 3. **Cellular Broadcast Transceiver:** A SIM900A / SIM800L GSM engine operating over hardware/software serial on pins D8 and D7, operating autonomously without reliance on local Wi-Fi or broadband networks.
  > 4. **Visual & Acoustic Alert Peripherals:** Pin D3 drives a high-intensity red emergency strobe, while Pin D4 drives a dual-tone 800 Hz / 1200 Hz acoustic siren through a discrete NPN transistor amplifier.
  > 5. **Local Telemetry Display:** A 16x2 character Liquid Crystal Display integrated via a PCF8574 I2C adapter on pins A4 and A5, reducing wiring overhead to just two signal conductors."*

---

### Step 3: High-Power Star Topology Power Architecture & 2A GSM RF Burst Protection
* **Screen / Bench Display:** [2S_18650_Buck_Setup/Instructions.md](file:///C:/xampp/htdocs/GasLeakDetector/2S_18650_Buck_Setup/Instructions.md) & [Diagram-Wiring.html](file:///C:/xampp/htdocs/GasLeakDetector/Diagram-Wiring.html)
* **Estimated Time:** 1.0 minute
* **Screen Action:** Point to the 2S battery pack, the buck step-down converter, and the heavy-gauge star wiring harness feeding the GSM module.
* **🗣️ Verbal Script:**
  > *"One of the most critical engineering challenges in cellular IoT development is electrical power stability.
  >
  > *During 2G cellular network handshakes and SMS transmission, the GSM radio draws transient peak current spikes of up to **2.0 Amperes**. If an engineer attempts to power the GSM module directly from the Arduino's 5V onboard regulator or through thin daisy-chained jumper wires, the supply voltage sags below 3.8V, triggering an instantaneous brownout reset on the microcontroller or throwing the GSM module into an infinite reboot loop.
  >
  > *We engineered a **Star Topology Power Distribution Architecture**:
  > * We utilize a 2S Li-ion battery pack delivering **7.4V nominal (8.4V max)** stepped down via a high-efficiency DC-DC Buck converter calibrated precisely to **5.0V**.
  > * From the buck converter's output terminals, we split the power into two isolated branches: **Branch 1** utilizes heavy-gauge dual conductors dedicated solely to the GSM module with a bulk 1000 µF low-ESR electrolytic reservoir capacitor.
  > * **Branch 2** powers the Arduino microcontroller, sensing circuits, and LCD panel.
  > * All ground reference planes are unified on a common bus, completely eliminating ground loop noise and brownout restarts."*

---

### Step 4: Precision Resistor Divider & Dynamic 2S Battery Telemetry Algorithm
* **Screen / Bench Display:** [Diagram-Resistors.html](file:///C:/xampp/htdocs/GasLeakDetector/Diagram-Resistors.html) & Highlight `serviceBatteryMonitor()` in [GasLeakDetector.ino](file:///C:/xampp/htdocs/GasLeakDetector/GasLeakDetector/GasLeakDetector.ino#L515-L562)
* **Estimated Time:** 1.0 minute
* **Screen Action:** Point to the resistor divider assembly on the breadboard and the LCD bottom row showing `Bat: 95% SAFE`.
* **🗣️ Verbal Script:**
  > *"To guarantee autonomy during utility power outages, the system features real-time onboard battery telemetry:
  >
  > *The raw 2S battery pack voltage fluctuates between **6.4V (discharged)** and **8.4V (fully charged)**. Because the ATmega328P analog pins cannot tolerate voltages exceeding 5.0V, we route the raw battery terminal through a precision **100kΩ / 100kΩ resistive voltage divider** to Analog Pin A2, creating a safe 1:2 scaling ratio where 8.4V is stepped down to 4.2V.
  >
  > *In software, raw analog battery reads often suffer from high impedance noise and momentary dips caused by siren activation. Our firmware implements three protective algorithms:
  > 1. **ADC Settling Dummy Read:** We execute a dummy read followed by a 250-microsecond delay to allow the internal ADC sample-and-hold capacitor to charge through the high-impedance divider.
  > 2. **Multi-Sample Averaging:** We accumulate 16 consecutive ADC samples to filter out transient electrical noise.
  > 3. **Alarm Load-Freeze Suppression:** Battery sampling is paused while `alarmActive` is true, ensuring that the heavy current draw of the audio siren and GSM module never skews the displayed battery state."*

---

### Step 5: Dual-Zone MQ Sensing Engine, Sensor Chemistry & 20s Warm-Up Sequence
* **Screen / Bench Display:** Trigger manual reset button on Arduino Uno -> Watch LCD splash and countdown.
* **Estimated Time:** 1.0 minute
* **Screen Action:** Press the reset button on the Arduino Uno. Panelists observe the LCD displaying: `Warming sensor.. Please wait 20s`.
* **🗣️ Verbal Script:**
  > *"When the system is first energized, semiconductor gas sensors require thermal stabilization. The tin-dioxide (SnO2) sensing element inside the MQ-6 sensor must reach its internal operating temperature of approximately 200°C to achieve optimal chemical adsorption.
  >
  > *Notice that upon boot, our firmware executes an automated **20-second warm-up countdown sequence** displayed directly on the LCD. During this window, false alarm trips are locked out, and the microcontroller accumulates background battery telemetry.
  >
  > *Once the countdown concludes, the LCD announces `'System Ready'` and transitions to active continuous monitoring mode."*

---

### Step 6: Non-Chattering Alarm State Machine with Built-in Hysteresis Deadband
* **Screen / Bench Display:** Open Serial Monitor streaming live telemetry: `A1:042  A2:038  limit:150  Bat:100% [safe]`
* **Estimated Time:** 1.0 minute
* **Screen Action:** Show the steady telemetry stream on the Serial Monitor and explain the mathematical threshold logic.
* **🗣️ Verbal Script:**
  > *"In our core execution loop, the microcontroller samples Analog Pin A0 for Area 1 and Analog Pin A1 for Area 2.
  >
  > *We enforce a strict **OR-Logic Hazard Detection Policy**:
  > ```cpp
  > bool eitherHigh = (gas1 >= GAS_THRESHOLD) || (gas2 >= GAS_THRESHOLD);
  > ```
  > *If **either** Area 1 or Area 2 exceeds the preset threshold of **150 ADC counts**, the system trips into emergency state.
  >
  > *Critically, to prevent the common engineering flaw known as **relay or alarm chattering**—where readings hovering right around the threshold cause the siren and SMS triggers to violently toggle on and off—we implemented a **25-count Hysteresis Deadband**:
  > ```cpp
  > bool bothCleared = (gas1 < GAS_THRESHOLD - GAS_HYSTERESIS) && (gas2 < GAS_THRESHOLD - GAS_HYSTERESIS);
  > ```
  > *Once tripped at 150, the alarm will **NOT** clear until both sensors drop cleanly below **125 counts**, guaranteeing a rock-solid, oscillation-free state transition."*

---

### Step 7: Multi-Tier Audio-Visual Emergency Siren & Transistor Driver Circuit
* **Screen / Bench Display:** Point to High-Intensity Red LED (D3), 2N2222 Transistor, and 4Ω 3W Speaker (D4).
* **Estimated Time:** 1.0 minute
* **Screen Action:** Showcase the hardware connections on the breadboard and explain the driver circuitry.
* **🗣️ Verbal Script:**
  > *"For localized warning, the device integrates a multi-tier audio-visual warning subsystem:
  >
  > *On Digital Pin D3, an ultra-bright red LED pulses at **4 Hz (every 250 milliseconds)**, providing rapid visual orientation even through dense smoke or darkness.
  >
  > *On Digital Pin D4, the system generates an alternating **European-style dual-tone siren** switching between **800 Hz and 1200 Hz**.
  >
  > *From an electrical engineering standpoint, connecting a 4Ω 3W speaker directly to an Arduino pin is dangerous: according to Ohm's Law, 5V divided by 4Ω pulls 1.25 Amperes—far exceeding the 40 mA absolute maximum rating of the ATmega328P I/O pin! To protect the microcontroller, we drive the speaker through an **NPN bipolar junction transistor (2N2222 / BC547)** configured as a saturated low-side switch with a 1kΩ current-limiting base resistor."*

---

### Step 8: Real-Time 16x2 I2C Liquid Crystal Display (LCD) Telemetry Interface
* **Screen / Bench Display:** Physical 16x2 LCD Panel mounted on the test rig.
* **Estimated Time:** 1.0 minute
* **Screen Action:** Highlight the clean telemetry formatting displayed on the two rows of the LCD.
* **🗣️ Verbal Script:**
  > *"The 16x2 Liquid Crystal Display acts as the on-site command display for facility managers and residents:
  >
  > *During normal safe operation:
  > * **Line 1** displays real-time gas concentration levels: `A1: 045  A2: 039`
  > * **Line 2** presents battery health and status: `Bat: 98% SAFE`
  >
  > *When a hazard is detected:
  > * **Line 1** dynamically updates to identify the specific compromised hazard zone: displaying `!! AREA 1 GAS !!`, `!! AREA 2 GAS !!`, or `!! BOTH AREAS !!`.
  > * **Line 2** displays the exact numerical sensor readings so emergency responders can assess whether the leak is escalating or dissipating."*

---

### Step 9: Cellular GSM Subsystem Boot Sequence & AT Command Handshake Pipeline
* **Screen / Bench Display:** Serial Monitor showing GSM initialization output:
  ```text
  [GSM] Initializing SIM900A...
  [GSM] Module Response: OK (9600 Baud)
  [GSM] SIM Card: READY
  [GSM] Signal Strength: OK (+CSQ verified)
  [GSM] Network Status: REGISTERED
  ```
* **Estimated Time:** 1.0 minute
* **Screen Action:** Explain the auto-baud locking and AT command validation pipeline in [GasLeakDetector.ino#L296-L387](file:///C:/xampp/htdocs/GasLeakDetector/GasLeakDetector/GasLeakDetector.ino#L296-L387).
* **🗣️ Verbal Script:**
  > *"When the GSM subsystem initializes, our firmware executes a defensive boot handshake:
  >
  > 1. **Auto-Baud Rate Scanner:** Commercial SIM modules often boot at unknown speeds such as 115200 or 19200 baud. Our initialization routine systematically probes five standard baud rates, verifies communication, and locks the transceiver permanently to 9600 baud via `AT+IPR=9600` and `AT&W`.
  > 2. **Hardware & SIM Verification:** We issue `AT+CPIN?` to verify that the subscriber SIM card is inserted and unlocked.
  > 3. **RF Signal Quality:** We interrogate cellular signal strength with `AT+CSQ`.
  > 4. **Carrier Registration:** We verify network connectivity using `AT+CREG?` to confirm registration on the national cellular network (Globe, Smart, or DITO).
  > 5. **Text Mode Initialization:** We configure standard SMS text mode via `AT+CMGF=1` and enable direct routing with `AT+CNMI`."*

---

### Step 10: Multi-Recipient Emergency SMS Broadcast Dispatch Pipeline
* **Screen / Bench Display:** Highlight `sendSmsToAll()` in [GasLeakDetector.ino#L440-L512](file:///C:/xampp/htdocs/GasLeakDetector/GasLeakDetector/GasLeakDetector.ino#L440-L512).
* **Estimated Time:** 1.5 minutes
* **Screen Action:** Showcase the multi-recipient array and demonstrate how message payloads are dynamically constructed based on the breached zone.
* **🗣️ Verbal Script:**
  > *"Our primary communications innovation is the **Multi-Recipient Emergency Dispatch Pipeline**:
  >
  > *Conventional systems only send an alert to a single phone number. In a real-world emergency, if that individual is unavailable or their phone is on silent, the alert is lost.
  >
  > *Our firmware maintains an array of recipient phone numbers (`ALERT_NUMBERS[]`) in international format:
  > * Recipient 1: Resident / Homeowner (`+639606619688`)
  > * Recipient 2: Building Administrator / Dorm Matron (`+639554097301`)
  >
  > *When an alarm trips, the system inspects the sensor flags and constructs an intelligent context-aware message:
  > * If Area 1 is breached: `'ALERT! Gas leak detected in AREA 1. Area1 level: 312 (limit 150). Area2: 45 (safe). Check area 1 now!'`
  > * If both are breached: `'ALERT! Gas leak detected in BOTH areas. Area1 level: 320, Area2 level: 290 (limit 150). Check immediately!'`
  >
  > *The system loops through each recipient, sends the payload via AT commands, pauses 2 seconds between recipients to allow cellular network buffers to settle, and displays transmission progress on the LCD (`SMS 1/2 Sending..`)."*

---

### Step 11: Emergency SMS Rate-Limiting & Anti-Spam Cooldown Engine
* **Screen / Bench Display:** Code inspection of `SMS_COOLDOWN` in [GasLeakDetector.ino#L52](file:///C:/xampp/htdocs/GasLeakDetector/GasLeakDetector/GasLeakDetector.ino#L52) and [GasLeakDetector.ino#L397](file:///C:/xampp/htdocs/GasLeakDetector/GasLeakDetector/GasLeakDetector.ino#L397).
* **Estimated Time:** 0.5 minute
* **Screen Action:** Explain how the 5-minute cooldown timer operates without halting the local siren.
* **🗣️ Verbal Script:**
  > *"In an ongoing gas leak emergency, the sensor may remain above threshold for 20 or 30 minutes. If the system sent an SMS every loop cycle, the SIM card's prepaid airtime balance would be completely depleted within minutes, or telecommunication providers would flag the SIM for spam.
  >
  > *To prevent this, we implemented a non-blocking **5-Minute Anti-Spam Cooldown Engine** (`SMS_COOLDOWN = 300000UL`).
  >
  > *Once an SMS batch is dispatched, the timer locks further SMS transmissions for 5 minutes. However, the visual LED strobe and audio siren remain continuously active. If the gas leak persists past 5 minutes, a follow-up warning batch is automatically dispatched."*

---

### Step 12: Non-Blocking Alarm Servicing During Synchronous Delays (`delayWithAlarm`)
* **Screen / Bench Display:** Highlight `serviceAlarm()` and `delayWithAlarm()` in [GasLeakDetector.ino#L252-L271](file:///C:/xampp/htdocs/GasLeakDetector/GasLeakDetector/GasLeakDetector.ino#L252-L271).
* **Estimated Time:** 1.0 minute
* **Screen Action:** Point out how the microcontroller continues to toggle the alarm while waiting for GSM responses.
* **🗣️ Verbal Script:**
  > *"One notorious flaw in beginner Arduino projects is the use of standard `delay()` calls during GSM operations. Because cellular AT commands require 2 to 5 seconds of network waiting time, a standard `delay()` completely halts the CPU, freezing the siren and shutting down LED blinking during transmission!
  >
  > *To resolve this, we engineered an alarm-aware execution wrapper:
  > ```cpp
  > void delayWithAlarm(unsigned long ms) {
  >   unsigned long start = millis();
  >   while (millis() - start < ms) {
  >     serviceAlarm();
  >     delay(10);
  >   }
  > }
  > ```
  > *Whenever the firmware must wait for GSM network handshakes, it calls `serviceAlarm()` every 10 milliseconds. The red LED continues flashing at 4 Hz and the dual-tone siren continues wailing without a fraction of a second of interruption!"*

---

### Step 13: Standalone GSM Diagnostic Suite & Interactive Hardware Documentation
* **Screen / Bench Display:** Show [GSM_Test/GSM_Test.ino](file:///C:/xampp/htdocs/GasLeakDetector/GSM_Test/GSM_Test.ino) and open [Checklist.html](file:///C:/xampp/htdocs/GasLeakDetector/Checklist.html) in the browser.
* **Estimated Time:** 0.5 minute
* **Screen Action:** Briefly show the standalone diagnostic sketch and the 8-stage interactive HTML bring-up checklist.
* **🗣️ Verbal Script:**
  > *"To ensure verifiable engineering quality and rapid field diagnostics, our project repository includes:
  > 1. A standalone diagnostic tool, [`GSM_Test.ino`](file:///C:/xampp/htdocs/GasLeakDetector/GSM_Test/GSM_Test.ino), which independently verifies GSM module health, signal quality, and AT command routing.
  > 2. An interactive browser-based 8-Stage Bring-Up Checklist ([`Checklist.html`](file:///C:/xampp/htdocs/GasLeakDetector/Checklist.html)) covering power isolation, continuity tests, sensor baseline verification, and soak testing."*

---

### Step 14: Controlled Live Demonstration: Single-Zone Gas Induction & SMS Reception
* **Screen / Bench Display:** Physical Hardware Test Rig + Demonstration Smartphone held up for panelists.
* **Estimated Time:** 2.0 minutes
* **Screen Action:**
  1. Hold the unlit butane lighter approximately 2–3 cm from **MQ-6 Sensor #1 (Area 1)**.
  2. Depress the lever for **1.5 seconds** to release a small burst of butane vapor.
  3. **Observe Rig Actions**:
     * Sensor 1 reading spikes past the 150 threshold (e.g., reaching 280–400 on the Serial Monitor).
     * The Red LED immediately begins flashing at 4 Hz.
     * The speaker erupts into the alternating 800 Hz / 1200 Hz emergency siren.
     * LCD Line 1 instantly flips to: `!! AREA 1 GAS !!`.
     * LCD Line 2 displays: `SMS 1/2 Sending..`.
  4. Hold up the demonstration smartphone to show the incoming SMS alert arriving within 5–10 seconds.
  5. Read the SMS payload aloud to the panel:
     > *"ALERT! Gas leak detected in AREA 1. Area1 level: 342 (limit 150). Area2: 42 (safe). Check area 1 now!"*
* **🗣️ Verbal Script:**
  > *"Now, we will perform a live, controlled demonstration of single-zone gas detection.
  >
  > *I am holding an unlit butane lighter near MQ-6 Sensor #1, representing Area 1 (such as the gas cylinder regulator in a kitchen). I release a 1-second burst of butane...
  >
  > *[Siren wails, LED strobes, LCD switches to `!! AREA 1 GAS !!`]*
  >
  > *Notice the instantaneous reaction: the threshold of 150 was breached. Digital Pin D3 is strobing the visual LED, the transistor on D4 is driving the dual-tone emergency siren, and the LCD identifies the exact compromised zone: Area 1.
  >
  > *Looking at our demonstration smartphone... here is the incoming SMS notification!
  >
  > *Notice that the SMS text explicitly isolates the hazard: it informs the resident that **Area 1** has breached safe levels with an exact reading of 342 counts, while confirming that **Area 2** remains safe at 42 counts. This allows emergency personnel to know precisely where to intervene before entering the premises."*

---

### Step 15: Controlled Live Demonstration: Recovery, Hysteresis & System Re-Arming
* **Screen / Bench Display:** Hardware Test Rig clearing back to safe state.
* **Estimated Time:** 1.0 minute
* **Screen Action:**
  1. Move the butane lighter away from the sensor and gently fan the sensor with a notepad to accelerate gas dispersion.
  2. Point to the Serial Monitor showing the reading dropping: `220` -> `170` -> `140` -> `124`.
  3. Show that at `140` (which is below the 150 threshold), the alarm remains active.
  4. Show that once the reading falls below `125` (150 minus 25 hysteresis), the siren silences, the LED turns off, and the LCD reverts to:
     `A1: 052  A2: 041` / `Bat: 98% SAFE`.
* **🗣️ Verbal Script:**
  > *"Now observing the recovery phase:
  >
  > *As the butane vapor dissipates into ambient air, observe the Serial Monitor: the reading drops through 145. Notice that even though 145 is below the 150 threshold, the siren is **still sounding**!
  >
  > *This demonstrates our **25-count Hysteresis Deadband** in action. The alarm refuses to clear until the reading drops safely below 125 counts.
  >
  > *As the level drops below 125... the siren silences, the LED extinguishes, and the LCD smoothly returns to `'SAFE'`. The system has automatically re-armed itself for the next hazard without requiring manual human reset."*

---

### Step 16: Real-World Deployment Feasibility, Conclusion & Transition to Panel Q&A
* **Screen / Bench Display:** Concluding Slide / Architecture Diagram.
* **Estimated Time:** 0.5 minute
* **Screen Action:** Summarize the real-world impact and open the floor to panelists.
* **🗣️ Verbal Script:**
  > *"In summary, our Dual-Zone Gas Leak Detector delivers an autonomous, life-saving early warning system engineered with robust electrical design, brownout-proof star power topology, dual-zone chemical sensing, and multi-recipient cellular dispatch.
  >
  > *It operates completely off-grid on rechargeable 2S Li-ion batteries, safeguarding homes, boarding houses, and commercial kitchens even during catastrophic weather blackouts.
  >
  > *Thank you very much, honorable members of the panel. We are now eager and ready to answer your questions."*

---

## 🛡️ Capstone Defense Panelist Q&A Cheat Sheet

| # | Panelist Question | Recommended Authoritative Technical & Engineering Answer |
| :- | :--- | :--- |
| **Q1** | **Why choose the Arduino Uno (ATmega328P) rather than an ESP32 or Raspberry Pi with built-in Wi-Fi?** | *"While the ESP32 and Raspberry Pi are powerful, our system is designed for **critical life safety**. During real-world gas explosion hazards and structural fires, commercial electrical grids and home Wi-Fi routers frequently trip, lose power, or suffer connectivity loss. An ESP32 relying on local Wi-Fi or cloud MQTT brokers would fail to deliver alerts during a blackout. The ATmega328P paired with a dedicated cellular GSM engine operates 100% off-grid via our 2S battery pack, delivering emergency SMS messages over cellular towers that have independent auxiliary battery backup. Additionally, the ATmega328P consumes significantly less quiescent power, boots up in milliseconds, and avoids operating system crashes."* |
| **Q2** | **Why use two MQ-6 sensors instead of the widely popular MQ-2 gas sensor?** | *"The standard MQ-2 sensor is a broad-spectrum metal-oxide sensor sensitive to LPG, methane, hydrogen, cigarette smoke, and ethanol. In residential kitchens and dormitories, cooking alcohol vapors, fried oil aerosols, or boiling vinegar frequently trigger false alarms on an MQ-2. The **MQ-6 sensor is chemically optimized with a specific tin-dioxide (SnO2) doping formula** that exhibits maximum sensitivity to heavy hydrocarbon gases—specifically Isobutane and Propane (the exact chemical constituents of commercial LPG)—while showing very low sensitivity to natural cooking alcohols and atmospheric humidity. This drastically reduces false-positive alarms."* |
| **Q3** | **How does your power supply handle the 2A transmit spikes of the SIM900A without resetting the Arduino?** | *"GSM transceivers emit high-power RF transmission bursts that draw brief current spikes of up to **2.0 Amperes**. We resolved this through a three-tier electrical protection architecture: (1) **Star Power Topology:** Rather than daisy-chaining power from the Arduino, the high-current GSM module is fed directly from the output of an external 3A DC-DC Buck converter via dedicated heavy-gauge conductors. (2) **Capacitive Reservoir:** A **1000 µF 16V low-ESR electrolytic capacitor** is mounted directly across the GSM VCC and GND terminals, acting as a local charge reservoir that instantly supplies current during the RF burst without allowing the voltage to drop below the 3.8V GSM brownout threshold. (3) **Unified Common Ground:** All system grounds are tied together to prevent floating reference offsets that cause serial communication corruption."* |
| **Q4** | **Why is a transistor required on Pin D4 to drive the 4Ω 3W speaker?** | *"An Arduino Uno digital I/O pin is rated for a **maximum absolute current of 40 mA** (recommended 20 mA continuous). According to Ohm's Law ($I = V / R$), driving a 4Ω speaker directly from a 5V logic pin attempts to draw: $5\text{V} / 4\Omega = 1.25\text{ Amperes}$. Attempting to draw 1,250 mA directly from an ATmega328P pin would permanently burn out the microcontroller's internal output transistor silicon or trigger an instant brownout reset. We utilize an **NPN bipolar junction transistor (2N2222 or BC547)** configured as an open-collector low-side driver with a 1kΩ base current-limiting resistor, allowing a tiny 4 mA logic signal from Pin D4 to safely modulate the speaker's higher current draw."* |
| **Q5** | **How does the battery percentage calculation work, and why does it not fluctuate when the alarm sounds?** | *"The raw 2S battery voltage (6.4V discharged to 8.4V charged) is stepped down through a **100kΩ / 100kΩ resistive divider** to Analog Pin A2. Because the divider has high equivalent impedance (50kΩ Thevenin resistance), our firmware executes a **dummy ADC read** followed by a 250 µs delay to settle the internal sample-and-hold capacitor, followed by **16-sample averaging**. To prevent the heavy instantaneous current draw of the audio siren and GSM module from causing a false 'low battery' drop on the LCD, our firmware explicitly skips battery sampling during active alarms (`if (!alarmActive)`), and updates the LCD on a 5-minute low-pass window."* |
| **Q6** | **What is the purpose of the 25-count hysteresis in the alarm algorithm?** | *"In electronic sensing, gas concentration around the threshold boundary is inherently noisy due to air currents and thermal turbulence. Without hysteresis, if the threshold is 150 and the gas level fluctuates between 149 and 151, the system would rapidly cycle the siren on and off several times per second—a catastrophic phenomenon known as **relay chattering** or alarm flutter. Our hysteresis algorithm dictates that once tripped at 150, the alarm status **will not clear until readings on both sensors fall below 125 (150 &minus; 25)**. This 25-count deadband ensures clean, stable, and decisive alarm state transitions."* |
| **Q7** | **Why did you implement a 5-minute cooldown on the SMS alerts?** | *"A standard residential gas leak can take 15 to 45 minutes to safely dissipate. If our firmware dispatched an SMS on every execution loop or upon every transient reading dip, the SIM card's prepaid airtime balance would be exhausted within 60 seconds, or cellular carriers would blacklist the number for automated SMS flooding. The **5-minute cooldown (`SMS_COOLDOWN = 300000UL`)** guarantees that an emergency alert is broadcast immediately upon breach, while enforcing a disciplined anti-spam throttle that preserves cellular balance while the local audio-visual siren remains 100% active."* |
| **Q8** | **How do you ensure the buzzer and LED do not freeze while the microcontroller waits for AT commands from the GSM module?** | *"Standard Arduino AT command libraries rely on blocking delays (`delay(3000)`), which halt CPU execution and silence sirens during network operations. In our firmware, all waiting routines are routed through our custom **`delayWithAlarm(unsigned long ms)`** function and **`sendATCommand()`** polling loop. While waiting for cellular responses, the microcontroller continuously invokes **`serviceAlarm()`** every 10 milliseconds, toggling the 4 Hz LED strobe and alternating the 800 Hz / 1200 Hz siren tones without the slightest stutter."* |
| **Q9** | **Why did you include a 1kΩ / 2kΩ voltage divider on the SIM module RX pin (Pin D7)?** | *"The Arduino Uno operates at **5.0V TTL logic levels**. However, GSM modules like the SIM800L and certain SIM900A serial interfaces are engineered for **3.3V or 2.8V logic**. Feeding a direct 5.0V signal from Arduino Pin D7 into the GSM RX pin over prolonged periods will cause thermal degradation and electrical breakdown of the transceiver's baseband processor. The **1kΩ / 2kΩ resistive voltage divider** steps the 5.0V digital output down to a safe **3.33V logic level** ($5\text{V} \times \frac{2\text{k}\Omega}{1\text{k}\Omega + 2\text{k}\Omega} = 3.33\text{V}$), preserving hardware longevity."* |
| **Q10** | **What happens if the SIM card has no signal, no credit balance, or is removed?** | *"Our system follows the principle of **Graceful Degradation**. During boot, `initSim900()` validates SIM card readiness (`AT+CPIN?`) and network registration (`AT+CREG?`). If the GSM module fails to connect or register, the failure is logged to the Serial Monitor and displayed on the LCD (`GSM: Check SIM!`), but the system **does NOT halt**. The microcontroller continues normal monitoring: if a gas leak occurs, the visual strobe and audio siren will still trigger at full power to alert on-site residents."* |
| **Q11** | **How does the system differentiate between a leak in Area 1 versus Area 2?** | *"Each sensor is routed to a dedicated analog channel: Sensor 1 is on Pin A0, and Sensor 2 is on Pin A1. In software, each reading is evaluated independently against `GAS_THRESHOLD`. When the alarm trips, our logic sets boolean flags (`bool area1 = gas1 >= GAS_THRESHOLD; bool area2 = gas2 >= GAS_THRESHOLD;`). The LCD dynamically prints `!! AREA 1 GAS !!`, `!! AREA 2 GAS !!`, or `!! BOTH AREAS !!`, and the SMS dispatch function dynamically formats the outgoing message text to state exactly which area is breached and its respective numerical reading."* |
| **Q12** | **How scalable is this project for commercial buildings, dormitories, or industrial kitchens?** | *"The architecture is highly scalable. The dual-zone design can be expanded to multi-zone arrays by integrating an **analog multiplexer (such as the 16-channel CD74HC4067)**, allowing a single microcontroller to monitor up to 16 independent rooms or kitchen stations. Furthermore, the alarm output pin (D4) can easily drive a **5V Relay Module** to automatically trigger an **emergency solenoid gas shut-off valve** and activate an **explosion-proof exhaust ventilation fan**, turning passive detection into active hazard mitigation."* |

---

## 💡 Pro-Tips for Defense Day

1. **Dual Demonstration Setup (Physical Rig + Live Phone)**:
   * Position the hardware rig clearly on the demonstration table facing the panel members.
   * Place one demonstrator phone on the table in front of the panel with the ringtone volume set to **High**.
   * When you release the butane gas, invite the panel to observe the LCD flip, the siren fire, and watch the emergency SMS appear on the smartphone screen in real time. Panels love seeing physical and digital synchronization!
2. **Controlled Butane Induction Technique**:
   * Practice the gas release beforehand. You only need a **1 to 2-second gentle depression** of the lighter valve directly above the sensor mesh.
   * Do **NOT** shake the lighter or spray liquid fuel. Releasing pure vapor allows the sensor to recover quickly and cleanly within 30 to 45 seconds during Step 15.
3. **Verify Prepaid SIM Balance & Coverage 1 Hour Before Defense**:
   * Send a test SMS using [`GSM_Test.ino`](file:///C:/xampp/htdocs/GasLeakDetector/GSM_Test/GSM_Test.ino) or your mobile phone to ensure the SIM card has at least ₱20 to ₱50 in active regular load or unlimited SMS promo.
   * Confirm that the defense room has adequate cellular signal reception for the chosen carrier network.
4. **Have a Small Screwdriver Ready for LCD Contrast**:
   * Different room lighting and battery voltages can affect LCD contrast. Keep a small flathead screwdriver in your pocket to adjust the blue trimpot on the back of the I2C backpack if characters appear faint under bright fluorescent defense room lights.
5. **Keep the Serial Monitor Open on Laptop Screen**:
   * Keep the Arduino Serial Monitor running at 9600 baud projected on the secondary monitor or laptop. Showing the live, continuous stream of raw ADC numbers, battery percentage countdown, and GSM handshake tokens provides undeniable proof of engineering rigor to technical panelists.
6. **Emphasize Off-Grid Life Safety**:
   * When asked why this is better than a simple smart plug or smartphone app, emphasize **life safety and grid independence**: when a gas leak causes a fire or electrical blackout, this system continues protecting lives on dedicated battery reserves.
