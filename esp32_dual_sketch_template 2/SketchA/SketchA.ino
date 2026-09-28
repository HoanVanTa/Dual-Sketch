// ============================================================
//  SketchA.ino  (minimal example)
//
//  Demonstrates the pattern only -- replace the body with your
//  real logic. This one just writes a counter to the shared store
//  every 5s and lets you switch to SketchB at any time.
// ============================================================

#include "BootSelect.h"
#include "SharedStore.h"

unsigned long lastWrite = 0;
int counter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  checkForPartitionSwitch("SketchA");

  Serial.println("SketchA running.");
  counter = sharedGetInt("counter", 0);  // resume where we left off, if any
  Serial.printf("Resuming counter at %d\n", counter);
}

void loop() {
  if (millis() - lastWrite >= 5000) {
    lastWrite = millis();
    counter++;
    sharedPutInt("counter", counter);
    Serial.printf("Wrote counter = %d to shared store\n", counter);
  }
}
