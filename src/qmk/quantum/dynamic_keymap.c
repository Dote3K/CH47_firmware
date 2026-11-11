/* Copyright 2023 lbtrobit
 * Copyright 2017 Jason Williams (Wilba)
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

#include <Arduino.h>
#include "../platforms/progmem.h"
#include "quantum.h"
#include "dynamic_keymap.h"
#include "via.h"
#include <stdint.h>
#include <ctype.h>

#ifndef KEYMAP_DEFAULT_LAYERS
#define KEYMAP_DEFAULT_LAYERS DYNAMIC_KEYMAP_LAYER_COUNT
#endif
// Only the first KEYMAP_DEFAULT_LAYERS layers have defaults in flash; the rest reset to KC_TRNS.
const __code uint16_t keymaps[KEYMAP_DEFAULT_LAYERS][MATRIX_ROWS][MATRIX_COLS] = KEY_MAPS
#ifdef ENCODER_ENABLE
const __code uint16_t encoder_map[DYNAMIC_KEYMAP_LAYER_COUNT][NUM_ENCODERS][NUM_DIRECTIONS] = ENCODER_MAP
#endif // ENCODER_ENABLE

#ifdef KEYMAP_IN_CODE_FLASH
// ---- CH552 code-flash storage (IAP). Verified on the CH47 on 2026-11-11 with an IAP test sketch:
// a word write (ROM_CMD_WRITE) overwrites any old value without a separate erase, ~100-250 us,
// neighbouring words untouched, survives reset. Spec endurance is only ~200 writes per word, so
// writes are skipped when the stored value is already correct.
#define CF_WORD(a) (*((__code uint16_t *)(a)))

static void cf_write_word(uint16_t addr, uint16_t val) {
    if (addr < KEYMAP_FLASH_MAGIC_ADDR || addr >= KEYMAP_FLASH_END || (addr & 1)) return;  // hard guard
    if (CF_WORD(addr) == val) return;                                                       // save wear
    __bit ea = EA;
    EA = 0;
    SAFE_MOD = 0x55; SAFE_MOD = 0xAA;
    GLOBAL_CFG |= bCODE_WE;
    SAFE_MOD = 0;
    ROM_ADDR_H = addr >> 8;
    ROM_ADDR_L = addr & 0xFF;
    ROM_DATA_H = val >> 8;
    ROM_DATA_L = val & 0xFF;
    if (ROM_STATUS & bROM_ADDR_OK) {
        ROM_CTRL = ROM_CMD_WRITE;
    }
    SAFE_MOD = 0x55; SAFE_MOD = 0xAA;
    GLOBAL_CFG &= ~bCODE_WE;
    SAFE_MOD = 0;
    EA = ea;
}

uint8_t dynamic_keymap_read_byte(uint16_t offset) {
    if (offset >= KEYMAP_FLASH_SIZE) return 0;
    return *((__code uint8_t *)(KEYMAP_FLASH_ADDR + offset));
}

void dynamic_keymap_write_byte(uint16_t offset, uint8_t value) {
    if (offset >= KEYMAP_FLASH_SIZE) return;
    uint16_t addr = KEYMAP_FLASH_ADDR + offset;
    uint16_t w    = CF_WORD(addr & 0xFFFE);
    if (addr & 1) w = (w & 0x00FF) | ((uint16_t)value << 8);
    else          w = (w & 0xFF00) | value;
    cf_write_word(addr & 0xFFFE, w);
}

bool keymap_flash_valid(void) {
    return CF_WORD(KEYMAP_FLASH_MAGIC_ADDR) == KEYMAP_FLASH_MAGIC;
}

void keymap_flash_set_valid(void) {
    cf_write_word(KEYMAP_FLASH_MAGIC_ADDR, KEYMAP_FLASH_MAGIC);
}

static uint16_t keymap_offset(uint8_t layer, uint8_t row, uint8_t column) {
    return ((uint16_t)(layer * MATRIX_ROWS + row) * MATRIX_COLS + column) * 2;
}

uint16_t dynamic_keymap_get_keycode(uint8_t layer, uint8_t row, uint8_t column) {
    if (layer >= DYNAMIC_KEYMAP_LAYER_COUNT || row >= MATRIX_ROWS || column >= MATRIX_COLS) return KC_NO;
    uint16_t w = CF_WORD(KEYMAP_FLASH_ADDR + keymap_offset(layer, row, column));
    return (w << 8) | (w >> 8);  // stored big-endian
}

void dynamic_keymap_set_keycode(uint8_t layer, uint8_t row, uint8_t column, uint16_t keycode) {
    if (layer >= DYNAMIC_KEYMAP_LAYER_COUNT || row >= MATRIX_ROWS || column >= MATRIX_COLS) return;
    cf_write_word(KEYMAP_FLASH_ADDR + keymap_offset(layer, row, column), (keycode << 8) | (keycode >> 8));
}

#else  // keymap in DataFlash (upstream)

uint8_t dynamic_keymap_read_byte(uint16_t offset) {
    return eeprom_read_byte(DYNAMIC_KEYMAP_EEPROM_ADDR + offset);
}

void dynamic_keymap_write_byte(uint16_t offset, uint8_t value) {
    eeprom_write_byte(DYNAMIC_KEYMAP_EEPROM_ADDR + offset, value);
}

uint16_t dynamic_keymap_get_keycode(uint8_t layer, uint8_t row, uint8_t column) {
    uint8_t address = DYNAMIC_KEYMAP_EEPROM_ADDR + (layer * MATRIX_ROWS * MATRIX_COLS * 2) + (row * MATRIX_COLS * 2) + (column * 2);
    // Big endian, so we can read/write EEPROM directly from host if we want
    uint16_t keycode = (eeprom_read_byte(address) << 8) + (eeprom_read_byte(address + 1) & 0xFF);
    return keycode;
}

void dynamic_keymap_set_keycode(uint8_t layer, uint8_t row, uint8_t column, uint16_t keycode) {
    uint8_t address = DYNAMIC_KEYMAP_EEPROM_ADDR + (layer * MATRIX_ROWS * MATRIX_COLS * 2) + (row * MATRIX_COLS * 2) + (column * 2);
    // Big endian, so we can read/write EEPROM directly from host if we want
    eeprom_write_byte(address, (uint8_t)(keycode >> 8));
    eeprom_write_byte(address + 1, (uint8_t)(keycode & 0xFF));
}

#endif // KEYMAP_IN_CODE_FLASH

#ifdef ENCODER_ENABLE
uint16_t dynamic_keymap_get_encoder(uint8_t layer, uint8_t encoder_id, bool clockwise) {
    uint8_t address = DYNAMIC_KEYMAP_ENCODER_EEPROM_ADDR + (layer * NUM_ENCODERS * 2 * 2) + (encoder_id * 2 * 2);
    // Big endian, so we can read/write EEPROM directly from host if we want
    uint16_t keycode = ((uint16_t)eeprom_read_byte(address + (clockwise ? 0 : 2))) << 8;
    keycode |= eeprom_read_byte(address + (clockwise ? 0 : 2) + 1);
    return keycode;
}

void dynamic_keymap_set_encoder(uint8_t layer, uint8_t encoder_id, bool clockwise, uint16_t keycode) {
    uint8_t address = DYNAMIC_KEYMAP_ENCODER_EEPROM_ADDR + (layer * NUM_ENCODERS * 2 * 2) + (encoder_id * 2 * 2);
    // Big endian, so we can read/write EEPROM directly from host if we want
    eeprom_write_byte(address + (clockwise ? 0 : 2), (uint8_t)(keycode >> 8));
    eeprom_write_byte(address + (clockwise ? 0 : 2) + 1, (uint8_t)(keycode & 0xFF));
}
#endif // ENCODER_ENABLE

void dynamic_keymap_reset(void) {
    // Reset the keymaps in EEPROM to what is in flash.
    for (uint8_t layer = 0; layer < DYNAMIC_KEYMAP_LAYER_COUNT; layer++) {
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t column = 0; column < MATRIX_COLS; column++) {
                dynamic_keymap_set_keycode(layer, row, column,
                    layer < KEYMAP_DEFAULT_LAYERS ? pgm_read_word(&keymaps[layer][row][column]) : KC_TRNS);
            }
        }
    }
#ifdef ENCODER_ENABLE
    for (uint8_t layer = 0; layer < DYNAMIC_KEYMAP_LAYER_COUNT; layer++) {
        for (uint8_t encoder = 0; encoder < NUM_ENCODERS; encoder++) {
            dynamic_keymap_set_encoder(layer, encoder, true, pgm_read_word(&encoder_map[layer][encoder][true ? 0 : 1]));
            dynamic_keymap_set_encoder(layer, encoder, false, pgm_read_word(&encoder_map[layer][encoder][false ? 0 : 1]));
        }
    }
#endif // ENCODER_ENABLE
}
