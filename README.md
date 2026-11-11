# CH47 firmware

VIA-compatible firmware for the **CH47** (47-key 4×12 ortholinear, WCH **CH552T**) with **4 layers**, all editable in VIA.

Fork of [lbtrobit/CH55x_QMK_Lite](https://github.com/lbtrobit/CH55x_QMK_Lite) at `90cc1ad`. The stock CH47 firmware is a 1-layer build of the same project. Prebuilt `.hex` files and the VIA layout `.json` are under **Releases**.

## Features

- 4 layers, all editable in VIA. The keymap is stored in code flash, because the 128-byte DataFlash only fits one layer.
- Layer keys: `MO(n)`, `TG(n)`, `TO(n)`, `DF(n)`, and `KC_TRNS` falls through to lower layers. `LT()` and `OSL()` are not supported.
- 5 Tap Dances and 92 bytes of macros. The stock firmware overflowed DataFlash and corrupted its keymap.

## Hardware

6×8 matrix, diodes COL2ROW.

| | Pins |
|---|---|
| Rows 0–5 | P1.2, P1.3, P3.5, P3.3, P1.7, P1.6 |
| Cols 0–7 | P1.5, P1.0, P3.2, P1.4, P3.0, P1.1, P3.4, P3.1 |

## Storage notes

- The keymap occupies code flash `0x367E`–`0x37FF`, just below the bootloader at `0x3800`. DataFlash holds the Tap Dances and macros.
- **Endurance:** the spec rates each code flash word at about 200 writes. The firmware only writes keys that actually change, so normal VIA use is fine.
- Re-flashing resets the keymap to its default, so save your keymap in VIA first.

## Build

Requires Arduino CLI or the Arduino IDE with the [CH55xDuino](https://github.com/DeqingSun/ch55xduino) 0.0.18 core.

1. Apply `patches/ch55xduino-0.0.18-main.c.patch` to the core's `cores/ch55xduino/main.c`. It removes unused ISRs to free ROM; without it, the program overlaps the keymap region.
2. Compile. The repo folder must be named `CH47_firmware`.
   ```sh
   arduino-cli compile -b CH55xDuino:mcs51:ch552:usb_settings=user266,clock=24internal --output-dir build CH47_firmware
   ```
3. Check the image before flashing. The script fails if the program overlaps the keymap region.
   ```sh
   python scripts/check_hex.py build/CH47_firmware.ino.hex
   ```

## Flash

1. Enter the bootloader by sending the VIA raw HID command `[0x00, 0x07, 0x06, 0x02, 0x01]`. On Windows, the bootloader `4348:55E0` needs the WinUSB driver, which you can install with Zadig.
2. Within about 10 seconds, run:
   ```sh
   vnproch55x -r 8 -t CH552 build/CH47_firmware.ino.hex
   ```

## License

Same as upstream (see `LICENSE`).
