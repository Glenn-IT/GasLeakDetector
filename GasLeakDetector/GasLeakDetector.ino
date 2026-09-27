/*
  ============================================================
  GAS LEAK DETECTOR with SMS ALERT (SIM900A / SIM800L)
  ============================================================
  Board   : Arduino Uno
  Sensors : MQ-6 #1 (Area 1) -> A0
            MQ-6 #2 (Area 2) -> A1
  Display : LCD 16x2 with I2C backpack (SDA->A4, SCL->A5)
  Alarm   : LED (D3), Speaker/Buzzer 4ohm 3W via transistor (D4)
  GSM     : SIM900A (SIM TX -> D8, SIM RX -> D7 through divider)
            *** SMS ENABLED (see ENABLE_SMS below) ***
  Battery : 100k/100k voltage divider -> A2

  LIBRARIES NEEDED (Arduino IDE > Library Manager):
    "LiquidCrystal I2C" by Frank de Brabander
    SoftwareSerial (built in, used when ENABLE_SMS is 1)

  BEFORE UPLOADING:
    1. Check LCD_ADDRESS - most modules are 0x27, some are 0x3F.
       Run an I2C scanner sketch if the screen stays blank/blue.
    2. Let the MQ-6 sensors pre-heat 20-30 s on first power up; the
       sketch does a warm-up countdown automatically.
    3. Watch the Serial Monitor in clean air, then set
       GAS_THRESHOLD about 150-200 counts above that baseline.
    4. Fill in ALERT_NUMBERS array below in international format (+63...).
    5. Alarm triggers if EITHER sensor (Area 1 or Area 2) exceeds
       GAS_THRESHOLD (OR logic — more sensitive coverage).
  ============================================================
*/

// ---- set to 1 to enable SIM900A SMS alert functionality ----
#define ENABLE_SMS 1

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#if ENABLE_SMS
  #include <SoftwareSerial.h>
#endif

// ---------------- USER SETTINGS ----------------
// Add up to 5 (or more) recipient phone numbers in international format (+63...)
const char *ALERT_NUMBERS[] = {
  // "+639169751409",  // Disabled: Recipient 1
  "+639606619688",   // Recipient 2
  "+639554097301",   // Recipient 3
};
const byte NUM_RECIPIENTS = sizeof(ALERT_NUMBERS) / sizeof(ALERT_NUMBERS[0]);

#define GAS_THRESHOLD    150             // raw ADC 0-1023 (updated from 400 to 150)
#define GAS_HYSTERESIS   25              // clears alarm below 125 (150 - 25)
#define LCD_ADDRESS      0x27            // try 0x3F if blank
const unsigned long SMS_COOLDOWN = 300000UL; // 5 min between SMS alert batches
const unsigned long WARMUP_MS    = 20000UL;  // MQ-2 pre-heat

// Battery Monitoring Settings (5-minute stable evaluation)
#define SYSTEM_VCC_VOLTS            5.00     // Regulated 5.0V rail from Buck Converter
const unsigned long BATTERY_UPDATE_INTERVAL = 300000UL; // 5 minutes (300,000 ms) stable display cycle
const unsigned long BATTERY_SAMPLE_INTERVAL = 2000UL;   // Sample battery every 2 seconds in background
// -----------------------------------------------

// ---------------- PIN MAP ----------------
const uint8_t PIN_MQ6_1   = A0;  // MQ-6 Sensor #1 AO (Area 1)
const uint8_t PIN_MQ6_2   = A1;  // MQ-6 Sensor #2 AO (Area 2)
const uint8_t PIN_BATTERY = A2;  // Battery 100k/100k voltage divider sense
const uint8_t PIN_LED     = 3;   // LED + leg (via 220R)
const uint8_t PIN_SPKR    = 4;   // Speaker + leg (via transistor)
const uint8_t PIN_SIM_TX  = 8;   // Arduino RX  <- SIM900A TXD
const uint8_t PIN_SIM_RX  = 7;   // Arduino TX  -> SIM900A RXD
// LCD uses A4 (SDA) and A5 (SCL) automatically
// -----------------------------------------

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
#if ENABLE_SMS
  SoftwareSerial sim900(PIN_SIM_TX, PIN_SIM_RX); // (RX, TX)
#endif

bool          alarmActive   = false;
unsigned long lastSmsTime   = 0;
bool          smsEverSent   = false;
unsigned long lastBlink     = 0;
bool          blinkState    = false;
unsigned long lastLcdUpdate = 0;
unsigned long lastSerialLog = 0;

// Battery stability tracking variables
int           displayedBatteryPercent = 100;
unsigned long lastBatteryUpdateTime   = 0;
unsigned long lastBatterySampleTime   = 0;
long          batteryAdcSum           = 0;
int           batterySampleCount      = 0;

// ---------- forward declarations ----------
void showSplash();
void warmUpSensor();
void runAlarm(int gas1, int gas2);
void serviceAlarm();
void delayWithAlarm(unsigned long ms);
void clearAlarm();
int  readBatteryRawADC();
int  calculateBatteryPercentFromADC(int adcVal);
void serviceBatteryMonitor();
int  getBatteryPercent();
void updateLcd(int gas1, int gas2);
#if ENABLE_SMS
  bool sendATCommand(const char *cmd, const char *expected, unsigned long timeoutMs);
  void initSim900();
  bool sendSms(const char *msg, const char *targetNumber);
  void sendSmsToAll(const char *msg);
#endif

// ============================================================
void setup() {
  pinMode(PIN_LED,  OUTPUT);
  pinMode(PIN_SPKR, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  noTone(PIN_SPKR);

  Serial.begin(9600);
#if ENABLE_SMS
  sim900.begin(9600);
#endif

  lcd.init();
  lcd.backlight();

  showSplash();
  warmUpSensor();
#if ENABLE_SMS
  initSim900();
#endif

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1000);
  lcd.clear();
}

// ============================================================
void loop() {
  int gas1 = analogRead(PIN_MQ6_1);  // Area 1
  int gas2 = analogRead(PIN_MQ6_2);  // Area 2

  // OR logic: alarm if EITHER sensor exceeds the threshold.
  // Hysteresis: trip at GAS_THRESHOLD, clear only when BOTH sensors
  // fall below GAS_THRESHOLD - GAS_HYSTERESIS.
  bool eitherHigh  = (gas1 >= GAS_THRESHOLD) || (gas2 >= GAS_THRESHOLD);
  bool bothCleared = (gas1 < GAS_THRESHOLD - GAS_HYSTERESIS) &&
                     (gas2 < GAS_THRESHOLD - GAS_HYSTERESIS);

  if (eitherHigh) {
    runAlarm(gas1, gas2);
  } else if (bothCleared) {
    clearAlarm();
  }

  serviceBatteryMonitor();
  updateLcd(gas1, gas2);

  // steady stream of readings for threshold tuning
  if (millis() - lastSerialLog >= 1000) {
    lastSerialLog = millis();
    unsigned long elapsed = millis() - lastBatteryUpdateTime;
    unsigned long remainSec = (elapsed < BATTERY_UPDATE_INTERVAL) ? ((BATTERY_UPDATE_INTERVAL - elapsed) / 1000) : 0;

    Serial.print(F("A1:"));
    Serial.print(gas1);
    Serial.print(F("  A2:"));
    Serial.print(gas2);
    Serial.print(F("  limit:"));
    Serial.print(GAS_THRESHOLD);
    Serial.print(F("  Bat:"));
    Serial.print(getBatteryPercent());
    Serial.print(F("% (next in:"));
    Serial.print(remainSec);
    Serial.print(F("s)"));
    Serial.println(alarmActive ? F("  [ALARM]") : F("  [safe]"));
  }

#if ENABLE_SMS
  // echo clean printable SIM900A chatter to Serial Monitor
  while (sim900.available()) {
    char c = sim900.read();
    if (isprint(c) || c == '\r' || c == '\n') {
      Serial.write(c);
    }
  }
#endif

  delay(50);
}

// ============================================================
void showSplash() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" GAS  DETECTOR ");
  lcd.setCursor(0, 1);
  lcd.print("  with  SMS    ");
  // short confidence beep + LED flash
  digitalWrite(PIN_LED, HIGH);
  tone(PIN_SPKR, 1000, 150);
  delay(300);
  digitalWrite(PIN_LED, LOW);
  delay(1200);
}

void warmUpSensor() {
  unsigned long start = millis();
  unsigned long lastSample = 0;

  batteryAdcSum = 0;
  batterySampleCount = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Warming sensor..");

  while (millis() - start < WARMUP_MS) {
    int remain = (WARMUP_MS - (millis() - start)) / 1000;
    lcd.setCursor(0, 1);
    lcd.print("Please wait ");
    if (remain < 10) lcd.print(" ");
    lcd.print(remain);
    lcd.print("s ");

    // Take battery samples every 500ms during sensor warm-up
    if (millis() - lastSample >= 500) {
      lastSample = millis();
      batteryAdcSum += readBatteryRawADC();
      batterySampleCount++;
    }

    delay(100);
  }

  // Calculate immediate stable battery percent from warmup readings
  if (batterySampleCount > 0) {
    int avgAdc = (int)(batteryAdcSum / batterySampleCount);
    displayedBatteryPercent = calculateBatteryPercentFromADC(avgAdc);
  } else {
    displayedBatteryPercent = calculateBatteryPercentFromADC(readBatteryRawADC());
  }

  // Reset counters for the regular 5-minute runtime monitoring window
  batteryAdcSum = 0;
  batterySampleCount = 0;
  lastBatteryUpdateTime = millis();
  lastBatterySampleTime = millis();
}

// Service alarm (LED blink + dual-tone siren) continuously even inside delays/SMS tasks
void serviceAlarm() {
  if (!alarmActive) return;
  if (millis() - lastBlink >= 250) {
    lastBlink  = millis();
    blinkState = !blinkState;
    digitalWrite(PIN_LED, blinkState ? HIGH : LOW);
    if (blinkState) tone(PIN_SPKR, 1200);
    else            tone(PIN_SPKR, 800);
  }
}

// Alarm-aware delay function so alarm keeps running during waiting periods
void delayWithAlarm(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    serviceAlarm();
    delay(10);
  }
}

#if ENABLE_SMS
// Helper to send AT command and wait for expected response (with active alarm servicing)
bool sendATCommand(const char *cmd, const char *expected, unsigned long timeoutMs) {
  if (cmd != NULL && strlen(cmd) > 0) {
    while (sim900.available()) sim900.read();
    sim900.println(cmd);
  }

  unsigned long start = millis();
  String response = "";
  while (millis() - start < timeoutMs) {
    serviceAlarm();
    while (sim900.available()) {
      char c = sim900.read();
      response += c;
      if (expected != NULL && response.indexOf(expected) != -1) {
        return true;
      }
    }
  }
  return (expected == NULL);
}

void initSim900() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting GSM...");
  Serial.println(F("\n========================================"));
  Serial.println(F("[GSM] Initializing SIM900A..."));

  // SIM900A may boot up at 9600, 115200, 19200, or auto-baud.
  // Probe common baud rates and lock module to 9600 baud (AT+IPR=9600).
  const long bauds[] = {9600, 115200, 19200, 38400, 4800};
  bool connected = false;

  for (byte b = 0; b < 5; b++) {
    sim900.begin(bauds[b]);
    delay(100);
    for (byte i = 0; i < 3; i++) {
      sim900.println("AT");
      delay(200);
      if (sim900.available()) {
        String resp = "";
        while (sim900.available()) resp += (char)sim900.read();
        if (resp.indexOf("OK") != -1 || resp.indexOf("AT") != -1) {
          sim900.println("AT+IPR=9600");
          delay(200);
          sim900.println("AT&W");
          delay(200);
          connected = true;
          break;
        }
      }
    }
    if (connected) break;
  }

  sim900.begin(9600);

  if (!connected) {
    for (int i = 1; i <= 5; i++) {
      lcd.setCursor(0, 1);
      lcd.print("Connecting..");
      lcd.print(i);
      if (sendATCommand("AT", "OK", 1000)) {
        connected = true;
        break;
      }
      delay(300);
    }
  }

  if (!connected) {
    Serial.println(F("[GSM ERROR] SIM900A not responding! Check TX/RX wiring & power."));
    Serial.println(F("========================================\n"));
    lcd.setCursor(0, 1);
    lcd.print("GSM: No Response");
    delay(2000);
    return;
  }

  Serial.println(F("[GSM] Module Response: OK (9600 Baud)"));
  sendATCommand("ATE0", "OK", 1000); // Echo off

  // Check SIM card insertion / PIN ready
  if (sendATCommand("AT+CPIN?", "READY", 2000)) {
    Serial.println(F("[GSM] SIM Card: READY"));
  } else {
    Serial.println(F("[GSM WARN] SIM Card: NOT READY / LOCKED!"));
    lcd.setCursor(0, 1);
    lcd.print("GSM: Check SIM! ");
    delay(1500);
  }

  // Check signal strength
  if (sendATCommand("AT+CSQ", "OK", 1000)) {
    Serial.println(F("[GSM] Signal Strength: OK (+CSQ verified)"));
  }

  // Check network registration (Home or Roaming)
  if (sendATCommand("AT+CREG?", "0,1", 2000) || sendATCommand("AT+CREG?", "0,5", 2000)) {
    Serial.println(F("[GSM] Network Status: REGISTERED"));
  } else {
    Serial.println(F("[GSM WARN] Network Status: SEARCHING..."));
  }

  sendATCommand("AT+CMGF=1", "OK", 1000);         // SMS text mode
  sendATCommand("AT+CNMI=2,2,0,0,0", "OK", 1000); // Route incoming SMS

  lcd.setCursor(0, 1);
  lcd.print("GSM Ready!      ");
  Serial.println(F("[GSM] Setup Complete!"));
  Serial.println(F("========================================\n"));
  delay(1000);
}
#endif  // ENABLE_SMS

// ------------------------------------------------------------
void runAlarm(int gas1, int gas2) {
  alarmActive = true;
  serviceAlarm();

#if ENABLE_SMS
  // send SMS batch to all recipients once, then respect the cooldown
  if (!smsEverSent || (millis() - lastSmsTime >= SMS_COOLDOWN)) {
    char msg[160];
    bool area1 = (gas1 >= GAS_THRESHOLD);
    bool area2 = (gas2 >= GAS_THRESHOLD);

    if (area1 && area2) {
      snprintf(msg, sizeof(msg),
               "ALERT! Gas leak detected in BOTH areas. "
               "Area1 level: %d, Area2 level: %d (limit %d). Check immediately!",
               gas1, gas2, GAS_THRESHOLD);
    } else if (area1) {
      snprintf(msg, sizeof(msg),
               "ALERT! Gas leak detected in AREA 1. "
               "Area1 level: %d (limit %d). Area2: %d (safe). Check area 1 now!",
               gas1, GAS_THRESHOLD, gas2);
    } else {
      snprintf(msg, sizeof(msg),
               "ALERT! Gas leak detected in AREA 2. "
               "Area2 level: %d (limit %d). Area1: %d (safe). Check area 2 now!",
               gas2, GAS_THRESHOLD, gas1);
    }
    sendSmsToAll(msg);
    lastSmsTime = millis();
    smsEverSent = true;
  }
#else
  (void)gas1;
  (void)gas2;
#endif
}

void clearAlarm() {
  if (alarmActive) {
    alarmActive = false;
    digitalWrite(PIN_LED, LOW);
    noTone(PIN_SPKR);
    // NOTE: smsEverSent is deliberately NOT reset here. The cooldown is what
    // re-arms the SMS. Clearing it would let a reading bouncing around the
    // threshold fire a message on every single dip.
  }
}

// ------------------------------------------------------------
#if ENABLE_SMS
void sendSmsToAll(const char *msg) {
  Serial.print(F("[GSM] Starting SMS dispatch to "));
  Serial.print(NUM_RECIPIENTS);
  Serial.println(F(" recipient(s)..."));

  for (byte i = 0; i < NUM_RECIPIENTS; i++) {
    lcd.setCursor(0, 1);
    lcd.print("SMS ");
    lcd.print(i + 1);
    lcd.print("/");
    lcd.print(NUM_RECIPIENTS);
    lcd.print(" Sending..");

    sendSms(msg, ALERT_NUMBERS[i]);

    if (i < NUM_RECIPIENTS - 1) {
      delayWithAlarm(2000); // 2 second pause between recipients for carrier network stability
    }
  }
}

bool sendSms(const char *msg, const char *targetNumber) {
  Serial.print(F("[GSM] Sending SMS to "));
  Serial.println(targetNumber);
  Serial.print(F("[GSM] Msg: "));
  Serial.println(msg);

  // Ensure text mode with full OK confirmation
  if (!sendATCommand("AT+CMGF=1", "OK", 1500)) {
    Serial.println(F("[GSM WARN] AT+CMGF=1 non-OK response, retrying..."));
    sim900.println("AT+CMGF=1");
    delayWithAlarm(300);
  }

  // Clear any residual characters in buffer before sending destination number
  while (sim900.available()) sim900.read();

  // Send destination number command
  sim900.print("AT+CMGS=\"");
  sim900.print(targetNumber);
  sim900.println("\"");

  // Wait for prompt '>' (up to 5 seconds)
  if (!sendATCommand(NULL, ">", 5000)) {
    Serial.println(F("[GSM ERROR] Did not receive '>' prompt from SIM900A. SMS failed."));
    lcd.setCursor(0, 1);
    lcd.print("SMS Failed!     ");
    delayWithAlarm(1500);
    return false;
  }

  // Send text message payload + CTRL+Z (ASCII 26)
  sim900.print(msg);
  delayWithAlarm(300);
  sim900.write(26);

  // Wait for confirmation (+CMGS: ... or OK)
  if (sendATCommand(NULL, "+CMGS:", 10000) || sendATCommand(NULL, "OK", 5000)) {
    Serial.println(F("[GSM SUCCESS] SMS sent successfully."));
    lcd.setCursor(0, 1);
    lcd.print("SMS Sent!       ");
    delayWithAlarm(1000);
    return true;
  } else {
    Serial.println(F("[GSM WARN] SMS command finished (no confirmation token)."));
    lcd.setCursor(0, 1);
    lcd.print("SMS Sent?       ");
    delayWithAlarm(1000);
    return false;
  }
}
#endif  // ENABLE_SMS

// ------------------------------------------------------------
// Read raw battery ADC with dummy read to settle high-impedance (50k) divider
int readBatteryRawADC() {
  analogRead(PIN_BATTERY);       // Dummy read to switch MUX and charge ADC capacitor
  delayMicroseconds(250);

  long sum = 0;
  for (byte i = 0; i < 16; i++) {
    sum += analogRead(PIN_BATTERY);
    delayMicroseconds(100);
  }
  return (int)(sum / 16);
}

// Convert raw ADC reading into battery percentage (6.4V = 0%, 8.4V = 100%)
int calculateBatteryPercentFromADC(int adcVal) {
  // Divider ratio: 100k / (100k + 100k) = 0.5 (multiply by 2.0)
  float voltage = ((float)adcVal * SYSTEM_VCC_VOLTS / 1023.0) * 2.0;
  int percent = map((int)(voltage * 100), 640, 840, 0, 100);
  return constrain(percent, 0, 100);
}

// Background monitor: accumulates noise-free readings and updates every 5 minutes
void serviceBatteryMonitor() {
  unsigned long now = millis();

  // Take background sample every 2 seconds (skip during active alarm to ignore siren/GSM dip)
  if (!alarmActive && (now - lastBatterySampleTime >= BATTERY_SAMPLE_INTERVAL)) {
    lastBatterySampleTime = now;
    batteryAdcSum += readBatteryRawADC();
    batterySampleCount++;
  }

  // Every 5 minutes (300 seconds), evaluate the stable battery percentage
  if (now - lastBatteryUpdateTime >= BATTERY_UPDATE_INTERVAL) {
    if (batterySampleCount > 0) {
      int avgAdc = (int)(batteryAdcSum / batterySampleCount);
      displayedBatteryPercent = calculateBatteryPercentFromADC(avgAdc);
    }
    batteryAdcSum = 0;
    batterySampleCount = 0;
    lastBatteryUpdateTime = now;
  }
}

int getBatteryPercent() {
  return displayedBatteryPercent;
}

// ------------------------------------------------------------
void updateLcd(int gas1, int gas2) {
  if (millis() - lastLcdUpdate < 300) return;
  lastLcdUpdate = millis();

  int bat = getBatteryPercent();

  if (alarmActive) {
    // Line 0: Which area(s) triggered
    lcd.setCursor(0, 0);
    bool a1 = (gas1 >= GAS_THRESHOLD);
    bool a2 = (gas2 >= GAS_THRESHOLD);
    if (a1 && a2) lcd.print("!! BOTH AREAS !!");
    else if (a1)  lcd.print("!! AREA 1 GAS !!");
    else           lcd.print("!! AREA 2 GAS !!");

    // Line 1: Raw levels
    char line1[17];
    snprintf(line1, sizeof(line1), "A1:%-3d A2:%-3d  ", gas1, gas2);
    lcd.setCursor(0, 1);
    lcd.print(line1);
  } else {
    // Line 0: "A1:--- A2:--- " with battery %
    char line0[17];
    snprintf(line0, sizeof(line0), "A1:%-3d A2:%-3d  ", gas1, gas2);
    lcd.setCursor(0, 0);
    lcd.print(line0);

    // Line 1: Battery + safe status
    char line1[17];
    snprintf(line1, sizeof(line1), "Bat:%3d%% SAFE   ", bat);
    lcd.setCursor(0, 1);
    lcd.print(line1);
  }
}
