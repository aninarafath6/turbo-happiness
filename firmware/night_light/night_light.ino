/*
 * Automatic night light
 * ---------------------
 * Hardware: Arduino Nano + 3-pin LDR light sensor module (DO / GND / VCC).
 *
 * Room light ON  -> LED off
 * Room light OFF -> LED on
 *
 * Wiring
 *   LDR module  VCC -> 5V
 *               GND -> GND
 *               DO  -> D2
 *   LED         built-in LED on D13 is used. For an external LED:
 *               D13 -> LED long leg (+), LED short leg -> 220 ohm -> GND
 *
 * Setup
 *   1. Upload, open Serial Monitor at 115200.
 *   2. It prints "DO=1 -> DARK" or "DO=0 -> LIGHT" every half second.
 *   3. Cover the LDR with your finger. If the printed word is wrong, flip
 *      DARK_IS_HIGH below.
 *   4. Turn the blue pot until the switch point is where you want it:
 *      it should say LIGHT with the room lights on and DARK with them off.
 */

const uint8_t PIN_LDR_DO = 2;
const uint8_t PIN_LED    = 13;   // built-in LED

// Most LM393 LDR boards output HIGH when dark. Set to false if yours is reversed.
const bool DARK_IS_HIGH = true;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LDR_DO, INPUT);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  Serial.println(F("Night light ready."));
}

void loop() {
  int  raw  = digitalRead(PIN_LDR_DO);
  bool dark = DARK_IS_HIGH ? (raw == HIGH) : (raw == LOW);

  digitalWrite(PIN_LED, dark ? HIGH : LOW);

  static unsigned long last = 0;
  if (millis() - last >= 500) {
    last = millis();
    Serial.print(F("DO="));
    Serial.print(raw);
    Serial.print(F(" -> "));
    Serial.println(dark ? F("DARK  (LED on)") : F("LIGHT (LED off)"));
  }
}
