# 2S 18650 Battery + 5V Buck Converter Setup Guide

This guide details the **Star Topology Power Distribution** method using **2x 18650 cells in series (7.4V nominal / 8.4V max)** and a **5V Buck Step-Down Converter**.

---

## 📐 Visual Wiring Diagram
An interactive dark-themed wiring diagram is available in this folder:
👉 Open [`Diagram-Wiring.html`](file:///C:/xampp/htdocs/GasLeakDetector/2S_18650_Buck_Setup/Diagram-Wiring.html) in your browser.

---

## ⚡ Why Star Wiring Works
During GSM SMS transmissions, the SIM module draws short **2A peak bursts**. If the SIM module and Arduino share the exact same thin jumper wires, the voltage drops, causing Arduino brown-outs or GSM resets.

**Star Wiring** solves this by running two separate wire branches directly from the Buck Converter Output:
* **Branch 1 (Dedicated Heavy Wires)**: Buck `OUT+` & `OUT-` ──► SIM Module `VCC` & `GND`
* **Branch 2 (System Power Rail)**: Buck `OUT+` & `OUT-` ──► Breadboard `+5V` & `-GND` Rails (Arduino, MQ-2, LCD)

---

## 🛠️ Step-by-Step Assembly Instructions

### Step 1: Calibrate Buck Converter Output to 5.0V
1. Connect the **2x 18650 Battery Pack (+ 7.4V)** to Buck **`IN+`** and **(&minus;)** to Buck **`IN-`**.
2. **Do NOT connect the Arduino or SIM module yet!**
3. Place multimeter probes on Buck **`OUT+`** and **`OUT-`**.
4. Turn the small brass screw on the blue potentiometer counter-clockwise/clockwise until your multimeter reads **exactly 5.0V** (or 5.1V).

### Step 2: Wire Branch 1 (GSM Module Direct Power)
1. Use **18 AWG to 22 AWG copper wire** (< 10 cm in length).
2. Connect Buck **`OUT+`** directly to SIM Module **`5V` / `VCC`**.
3. Connect Buck **`OUT-`** directly to SIM Module **`GND`**.

### Step 3: Wire Branch 2 (Breadboard Power Rail)
1. Run a second pair of wires from Buck **`OUT+`** to Breadboard **`+ 5V Rail`**.
2. Run a second pair of wires from Buck **`OUT-`** to Breadboard **`- GND Rail`**.
3. Connect Arduino **`5V`** pin to Breadboard **`+ 5V Rail`**.
4. Connect Arduino **`GND`** pin to Breadboard **`- GND Rail`**.

### Step 4: Wire Peripheral Signals
* **LCD 16x2 (I2C)**: `VCC` ──► `+5V Rail`, `GND` ──► `-GND Rail`, `SDA` ──► `A4`, `SCL` ──► `A5`.
* **MQ-2 Gas Sensor**: `VCC` ──► `+5V Rail`, `GND` ──► `-GND Rail`, `AO` ──► `A0`.
* **Alarm LED**: Anode (+) ──► `D3` (via 220Ω resistor), Cathode (-) ──► `-GND Rail`.
* **Alarm Speaker**: Positive (+) ──► `D4`, Negative (-) ──► `-GND Rail`.
* **GSM Serial**: SIM `TXD` ──► Arduino `D8`, SIM `RXD` ──► Arduino `D7` (via 1kΩ/2kΩ divider).

### Step 5: Wire Battery Percentage Sensing (Pin A1)
To monitor your 2S 18650 battery percentage on the LCD (`B:95%`):
1. Connect raw battery positive (**7.4V–8.4V**) to top of **100kΩ Resistor (R1)**.
2. Connect bottom of **47kΩ Resistor (R2)** to **`-GND Rail`**.
3. Connect the junction point between R1 and R2 to Arduino Pin **`A1`**.

---

## 📌 Pin Connection Reference Table

| Component | Component Pin | Connection Destination | Wire Type / Notes |
| :--- | :--- | :--- | :--- |
| **2x 18650 Battery Pack** | `+` (7.4V - 8.4V) | Buck Converter **`IN+`** & Divider R1 | Raw Battery Voltage |
| | `-` (GND) | Buck Converter **`IN-`** & Common GND | Common Ground |
| **Buck Step-Down Module** | `OUT+` (5.0V) | **Branch 1**: SIM VCC<br>**Branch 2**: Breadboard +5V Rail | Star Topology |
| | `OUT-` (GND) | **Branch 1**: SIM GND<br>**Branch 2**: Breadboard -GND Rail | Common Ground |
| **SIM Module** | `VCC / 5V` | Buck Converter `OUT+` (Branch 1) | Dedicated Heavy Wire |
| | `GND` | Buck Converter `OUT-` (Branch 1) | Dedicated Heavy Wire |
| | `TXD` | Arduino **`D8`** | Serial RX |
| | `RXD` | Arduino **`D7`** (via 1k/2k divider) | Serial TX |
| **MQ-2 Sensor** | `VCC` / `GND` / `AO` | `+5V Rail` / `-GND Rail` / Arduino **`A0`** | Analog Gas Sensor |
| **LCD 16x2 I2C** | `VCC` / `GND` / `SDA` / `SCL` | `+5V Rail` / `-GND Rail` / Arduino **`A4`** / **`A5`** | I2C Display |
| **Alarm LED** | Anode (+) / Cathode (-) | Arduino **`D3`** (via 220Ω) / `-GND Rail` | Visual Alarm |
| **Speaker / Buzzer** | (+) / (-) | Arduino **`D4`** (via Transistor) / `-GND Rail` | Audio Siren |
| **Arduino Uno** | `5V` / `GND` | Breadboard `+5V Rail` / `-GND Rail` | Power from Buck |
| | `A1` | 100kΩ / 47kΩ Divider Junction | Battery Percentage |
