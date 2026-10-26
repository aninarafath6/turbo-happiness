/*
 * Wet / Dry waste sorter
 * ----------------------
 * Hardware: Arduino Nano, SG90 micro servo, HW-080 soil-moisture sensor
 *           (2-prong probe + LM393 comparator board), jumper wires.
 *
 * How it works
 *   1. Waste is dropped on a small tray. The moisture probe sits flush in the
 *      tray floor so the waste rests on both prongs.
 *   2. A trigger (jumper wire on D2 touched to GND, or 's' in Serial Monitor)
 *      starts a sort cycle.
 *   3. The sensor is powered up, sampled several times, and the median reading
 *      is compared with WET_THRESHOLD.
 *   4. The servo tilts the tray left (wet bin) or right (dry bin), waits for
 *      the item to slide off, then returns to centre.
 *
 * Wiring
 *   Sensor board  VCC -> D7     (powered only while reading; slows corrosion)
 *                 GND -> GND
 *                 A0  -> A0
 *                 D0  -> not used
 *   Probe (2 pins) -> sensor board 2-pin header (black/white jumper pair)
 *   Servo  brown  -> GND
 *          red    -> 5V
 *          orange -> D9
 *   Trigger: one jumper wire in D2, touch its free end to GND to sort.
 *
 * Calibration
 *   Open Serial Monitor at 115200 and send 'r'. The sensor is read every
 *   second. Note the value with nothing on the tray (~1000+), with dry waste
 *   (~900-1023) and with wet waste (often < 600). Set WET_THRESHOLD between
 *   the dry and wet values. Send 'r' again to stop.
 */

#include <Servo.h>

// ---------- pins ----------
const uint8_t PIN_SENSOR_VCC = 7;   // powers the LM393 board
const uint8_t PIN_SENSOR_A0  = A0;  // analog moisture reading
const uint8_t PIN_SERVO      = 9;
const uint8_t PIN_TRIGGER    = 2;   // INPUT_PULLUP, pull LOW to sort

// ---------- tuning ----------
// Raw ADC value: higher = drier (open circuit reads ~1023).
// Readings BELOW this are treated as wet.  Adjust after calibration.
int WET_THRESHOLD = 700;

// Servo positions (degrees). Swap WET/DRY if your tray tilts the wrong way.
const uint8_t POS_CENTER = 90;
const uint8_t POS_WET    = 20;
const uint8_t POS_DRY    = 160;

const uint16_t SENSOR_WARMUP_MS = 100;   // let the board settle after power-up
const uint8_t  SAMPLES          = 9;     // odd number, median is taken
const uint16_t DUMP_HOLD_MS     = 1200;  // how long the tray stays tilted
const uint16_t DEBOUNCE_MS      = 50;

Servo tray;
bool liveReadMode = false;

// ---------- helpers ----------
int readMoistureRaw() {
  digitalWrite(PIN_SENSOR_VCC, HIGH);
  delay(SENSOR_WARMUP_MS);

  int s[SAMPLES];
  for (uint8_t i = 0; i < SAMPLES; i++) {
    s[i] = analogRead(PIN_SENSOR_A0);
    delay(20);
  }
  digitalWrite(PIN_SENSOR_VCC, LOW);

  // insertion sort, then take the median
  for (uint8_t i = 1; i < SAMPLES; i++) {
    int v = s[i];
    int8_t j = i - 1;
    while (j >= 0 && s[j] > v) { s[j + 1] = s[j]; j--; }
    s[j + 1] = v;
  }
  return s[SAMPLES / 2];
}

void moveServoSlowly(uint8_t from, uint8_t to) {
  int8_t step = (to > from) ? 1 : -1;
  for (int p = from; p != to; p += step) {
    tray.write(p);
    delay(8);
  }
  tray.write(to);
}

void sortOnce() {
  int raw = readMoistureRaw();
  bool wet = raw < WET_THRESHOLD;

  Serial.print(F("raw="));
  Serial.print(raw);
  Serial.print(F("  -> "));
  Serial.println(wet ? F("WET") : F("DRY"));

  uint8_t target = wet ? POS_WET : POS_DRY;
  moveServoSlowly(POS_CENTER, target);
  delay(DUMP_HOLD_MS);
  moveServoSlowly(target, POS_CENTER);
}

bool triggerPressed() {
  if (digitalRead(PIN_TRIGGER) != LOW) return false;
  delay(DEBOUNCE_MS);
  if (digitalRead(PIN_TRIGGER) != LOW) return false;
  while (digitalRead(PIN_TRIGGER) == LOW) { /* wait for release */ }
  delay(DEBOUNCE_MS);
  return true;
}

void handleSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 's': case 'S':
        sortOnce();
        break;
      case 'r': case 'R':
        liveReadMode = !liveReadMode;
        Serial.println(liveReadMode ? F("live read ON") : F("live read OFF"));
        break;
      case 't': case 'T': {
        // set threshold at runtime, e.g. "t650"
        long v = Serial.parseInt();
        if (v > 0 && v < 1024) {
          WET_THRESHOLD = v;
          Serial.print(F("threshold = "));
          Serial.println(WET_THRESHOLD);
        }
        break;
      }
      case 'w': case 'W':  // manual servo test
        moveServoSlowly(POS_CENTER, POS_WET); delay(500); moveServoSlowly(POS_WET, POS_CENTER);
        break;
      case 'd': case 'D':
        moveServoSlowly(POS_CENTER, POS_DRY); delay(500); moveServoSlowly(POS_DRY, POS_CENTER);
        break;
      default:
        break;
    }
  }
}

// ---------- main ----------
void setup() {
  Serial.begin(115200);
  pinMode(PIN_SENSOR_VCC, OUTPUT);
  digitalWrite(PIN_SENSOR_VCC, LOW);
  pinMode(PIN_TRIGGER, INPUT_PULLUP);

  tray.attach(PIN_SERVO);
  tray.write(POS_CENTER);

  Serial.println(F("Wet/Dry waste sorter ready."));
  Serial.println(F("Commands: s=sort  r=live readings  tNNN=set threshold  w/d=servo test"));
  Serial.print(F("threshold = "));
  Serial.println(WET_THRESHOLD);
}

void loop() {
  handleSerial();

  if (triggerPressed()) {
    sortOnce();
  }

  if (liveReadMode) {
    static unsigned long last = 0;
    if (millis() - last >= 1000) {
      last = millis();
      Serial.print(F("raw="));
      Serial.println(readMoistureRaw());
    }
  }
}
