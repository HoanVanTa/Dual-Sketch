# General procedure: two sketches, one ESP32, NVS as shared store

This is the reusable pattern behind the Daikin listener/controller setup,
stripped of anything AC-specific. Use it any time you want two
independently-built sketches to live on one chip, choose which one boots,
and pass data between them across a reboot.

## The four pieces

1. **A partition table with two app slots** (`ota_0`, `ota_1`) instead of
   the usual single app partition.
2. **A boot-selector mechanism** using ESP-IDF's
   `esp_ota_set_boot_partition()` to decide, at runtime, which slot boots
   next — triggered by a button, a serial command, a sensor value,
   whatever fits your project.
3. **NVS (`Preferences`) as the shared store** — it lives in its own
   partition, untouched by either app slot, so both binaries can
   read/write the same keys even though they're unrelated compiled
   programs.
4. **A build/flash procedure** that gets sketch A into `ota_0` and sketch
   B into `ota_1` on the same physical chip.

## Step-by-step

### 1. Design the partition table

Copy `partitions.csv` from this template. The only things you'd normally
change:
- App slot **size** — bump it up if either sketch + its libraries won't
  fit in 1.5MB (check the compiled `.bin` size; Arduino's IDE console
  shows "Sketch uses X bytes").
- Whether you need extra partitions (e.g. SPIFFS/LittleFS) — add them
  after `app1`, 4KB-aligned offsets.

The two things that must NOT change without understanding why:
- `otadata` must exist and be a `data`/`ota` partition — it's what the
  ROM bootloader reads to know which app slot to boot. Blank on first
  flash, so it defaults to `ota_0` (whichever sketch you flash there
  runs first).
- The app slot `subtype` values must literally be `ota_0` and `ota_1` —
  that's what `esp_ota_set_boot_partition()` and
  `esp_partition_find_first()` look up by.

### 2. Add the boot-selector to both sketches

Copy `BootSelect.h` into both sketch folders (must be identical in
both). Call `checkForPartitionSwitch("MySketchName")` near the top of
`setup()` in both sketches — it blocks briefly for a trigger, then
returns and the sketch continues normally if nothing happened.

Customize the **trigger condition** for your project — the button/serial
check here is just one option:
- Physical button (as-is)
- A specific serial command
- A GPIO tied to a switch/jumper
- A condition read from NVS itself (e.g. "if a `mode` flag says
  `controller`, switch")
- A sensor threshold, a timeout, a schedule — anything your app logic
  can decide

Whatever the trigger, the actual switch action is always the same two
lines:
```cpp
esp_ota_set_boot_partition(target_partition);
esp_restart();
```

### 3. Add the shared store to both sketches

Copy `SharedStore.h` into both sketch folders (identical in both). It
wraps `Preferences` with typed helpers (`bytes`, `int`, `float`,
`String`) under one namespace, so sketch A can write a value under a key
and sketch B can read it after a reboot into a completely different
binary. Add whatever helpers your data needs — it's a thin wrapper, not
a fixed schema.

Design the same way you'd design any shared API: agree on key names and
value types up front (e.g. in a comment block both sketches share), since
there's no compiler to catch a mismatch between what A wrote and what B
expects to read.

### 4. Build and flash

1. Set `Tools > Partition Scheme > Custom` in the Arduino IDE for
   **both** sketches (each needs `partitions.csv` sitting next to its
   `.ino`).
2. Flash **Sketch A** normally via the IDE (Ctrl+U). This lays down
   bootloader + partition table + Sketch A in `ota_0`. Blank `otadata`
   means it boots first.
3. For **Sketch B**: `Sketch > Export Compiled Binary`, then flash it
   directly to the `ota_1` offset with `esptool.py`:
   ```bash
   esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash \
     0x190000 SketchB/build/.../SketchB.ino.bin
   ```
   (Replace `0x190000` with whatever offset you gave `app1` in your
   `partitions.csv` if you changed it.)

Re-flashing later: redo only the half that changed. Changing Sketch A
means redoing step 2 (which also re-writes the partition table, so do
this if you ever change `partitions.csv` too). Changing only Sketch B
means just step 3.

### 5. Runtime behavior

- First power-up boots Sketch A (blank `otadata` defaults to `ota_0`).
- Trigger the switch (per however you wired it in step 2) to reboot into
  Sketch B.
- From then on, every power-cycle boots directly into whichever slot was
  last selected — the choice persists in `otadata` until changed again.
- Data written to NVS by either sketch survives every reboot and every
  switch, in both directions.

## Files in this template

```
esp32_dual_sketch_template/
├── partitions.csv
├── SketchA/
│   ├── SketchA.ino          (minimal example: writes a value, can switch)
│   ├── BootSelect.h
│   ├── SharedStore.h
│   └── partitions.csv
└── SketchB/
    ├── SketchB.ino          (minimal example: reads the value, can switch)
    ├── BootSelect.h
    ├── SharedStore.h
    └── partitions.csv
```

Treat `SketchA.ino`/`SketchB.ino` as a skeleton to gut and replace with
your real logic — the boot-select and NVS calls are the only parts that
matter for the pattern itself.
