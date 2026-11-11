"""Post-build guard: fail if the firmware image reaches the code-flash keymap block.

Usage: python scripts/check_hex.py build/CH47_firmware.ino.hex [layers]
The keymap block is KEYMAP_FLASH_MAGIC_ADDR .. 0x37FF (see src/qmk/quantum/dynamic_keymap.h):
  0x3800 - layers * 6 rows * 8 cols * 2 bytes - 2 (magic word).
Flashing an image that overlaps it would let VIA keymap writes corrupt the program.
"""
import sys

hex_path = sys.argv[1]
layers = int(sys.argv[2]) if len(sys.argv) > 2 else 4
limit = 0x3800 - layers * 6 * 8 * 2 - 2

end, base = 0, 0
with open(hex_path) as f:
    for line in f:
        line = line.strip()
        if not line.startswith(":"):
            continue
        n, addr, rtype = int(line[1:3], 16), int(line[3:7], 16), int(line[7:9], 16)
        if rtype == 0x04:
            base = int(line[9:13], 16) << 16
        elif rtype == 0x00:
            end = max(end, base + addr + n)

free = limit - end
print(f"image end 0x{end:04X}, keymap block starts 0x{limit:04X}, free {free} bytes")
sys.exit(0 if free >= 0 else 1)
