#pragma once

// CH47 — 47-key 4x12 ortholinear, WCH CH552T.
// Pin map measured on 2026-11-11 with a GPIO pin-finder sketch:
// 47/47 keys, one pin pair each, diode COL2ROW.

#define VENDOR_ID       0x1209
#define PRODUCT_ID      0xAB73                  // same as original, so the original CH47_via.json keeps working
#define MANUFACTURER    'C','H','4','7'
#define PRODUCT         'C','H','4','7'

#define MATRIX_ROWS     6
#define MATRIX_COLS     8

//                       P1.2 P1.3 P3.5 P3.3 P1.7 P1.6
#define MATRIX_ROW_PINS { 12,  13,  35,  33,  17,  16 }
//                       P1.5 P1.0 P3.2 P1.4 P3.0 P1.1 P3.4 P3.1
#define MATRIX_COL_PINS { 15,  10,  32,  14,  30,  11,  34,  31 }

#define DIODE_DIRECTION COL2ROW

// B2 build: keymap stored in a reserved code-flash block (see dynamic_keymap.h,
// KEYMAP_IN_CODE_FLASH), so DataFlash (128 B) only holds:
//   magic 1 + macro loops 5 + tap dance 5*6 = 36 -> 92 bytes left for macros.
#define KEYMAP_IN_CODE_FLASH
#define MULTI_LAYER_ENABLE
#define DYNAMIC_KEYMAP_LAYER_COUNT  4      // 4 * 96 B = 384 B at 0x3680-0x37FF (+ magic at 0x367E)
#define KEYMAP_DEFAULT_LAYERS       2      // layers 2-3 reset to KC_TRNS

#define TAP_DANCE_ENABLE
#define TAP_DANCE_COUNT  5
#define TAPPING_TERM     200

#define EXTRAKEY_ENABLE
#define MOUSE_ENABLE
#define FCODE_TO_MOUSE_ENABLE
#define DIAL_ENABLE

// No RGB: all 14 free GPIOs of the CH552T are used by the 6x8 matrix.
#define RGB_MATRIX_LED_COUNT 1   // only to satisfy rgb_matrix headers; RGB_MATRIX_ENABLE stays off
