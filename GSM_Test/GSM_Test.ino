/*
  ============================================================
  SIM900A (S2-1040V-Z096S) STANDALONE DIAGNOSTIC & SMS TEST SKETCH
  ============================================================
  Board   : Arduino Uno R3
  GSM     : SIM900A (S2-1040V-Z096S)
  Wiring  :
    - SIM900A Pin 1 (VCC5) ──► Buck Converter OUT+ (5V)
    - SIM900A Pin 2 (GND)  ──► Arduino GND (Common Ground)
    - SIM900A Pin 3 (5VT)  ──► Arduino D8 (SIM TX -> Arduino RX)
    - SIM900A Pin 4 (5VR)  ──► Arduino D7 (SIM RX <- Arduino TX)

  INSTRUCTIONS:
    1. Upload this sketch to Arduino Uno.
    2. Open Serial Monitor (Set to 9600 baud and "Both NL & CR").
    3. Type AT commands directly in Serial Monitor.
    4. To send a test SMS, type:
       SEND +639242074903 Hello this is a test SMS!
  ============================================================
*/

#include <SoftwareSerial.h>

// Default software serial setup: RX=D8 (connects to 5VT), TX=D7 (connects to 5VR)
SoftwareSerial sim(8, 7);

void sendSmsTest(String inputLine);

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  Serial.println(F("\n=================================================="));
  Serial.println(F("    SIM900A HARDWARE DIAGNOSTIC & SMS TEST        "));
  Serial.println(F("=================================================="));
  Serial.println(F("[INFO] Serial Monitor set to 9600 baud & 'Both NL & CR'"));
  Serial.println(F("[INFO] Starting baud rate scanner...\n"));

  bool found = false;
  long matchedBaud = 0;
  const long baudRates[] = {9600, 115200, 19200, 38400, 4800, 57600};

  // Test Standard Pin Wiring (D8=RX, D7=TX)
  for (byte i = 0; i < 6; i++) {
    long b = baudRates[i];
    sim.begin(b);
    delay(100);
    while (sim.available()) sim.read();
    sim.println("AT");
    delay(250);

    if (sim.available()) {
      String resp = "";
      while (sim.available()) resp += (char)sim.read();
      if (resp.indexOf("OK") != -1 || resp.indexOf("AT") != -1) {
        matchedBaud = b;
        found = true;
        break;
      }
    }
  }

  // Test Swapped Pin Wiring if needed
  if (!found) {
    SoftwareSerial simSwap(7, 8);
    for (byte i = 0; i < 6; i++) {
      long b = baudRates[i];
      simSwap.begin(b);
      delay(100);
      while (simSwap.available()) simSwap.read();
      simSwap.println("AT");
      delay(250);
      if (simSwap.available()) {
        String resp = "";
        while (simSwap.available()) resp += (char)simSwap.read();
        if (resp.indexOf("OK") != -1 || resp.indexOf("AT") != -1) {
          matchedBaud = b;
          found = true;
          break;
        }
      }
    }
  }

  if (found) {
    Serial.println(F("=================================================="));
    Serial.println(F(" 🎉 GSM MODULE VERIFIED & LOCKED AT 9600 BAUD!"));
    Serial.println(F("=================================================="));
    
    // Lock SIM900A to 9600 baud
    sim.begin(matchedBaud);
    sim.println("AT+IPR=9600");
    delay(200);
    sim.println("AT&W");
    delay(200);
    sim.begin(9600);

    Serial.println(F("\n[HOW TO SEND TEST SMS]:"));
    Serial.println(F("Type 'SEND <NUMBER> <MESSAGE>' in Serial Monitor. Example:"));
    Serial.println(F("  SEND +639242074903 Gas detector test message working!\n"));
    Serial.println(F("[PASSTHROUGH MODE READY] Type any AT command (e.g. 'AT', 'AT+CSQ'):"));
    Serial.println(F("--------------------------------------------------\n"));
  } else {
    Serial.println(F("❌ ERROR: NO RESPONSE FROM SIM900A. Check wiring."));
    sim.begin(9600);
  }
}

String inputBuffer = "";

void loop() {
  // Listen for user input from Serial Monitor
  while (Serial.available()) {
    char c = Serial.read();
    
    if (c == '\r' || c == '\n') {
      inputBuffer.trim();
      if (inputBuffer.length() > 0) {
        if (inputBuffer.startsWith("SEND ") || inputBuffer.startsWith("send ")) {
          sendSmsTest(inputBuffer);
        } else {
          // Send raw AT command to SIM900A
          sim.println(inputBuffer);
        }
        inputBuffer = "";
      }
    } else {
      inputBuffer += c;
    }
  }

  // Display raw responses from SIM900A on Serial Monitor
  while (sim.available()) {
    char c = sim.read();
    Serial.write(c);
  }
}

void sendSmsTest(String inputLine) {
  // Expected format: SEND +639242074903 Hello world!
  int firstSpace = inputLine.indexOf(' ');
  int secondSpace = inputLine.indexOf(' ', firstSpace + 1);

  if (firstSpace == -1 || secondSpace == -1) {
    Serial.println(F("\n[ERROR] Incorrect format! Use format: SEND +639XXXXXXXXX Your message here"));
    return;
  }

  String targetNumber = inputLine.substring(firstSpace + 1, secondSpace);
  String message = inputLine.substring(secondSpace + 1);

  Serial.println(F("\n=================================================="));
  Serial.print(F("[SMS TEST] Sending message to: "));
  Serial.println(targetNumber);
  Serial.print(F("[SMS TEST] Payload: "));
  Serial.println(message);

  // Set SMS Text Mode
  sim.println("AT+CMGF=1");
  delay(500);

  // Clear buffer
  while (sim.available()) sim.read();

  // Send destination number
  sim.print("AT+CMGS=\"");
  sim.print(targetNumber);
  sim.println("\"");
  delay(1000);

  // Send message text payload + CTRL+Z (26)
  sim.print(message);
  delay(300);
  sim.write(26);

  Serial.println(F("[SMS TEST] Waiting for network confirmation (+CMGS)..."));
  Serial.println(F("=================================================="));
}
