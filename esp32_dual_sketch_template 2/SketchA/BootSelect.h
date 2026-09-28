#pragma once
// ============================================================
//  BootSelect.h  (generic template)
//
//  Lets either sketch hand off control to the OTHER app partition
//  and reboot into it, using ESP-IDF's OTA boot-partition selector.
//  No actual OTA update happens -- we're just choosing which
//  already-flashed binary runs next.
//
//  Default trigger: hold a button on BOOT_SELECT_PIN for
//  BOOT_SELECT_HOLD_MS, or send "SWITCH" over Serial, within
//  BOOT_SELECT_WINDOW_MS of boot. Replace the trigger check inside
//  checkForPartitionSwitch() with whatever condition fits your
//  project -- a sensor value, an NVS flag, a schedule, etc. The
//  actual switch action (last 2 lines of
//  switchToOtherPartitionAndReboot) never needs to change.
//
//  IMPORTANT: keep this file IDENTICAL in both sketch folders.
// ============================================================

#include <Arduino.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>

// Pick any GPIO. Avoid strapping pins (0, 2, 5, 12, 15) if you can;
// GPIO34-39 are input-only with no internal pull-up, so INPUT_PULLUP
// won't work there -- use an external resistor if you must use one.
#define BOOT_SELECT_PIN         0     // e.g. the BOOT button on most dev boards
#define BOOT_SELECT_HOLD_MS     2000  // hold time required to trigger a switch
#define BOOT_SELECT_WINDOW_MS   3000  // how long at startup we listen for a request

// Finds the "other" app partition (the one NOT currently running),
// points the boot selector at it, and restarts into it.
inline void switchToOtherPartitionAndReboot() {
  const esp_partition_t *running = esp_ota_get_running_partition();
  if (running == nullptr) {
    Serial.println("[BootSelect] Could not determine running partition.");
    return;
  }

  esp_partition_subtype_t targetSubtype =
      (running->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0)
          ? ESP_PARTITION_SUBTYPE_APP_OTA_1
          : ESP_PARTITION_SUBTYPE_APP_OTA_0;

  const esp_partition_t *target =
      esp_partition_find_first(ESP_PARTITION_TYPE_APP, targetSubtype, NULL);

  if (target == nullptr) {
    Serial.println("[BootSelect] Target partition not found -- check partitions.csv was flashed.");
    return;
  }

  Serial.printf("[BootSelect] Switching boot partition: '%s' -> '%s'\n",
                running->label, target->label);

  esp_err_t err = esp_ota_set_boot_partition(target);
  if (err != ESP_OK) {
    Serial.printf("[BootSelect] esp_ota_set_boot_partition failed (err=%d)\n", err);
    return;
  }

  Serial.println("[BootSelect] Rebooting into the other sketch now...");
  Serial.flush();
  delay(200);
  esp_restart();
}

// Call once, near the top of setup(), in BOTH sketches.
// Blocks for up to BOOT_SELECT_WINDOW_MS checking for a switch request;
// if none arrives, returns and the sketch continues normally.
//
// Replace the body of this loop with whatever trigger makes sense for
// your project -- this default is button-hold OR serial command.
inline void checkForPartitionSwitch(const char *mySketchName) {
  pinMode(BOOT_SELECT_PIN, INPUT_PULLUP);

  const esp_partition_t *running = esp_ota_get_running_partition();
  Serial.printf("\n[BootSelect] Running '%s' from partition '%s'\n",
                mySketchName, running ? running->label : "?");
  Serial.println("[BootSelect] Hold button 2s, or send \"SWITCH\" over "
                  "Serial, within 3s to swap to the other sketch...");

  unsigned long windowStart = millis();
  unsigned long pressedSince = 0;

  while (millis() - windowStart < BOOT_SELECT_WINDOW_MS) {
    if (digitalRead(BOOT_SELECT_PIN) == LOW) {
      if (pressedSince == 0) pressedSince = millis();
      if (millis() - pressedSince >= BOOT_SELECT_HOLD_MS) {
        switchToOtherPartitionAndReboot();
        break; // only reached if the switch failed
      }
    } else {
      pressedSince = 0;
    }

    if (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      if (cmd.equalsIgnoreCase("SWITCH")) {
        switchToOtherPartitionAndReboot();
        break;
      }
    }

    delay(10);
  }

  Serial.println("[BootSelect] Continuing with this sketch.\n");
}
