#pragma once

// clang-format off
// Electrical 6x8 matrix, see CH47_via.json:
//   rows 0-3, cols 0-7 = left 8 keys of visible rows 1-4
//   row 4 = right 4 keys of visible rows 1 (cols 0-3) and 2 (cols 4-7)
//   row 5 = right 4 keys of visible rows 3 (cols 0-3) and 4 (cols 4-7)
// (3,5) has no physical switch.
//
// Layer 0 = keymap read from the original CH47 firmware on 2026-11-11
// (read back over VIA before re-flashing). (3,3) = 0x5221 = MO(1), which now works.
//
// Layer 1 (hold MO(1)), as seen on the board:
//   `    1    2    3    4    5    6    7    8    9    0    Del
//   Caps F1   F2   F3   F4   F5   F6   -    =    [    ]    \
//   ___  F7   F8   F9   F10  F11  F12  '    ___  ___  PgUp ___
//   ___  ___  ___  ___  ____ ___ (2u)  Del  ___  Home PgDn End
// Layers 2-3: all transparent, assign MO(2)/MO(3)/TG(n) in VIA to use them.

#define KEY_MAPS {                                                                 \
   {{KC_ESC,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U},         \
    {KC_TAB,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J},         \
    {KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M},         \
    {KC_LCTL, KC_LGUI, KC_LALT, MO(1),   KC_SPC,  KC_SPC,  KC_SPC,  KC_BSPC},      \
    {KC_I,    KC_O,    KC_P,    KC_BSPC, KC_K,    KC_L,    KC_SCLN, KC_ENT},       \
    {KC_COMM, KC_DOT,  KC_UP,   KC_SLSH, KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT},},    \
   {{KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7},         \
    {KC_CAPS, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_MINS},      \
    {KC_TRNS, KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_QUOT},      \
    {KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_DEL},       \
    {KC_8,    KC_9,    KC_0,    KC_DEL,  KC_EQL,  KC_LBRC, KC_RBRC, KC_BSLS},      \
    {KC_TRNS, KC_TRNS, KC_PGUP, KC_TRNS, KC_TRNS, KC_HOME, KC_PGDN, KC_END},},     \
};

// clang-format on
