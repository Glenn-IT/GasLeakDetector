# Gas Leak Detector — Detailed Wiring List

## 1. Power Supply System (2S 18650 Pack + Buck Converter)
* **2x 18650 Battery Pack (+ / 8.4V Max)** ──► Buck Converter **IN+** AND Battery Divider **R1 (100kΩ)**
* **2x 18650 Battery Pack (- / GND)** ──► Buck Converter **IN-** AND Breadboard **GND Rail**
* **Buck Converter OUT+ (5.0V Regulated)** ──► Breadboard **+5V Rail** AND **GSM Module VCC**
* **Buck Converter OUT- (GND)** ──► Breadboard **GND Rail** (Common System Ground)

## 2. Arduino Uno Power & Sensing Connections
* **Arduino 5V Pin** ──► Breadboard **+5V Rail** (Disconnect when plugged into USB)
* **Arduino GND Pin** ──► Breadboard **GND Rail**
* **Arduino Pin A1 (Battery % Sense)** ──► **Junction of 100kΩ / 47kΩ Resistor Divider**
  * *Divider Assembly*: Raw Battery (+) ──► [100kΩ Resistor] ──► **[Junction to A1]** ──► [47kΩ Resistor] ──► GND Rail

## 3. MQ-2 Gas Sensor
* **VCC** ──► Breadboard **+5V Rail**
* **GND** ──► Breadboard **GND Rail**
* **AO (Analog Out)** ──► Arduino Pin **A0**
* **DO (Digital Out)** ──► *Not Connected*

## 4. LCD 16x2 Display (I2C Adapter)
* **VCC** ──► Breadboard **+5V Rail**
* **GND** ──► Breadboard **GND Rail**
* **SDA** ──► Arduino Pin **A4**
* **SCL** ──► Arduino Pin **A5**

## 5. Alarm Peripherals (LED & Speaker)
* **LED Anode (+)** ──► 220Ω Resistor ──► Arduino Pin **D3**
* **LED Cathode (-)** ──► Breadboard **GND Rail**
* **Speaker (+)** ──► Arduino Pin **D4** (or transistor base circuit)
* **Speaker (-)** ──► Breadboard **GND Rail**

## 6. GSM SMS Module (SIM900A / SIM800L)
* **VCC** ──► Buck Converter **OUT+** *(For SIM800L max 4.4V: Add 1N4007 Diode in series, Anode to Buck 5V, Cathode/silver stripe to SIM VCC)*
* **GND** ──► Breadboard **GND Rail**
* **TXD** ──► Arduino Pin **D8**
* **RXD** ──► Arduino Pin **D7** *(through 1kΩ / 2kΩ voltage divider: D7 ──► 1kΩ ──► [GSM RXD] ──► 2kΩ ──► GND)*