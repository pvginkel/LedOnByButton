#include <EEPROM.h>
#include "support.h"

#define DEBOUNCE 50
#define IO_LED 3
#define IO_BUTTON 10

time_t blinkInterval;
time_t lastMillis = 0;
time_t programmingStart = 0;
bool isProgramming = false;

void setup() {
  pinMode(IO_BUTTON, INPUT_PULLUP);
  pinMode(IO_LED, OUTPUT);

  Serial.begin(115200);
  while (!Serial) ;

  EEPROM.get(0, blinkInterval);
  LOG("READ BLINK INTERVAL ", blinkInterval);
}

void loop() {
  if (isProgramming) {
    programmingLoop();
  } else {
    normalLoop();
  }
}

void normalLoop() {
  if (isButtonPressed()) {
    startProgramming();
    return;
  }

  auto currentMillis = millis();
  auto delta = (currentMillis - lastMillis) % (blinkInterval * 2);
  auto fraction = (float)delta / blinkInterval;

  // Make the light go up and down.
  if (fraction > 1) {
    fraction = 2 - fraction;
  }

  // Scale the factor to make it appear more natural.
  fraction = fraction * fraction;

  analogWrite(IO_LED, (int)(fraction * 256));
  delay(1);
}

void startProgramming() {
  LOG("START PROGRAMMING");

  analogWrite(IO_LED, 255);

  programmingStart = millis();
  isProgramming = true;
}

void programmingLoop() {
    if (!isButtonPressed()) {
      return;
    }

    lastMillis = millis();

    blinkInterval = lastMillis - programmingStart;
    isProgramming = false;

    EEPROM.put(0, blinkInterval);

    LOG("END PROGRAMMING interval ", blinkInterval);

    analogWrite(IO_LED, 0);
}

int isButtonPressed() {
  if (digitalRead(IO_BUTTON) != LOW) {
    return 0;
  }

  auto startMillis = millis();
  auto debounceStartMillis = startMillis;
  auto state = LOW;

  while (true) {
    auto newState = digitalRead(IO_BUTTON);
    auto currentMillis = millis();

    if (newState != state) {
      state = newState;
      debounceStartMillis = currentMillis;
    } else if (currentMillis - debounceStartMillis > DEBOUNCE && state != LOW) {
      break;
    }
  }

  auto diff = millis() - startMillis;
  LOG("BUTTON PRESS TIME ", diff);
  return diff;
}

bool blinkLed() {
  digitalWrite(IO_LED, HIGH);
  delay(50);
  digitalWrite(IO_LED, LOW);
}
