/* Copyright 2016-2017 Jack Humbert
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "quantum.h"
#include "dynamic_keymap.h"

__data uint8_t current_layer = 0, layer_store = 0;

#ifdef MULTI_LAYER_ENABLE
// CH47: QMK-style layer keycodes (as offered by VIA) for up to 8 layers.
//   MO(n) momentary, TG(n) toggle, TO(n) switch (clears toggles), DF(n) default layer,
//   plus the CH55x_QMK_Lite custom KC_TO_0/1 and KC_MO_0/1.
// Active layer = highest layer that is held (MO) or toggled (TG), else the default layer.
// KC_TRNS falls through to the next lower active layer. The layer a key resolved to is
// remembered at press time, so releasing it after the layer changed releases the right key.
__data uint8_t layer_default = 0, layer_mo = 0, layer_tg = 0;
__data uint8_t event_layer = 0;
static __xdata uint8_t key_layer[MATRIX_ROWS * MATRIX_COLS];

static uint8_t layer_active_mask(void) {
    return layer_mo | layer_tg | (uint8_t)(1 << layer_default) | 1;
}

static void layer_update(void) {
    uint8_t mask = layer_mo | layer_tg;
    current_layer = layer_default;
    for (uint8_t l = DYNAMIC_KEYMAP_LAYER_COUNT - 1; l > layer_default; l--) {
        if (mask & (1 << l)) { current_layer = l; break; }
    }
}

// Called from action_exec for every matrix key event, before the keycode is looked up.
void layer_resolve_event(keypos_t key, bool pressed) {
    uint8_t idx = key.row * MATRIX_COLS + key.col;
    if (pressed) {
        uint8_t mask = layer_active_mask();
        uint8_t l = current_layer;
        while (l > 0 && (!(mask & (1 << l)) || dynamic_keymap_get_keycode(l, key.row, key.col) == KC_TRNS)) l--;
        key_layer[idx] = l;
    }
    event_layer = key_layer[idx];
}

// Returns true if keycode was a layer key (and has been handled).
static bool layer_switch(uint16_t keycode, keyrecord_t *record) {
    uint8_t kind, n;
    if      (keycode == KC_TO_0) { kind = 0; n = 0; }
    else if (keycode == KC_TO_1) { kind = 0; n = 1; }
    else if (keycode == KC_MO_0) { kind = 1; n = 0; }
    else if (keycode == KC_MO_1) { kind = 1; n = 1; }
    else if (keycode >= QK_TO && keycode <= QK_TOGGLE_LAYER_MAX) {
        kind = (keycode - QK_TO) >> 5;   // 0=TO 1=MO 2=DF 3=TG
        n    = keycode & 0x1F;
    } else {
        return false;
    }
    if (n >= DYNAMIC_KEYMAP_LAYER_COUNT) return true;
    uint8_t bit = 1 << n;
    if (record->event.pressed) {
        switch (kind) {
            case 0: layer_tg = 0; layer_mo = 0; layer_default = n; break;  // TO: only layer n (plus default)
            case 1: layer_mo |= bit; break;
            case 2: layer_default = n; break;
            case 3: layer_tg ^= bit; break;
        }
    } else if (kind == 1) {
        layer_mo &= ~bit;
    }
    layer_update();
    return true;
}
#endif // MULTI_LAYER_ENABLE

#ifdef TWO_LAYER_ENABLE
void layer_switch(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        switch (keycode) {
            case KC_TO_0: current_layer = 0; break;
            case KC_TO_1: current_layer = 1; break;
            case KC_MO_0: layer_store = current_layer; current_layer = 0; break;
            case KC_MO_1: layer_store = current_layer; current_layer = 1; break;
            default: break;
        }
    } else {
        if (keycode == KC_MO_0 || keycode == KC_MO_1) {
            current_layer = layer_store;
        }
    }
}
#endif // TWO_LAYER_ENABLE

#ifdef DIAL_ENABLE
void process_dial_key(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        add_dial_key(keycode);
    } else {
        del_dial_key(keycode);
    }
    send_dial_report();
}
#endif

/* Core keycode function, hands off handling to other functions,
    then processes internal quantum keycodes, and then processes
    ACTIONs.                                                      */
bool process_record_quantum(keyrecord_t *record) {
    uint16_t keycode = keymap_key_to_keycode(record->event.key);

#ifdef MULTI_LAYER_ENABLE
    if (layer_switch(keycode, record)) {
        return false;
    }
#endif // MULTI_LAYER_ENABLE

#ifdef TWO_LAYER_ENABLE
    if (keycode >= KC_TO_0 && keycode <= KC_MO_1) {
        layer_switch(keycode, record);
        return false;
    }
#endif // TWO_LAYER_ENABLE

#ifdef TAP_DANCE_ENABLE
    if (process_tap_dance(keycode, record) == false) {
        return false;
    }
#endif // TAP_DANCE_ENABLE

#ifdef DIAL_ENABLE
    if (keycode >= DIAL_CCW && keycode <= DIAL_CW) {
        process_dial_key(keycode, record);
        return false;
    }
#endif // DIAL_ENABLE

    if (process_record_via(keycode, record) == false) {
        return false;
    }

#if defined(RGB_EFFECTS_PLUS) && defined(RGB_MATRIX_ENABLE) && defined(RGB_KEY_CONTROL)
    if (process_rgb_matrix(keycode, record) == false) {
        return false;
    }
#endif
    
    return true;
}

// void usb_send_for_debug(uint8_t v1, uint8_t v2)
// {
//     report_keyboard_t report1;
//     report1.keys[0] = 0x1E + v1;
//     report1.keys[1] = v2;
//     host_keyboard_send(&report1);
//     report1.keys[0] = 0x00;
//     report1.keys[1] = 0x00;
//     host_keyboard_send(&report1);
// }