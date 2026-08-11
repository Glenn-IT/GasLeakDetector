/*
  ============================================================
  GAS LEAK DETECTOR with SMS ALERT (SIM800L / SIM900A)
  ============================================================
  Board   : Arduino Uno
  Sensor  : MQ-2  (Analog Out -> A0)
  Display : LCD 16x2 with I2C backpack (SDA->A4, SCL->A5)
  Alarm   : LED (D3), Speaker/Buzzer 4ohm 3W via transistor (D4)
  GSM     : SIM800L (SIM TX -> D8, SIM RX -> D7 through divider)
            *** SMS ENABLED (see ENABLE_SMS below) ***

  LIBRARIES NEEDED (Arduino IDE > Library Manager):
    "LiquidCrystal I2C" by Frank de Brabander
    SoftwareSerial (built in, used when ENABLE_SMS is 1)

  BEFORE UPLOADING:
    1. Check LCD_ADDRESS - most modules are 0x27, some are 0x3F.
       Run an I2C scanner sketch if the screen stays blank/blue.
    2. Let the MQ-2 pre-heat 20-30 s on first power up; the
       sketch does a warm-up countdown automatically.
    3. Watch the Serial Monitor in clean air, then set
       GAS_THRESHOLD about 150-200 counts above that baseline.
    4. Fill in ALERT_NUMBER below in international format (+63...).
  ============================================================
*/

// ---- set to 1 to enable SIM800L SMS alert functionality ----
#define ENABLE_SMS 1

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#if ENABLE_SMS
  #include <SoftwareSerial.h>
#endif

// ---------------- USER SETTINGS ----------------
#define ALERT_NUMBER   "+639242074903"  // <-- CHANGE THIS (only used if ENABLE_SMS)
#define GAS_THRESHOLD  400              // raw ADC 0-1023, raise if false alarms
#define GAS_HYSTERESIS 40               // must drop this far below to clear
#define LCD_ADDRESS    0x27             // try 0x3F if blank
const unsigned long SMS_COOLDOWN = 300000UL; // 5 min between SMS
const unsigned long WARMUP_MS    = 20000UL;  // MQ-2 pre-heat
// -----------------------------------------------

// ---------------- PIN MAP ----------------
const uint8_t PIN_MQ2     = A0;  // MQ-2 AO
const uint8_t PIN_BATTERY = A1;  // Battery 100k/100k voltage divider sense
const uint8_t PIN_LED     = 3;   // LED + leg (via 220R)
const uint8_t PIN_SPKR    = 4;   // Speaker + leg (via transistor)
const uint8_t PIN_SIM_TX  = 8;   // Arduino RX  <- SIM800L TXD
const uint8_t PIN_SIM_RX  = 7;   // Arduino TX  -> SIM800L RXD
// LCD uses A4 (SDA) and A5 (SCL) automatically
// -----------------------------------------

LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
#if ENABLE_SMS
  SoftwareSerial sim800(PIN_SIM_TX, PIN_SIM_RX); // (RX, TX)
#endif

bool          alarmActive   = false;
unsigned long lastSmsTime   = 0;
bool          smsEverSent   = false;
unsigned long lastBlink     = 0;
bool          blinkState    = false;
unsigned long lastLcdUpdate = 0;
unsigned long lastSerialLog = 0;

// ---------- forward declarations ----------
void showSplash();
void warmUpSensor();
void runAlarm(int gasValue);
void clearAlarm();
int  getBatteryPercent();
void updateLcd(int gasValue);
#if ENABLE_SMS
  bool sendATCommand(const char *cmd, const char *expected, unsigned long timeoutMs);
  void initSim800();
  void sendSms(const char *msg);
#endif

// ============================================================
void setup() {
  pinMode(PIN_LED,  OUTPUT);
  pinMode(PIN_SPKR, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  noTone(PIN_SPKR);

  Serial.begin(9600);
#if ENABLE_SMS
  sim800.begin(9600);
#endif

  lcd.init();
  lcd.backlight();

  showSplash();
  warmUpSensor();
#if ENABLE_SMS
  initSim800();
#endif

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1000);
  lcd.clear();
}

// ============================================================
void loop() {
  int gasValue = analogRead(PIN_MQ2);

  // Hysteresis: trip at the threshold, but don't clear until the reading
  // falls a good margin below it. Stops a value hovering right at the
  // limit from chattering the siren on and off.
  if (gasValue >= GAS_THRESHOLD) {
    runAlarm(gasValue);
  } else if (gasValue < GAS_THRESHOLD - GAS_HYSTERESIS) {
    clearAlarm();
  }

  updateLcd(gasValue);

  // steady stream of readings for threshold tuning
  if (millis() - lastSerialLog >= 1000) {
    lastSerialLog = millis();
    Serial.print(F("Gas: "));
    Serial.print(gasValue);
    Serial.print(F("  limit: "));
    Serial.print(GAS_THRESHOLD);
    Serial.print(F("  Bat: "));
    Serial.print(getBatteryPercent());
    Serial.print(F("%"));
    Serial.println(alarmActive ? F("  [ALARM]") : F("  [safe]"));
  }

#if ENABLE_SMS
  // echo clean printable SIM800L chatter to Serial Monitor
  while (sim800.available()) {
    char c = sim800.read();
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
    delay(200);
  }
}

#if ENABLE_SMS
// Helper to send AT command and wait for expected response (with timeout)
bool sendATCommand(const char *cmd, const char *expected, unsigned long timeoutMs) {
  if (cmd != NULL && strlen(cmd) > 0) {
    while (sim800.available()) sim800.read();
    sim800.println(cmd);
  }

  unsigned long start = millis();
  String response = "";
  while (millis() - start < timeoutMs) {
    while (sim800.available()) {
      char c = sim800.read();
      response += c;
      if (expected != NULL && response.indexOf(expected) != -1) {
        return true;
      }
    }
  }
  return (expected == NULL);
}

void initSim800() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting GSM...");
  Serial.println(F("\n========================================"));
  Serial.println(F("[GSM] Initializing SIM800L..."));

  // SIM800L may boot up at 9600, 115200, 19200, or auto-baud.
  // Probe common baud rates and lock module to 9600 baud (AT+IPR=9600).
  const long bauds[] = {9600, 115200, 19200, 38400, 4800};
  bool connected = false;

  for (byte b = 0; b < 5; b++) {
    sim800.begin(bauds[b]);
    delay(100);
    for (byte i = 0; i < 3; i++) {
      sim800.println("AT");
      delay(200);
      if (sim800.available()) {
        String resp = "";
        while (sim800.available()) resp += (char)sim800.read();
        if (resp.indexOf("OK") != -1 || resp.indexOf("AT") != -1) {
          sim800.println("AT+IPR=9600");
          delay(200);
          sim800.println("AT&W");
          delay(200);
          connected = true;
          break;
        }
      }
    }
    if (connected) break;
  }

  sim800.begin(9600);

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
    Serial.println(F("[GSM ERROR] SIM800L not responding! Check TX/RX wiring & power."));
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
void runAlarm(int gasValue) {
  alarmActive = true;

  // blinking LED + pulsing siren, non-blocking
  if (millis() - lastBlink >= 250) {
    lastBlink  = millis();
    blinkState = !blinkState;
    digitalWrite(PIN_LED, blinkState ? HIGH : LOW);
    if (blinkState) tone(PIN_SPKR, 1200);
    else            tone(PIN_SPKR, 800);
  }

#if ENABLE_SMS
  // send SMS once, then respect the cooldown
  if (!smsEverSent || (millis() - lastSmsTime >= SMS_COOLDOWN)) {
    char msg[90];
    snprintf(msg, sizeof(msg),
             "ALERT! Gas leak detected. Sensor level: %d (limit %d). Check the area now.",
             gasValue, GAS_THRESHOLD);
    noTone(PIN_SPKR);   // tone() interrupts corrupt SoftwareSerial timing
    sendSms(msg);
    lastSmsTime = millis();
    smsEverSent = true;
  }
#else
  (void)gasValue;
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
void sendSms(const char *msg) {
  Serial.print(F("[GSM] Sending SMS to "));
  Serial.println(ALERT_NUMBER);
  Serial.print(F("[GSM] Msg: "));
  Serial.println(msg);

  lcd.setCursor(0, 1);
  lcd.print("Sending SMS...  ");

  // Ensure text mode with full OK confirmation
  if (!sendATCommand("AT+CMGF=1", "OK", 1500)) {
    Serial.println(F("[GSM WARN] AT+CMGF=1 non-OK response, retrying..."));
    sim800.println("AT+CMGF=1");
    delay(300);
  }

  // Clear any residual characters in buffer before sending destination number
  while (sim800.available()) sim800.read();

  // Send destination number command
  sim800.print("AT+CMGS=\"");
  sim800.print(ALERT_NUMBER);
  sim800.println("\"");

  // Wait for prompt '>' (up to 5 seconds)
  if (!sendATCommand(NULL, ">", 5000)) {
    Serial.println(F("[GSM ERROR] Did not receive '>' prompt from SIM800L. SMS failed."));
    lcd.setCursor(0, 1);
    lcd.print("SMS Failed!     ");
    delay(1500);
    return;
  }

  // Send text message payload + CTRL+Z (ASCII 26)
  sim800.print(msg);
  delay(300);
  sim800.write(26);

  // Wait for confirmation (+CMGS: ... or OK)
  if (sendATCommand(NULL, "+CMGS:", 10000) || sendATCommand(NULL, "OK", 5000)) {
    Serial.println(F("[GSM SUCCESS] SMS sent successfully."));
    lcd.setCursor(0, 1);
    lcd.print("SMS Sent!       ");
  } else {
    Serial.println(F("[GSM WARN] SMS command finished (no confirmation token)."));
    lcd.setCursor(0, 1);
    lcd.print("SMS Sent?       ");
  }
  delay(1500);
}
#endif  // ENABLE_SMS

// ------------------------------------------------------------
int getBatteryPercent() {
  int rawADC = analogRead(PIN_BATTERY);
  // Voltage divider (100k / 100k): Multiply by 2.0 to get raw battery voltage (3.2V - 4.2V)
  float voltage = (rawADC * 5.0 / 1023.0) * 2.0;

  // Map 3.2V (0%) to 4.2V (100%)
  int percent = map((int)(voltage * 100), 320, 420, 0, 100);
  return constrain(percent, 0, 100);
}

// ------------------------------------------------------------
void updateLcd(int gasValue) {
  if (millis() - lastLcdUpdate < 300) return;
  lastLcdUpdate = millis();

  int bat = getBatteryPercent();

  // Line 0: "Gas:180   B:95%" (16 chars)
  lcd.setCursor(0, 0);
  lcd.print("Gas:");
  lcd.print(gasValue);
  if (gasValue < 1000) lcd.print(" ");
  lcd.print("   B:");
  if (bat < 100) lcd.print(" ");
  lcd.print(bat);
  lcd.print("%");

  // Line 1: Status
  lcd.setCursor(0, 1);
  if (alarmActive) lcd.print("!! GAS LEAK !!  ");
  else             lcd.print("Status: SAFE    ");
}
