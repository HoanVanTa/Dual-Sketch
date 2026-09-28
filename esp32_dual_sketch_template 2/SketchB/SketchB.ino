// ============================================================
//  SketchB.ino  (minimal example)
//
//  Demonstrates the pattern only -- replace the body with your
//  real logic. This one just reads the counter SketchA wrote and
//  lets you switch back to SketchA at any time.
// ============================================================

#include "BootSelect.h"
#include "SharedStore.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  checkForPartitionSwitch("SketchB");

  Serial.println("SketchB running.");

  if (sharedHasKey("counter")) {
    int counter = sharedGetInt("counter", -1);
    Serial.printf("Read counter = %d from shared store (written by SketchA)\n", counter);
  } else {
    Serial.println("No 'counter' key found yet -- switch to SketchA first.");
  }
}

void loop() {
  // real logic goes here
}
