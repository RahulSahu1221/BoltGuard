// ===================================================================
// BOLTGUARD — SENSE CONTROLLER
// Stage 1: torque + angle pass/fail check per bolt
// Stage 2: post-install angle drift watch (loosening early-warning)
// ===================================================================

#include <LiquidCrystal.h>

// ---- LCD setup: change pin numbers to match your wiring ----
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); // RS, EN, D4, D5, D6, D7
// NOTE: if you also use pins 9,10,11 for LEDs/buzzer above, RENUMBER
// either the LCD pins or the LED/buzzer pins so nothing overlaps.
// Simple fix used in this guide: LCD uses pins 7,8,9,10,11,12 ONLY
// for LCD. Move Green LED to A3, Red LED to A4, Buzzer to A5 instead
// (analog pins can be used as plain digital pins on Uno).

// ---- Sensor pins ----
const int TORQUE_PIN = A0;
const int ANGLE_PIN  = A1;
const int DRIFT_PIN  = A2;

// ---- Station Controller communication pins ----
const int EN_BOLT1 = 2;
const int EN_BOLT2 = 3;
const int EN_BOLT3 = 4;
const int PASS1      = 5;
const int PASS2      = 6;
const int PASS3_PIN  = 13; // NOTE: not pin 7 — pin 7 is used by the LCD below
const int FAIL_PIN   = A3;
const int GREEN_LED  = A4;
const int RED_LED    = A5;
const int BUZZER_PIN = 12; // NOTE: not pin 0 — pin 0 is the Serial RX line, never reuse it

// ---- Pass windows (adjust these numbers to whatever feels realistic) ----
const int TORQUE_MIN = 300, TORQUE_MAX = 700;  // out of 0-1023 ADC range
const int ANGLE_MIN  = 300, ANGLE_MAX  = 700;

// ---- Stage 2 memory: baseline angle stored per bolt after PASS ----
int baselineAngle[3] = {0, 0, 0};
bool bolted[3] = {false, false, false};
const int DRIFT_THRESHOLD = 80; // ADC units — tune during testing

void setup() {
  Serial.begin(9600); // used to send alerts to Central Monitor

  pinMode(TORQUE_PIN, INPUT);
  pinMode(ANGLE_PIN, INPUT);
  pinMode(DRIFT_PIN, INPUT);

  pinMode(EN_BOLT1, INPUT);
  pinMode(EN_BOLT2, INPUT);
  pinMode(EN_BOLT3, INPUT);

  pinMode(PASS1, OUTPUT);
  pinMode(PASS2, OUTPUT);
  pinMode(PASS3_PIN, OUTPUT);
  pinMode(FAIL_PIN, OUTPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.begin(16, 2);
  lcd.print("BoltGuard Ready");
}

void loop() {

  // ---------------- STAGE 1: install-time check ----------------
  if (digitalRead(EN_BOLT1) == HIGH) {
    checkBolt(0, PASS1);
  } else if (digitalRead(EN_BOLT2) == HIGH) {
    checkBolt(1, PASS2);
  } else if (digitalRead(EN_BOLT3) == HIGH) {
    checkBolt(2, PASS3_PIN);
  } else {
    // ---------------- STAGE 2: watch mode ----------------
    // Runs only when no bolt station is currently active,
    // i.e. all bolts have already passed and line has advanced.
    watchForDrift();
  }

  delay(200); // simple polling delay, keeps things readable on LCD
}

// Reads torque + angle for the given bolt index (0,1,2),
// compares to the pass window, and pulses PASS or FAIL accordingly.
void checkBolt(int boltIndex, int passPin) {
  int torqueVal = analogRead(TORQUE_PIN);
  int angleVal  = analogRead(ANGLE_PIN);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bolt ");
  lcd.print(boltIndex + 1);
  lcd.print(" Checking");

  bool torqueOK = (torqueVal >= TORQUE_MIN && torqueVal <= TORQUE_MAX);
  bool angleOK  = (angleVal  >= ANGLE_MIN  && angleVal  <= ANGLE_MAX);

  if (torqueOK && angleOK) {
    // ---- PASS ----
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    lcd.setCursor(0, 1);
    lcd.print("PASS - OK");

    baselineAngle[boltIndex] = angleVal; // store for Stage 2 later
    bolted[boltIndex] = true;

    digitalWrite(passPin, HIGH);
    delay(300); // short pulse so the Station Controller sees it
    digitalWrite(passPin, LOW);
    digitalWrite(GREEN_LED, LOW);

  } else {
    // ---- FAIL ----
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("FAIL - RETRY");

    digitalWrite(FAIL_PIN, HIGH);
    delay(300);
    digitalWrite(FAIL_PIN, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// Stage 2: checks stored bolts for angular drift using the
// DRIFT_PIN potentiometer as a stand-in for real vibration.
// In this simulation, all bolted[] positions share the same
// drift knob for simplicity — turning it simulates "time passing".
void watchForDrift() {
  if (!bolted[0] && !bolted[1] && !bolted[2]) {
    return; // nothing installed yet, nothing to watch
  }

  int currentReading = analogRead(DRIFT_PIN);

  for (int i = 0; i < 3; i++) {
    if (bolted[i]) {
      int drift = abs(currentReading - baselineAngle[i]);
      if (drift > DRIFT_THRESHOLD) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ALERT Bolt ");
        lcd.print(i + 1);
        lcd.setCursor(0, 1);
        lcd.print("Loosening drift");

        // Send the alert to the Central Monitor over serial
        Serial.print("ALERT: Bolt ");
        Serial.print(i + 1);
        Serial.print(" drifted ");
        Serial.print(drift);
        Serial.println(" units - check joint");

        digitalWrite(RED_LED, HIGH);
        delay(500);
        digitalWrite(RED_LED, LOW);
      }
    }
  }
}
