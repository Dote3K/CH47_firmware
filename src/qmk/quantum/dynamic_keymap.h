/* Copyright 2017 Jason Williams (Wilba)
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
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "quantum.h"
#include "dynamic_macro.h"
#include "dynamic_tap_dance.h"
#ifdef ENCODER_ENABLE
#    include "encoder.h"
#endif

#ifndef DYNAMIC_KEYMAP_LAYER_COUNT
#ifdef TWO_LAYER_ENABLE
#define DYNAMIC_KEYMAP_LAYER_COUNT          2
#else
#define DYNAMIC_KEYMAP_LAYER_COUNT          1
#endif
#endif

#ifdef KEYMAP_IN_CODE_FLASH
// CH47: keymap lives in a reserved code-flash block just below the bootloader (0x3800).
// The 128-byte DataFlash only holds magic, macro loops, tap dances and macros.
// Layout: [magic word][layer 0][layer 1]...  keycodes big-endian (VIA buffer order).
#   define KEYMAP_FLASH_END                 0x3800
#   define KEYMAP_FLASH_SIZE                (DYNAMIC_KEYMAP_LAYER_COUNT * MATRIX_ROWS * MATRIX_COLS * 2)
#   define KEYMAP_FLASH_ADDR                (KEYMAP_FLASH_END - KEYMAP_FLASH_SIZE)
#   define KEYMAP_FLASH_MAGIC_ADDR          (KEYMAP_FLASH_ADDR - 2)
#   define KEYMAP_FLASH_MAGIC               (0xC400 | DYNAMIC_KEYMAP_LAYER_COUNT)
#   if (KEYMAP_FLASH_ADDR & 1)
#       error "keymap flash block must be word aligned"
#   endif
#endif

#define TOTAL_EEPROM_BYTE_COUNT             128

#define DYNAMIC_KEYMAP_MACRO_COUNT          5
#define DYNAMIC_KEYMAP_EEPROM_START         (VIA_EEPROM_CONFIG_END)
#define DYNAMIC_KEYMAP_EEPROM_MAX_ADDR      (TOTAL_EEPROM_BYTE_COUNT - 1)
#define DYNAMIC_KEYMAP_EEPROM_ADDR          DYNAMIC_KEYMAP_EEPROM_START
#ifdef KEYMAP_IN_CODE_FLASH
#define DYNAMIC_KEYMAP_ENCODER_EEPROM_ADDR  DYNAMIC_KEYMAP_EEPROM_ADDR
#else
#define DYNAMIC_KEYMAP_ENCODER_EEPROM_ADDR  (DYNAMIC_KEYMAP_EEPROM_ADDR + (DYNAMIC_KEYMAP_LAYER_COUNT * MATRIX_ROWS * MATRIX_COLS * 2))
#endif

#ifdef ENCODER_ENABLE
#   define DYNAMIC_TAP_DANCE_EEPROM_ADDR    (DYNAMIC_KEYMAP_ENCODER_EEPROM_ADDR + (DYNAMIC_KEYMAP_LAYER_COUNT * NUM_ENCODERS * 2 * 2))
#else
#   define DYNAMIC_TAP_DANCE_EEPROM_ADDR    DYNAMIC_KEYMAP_ENCODER_EEPROM_ADDR
#endif // ENCODER_ENABLE

#ifdef TAP_DANCE_ENABLE
#   define DYNAMIC_TAP_DANCE_EEPROM_SIZE    (TAP_DANCE_COUNT * 2 * 3)  
#   define DYNAMIC_KEYMAP_MACRO_EEPROM_ADDR (DYNAMIC_TAP_DANCE_EEPROM_ADDR + DYNAMIC_TAP_DANCE_EEPROM_SIZE)
#else
#   define DYNAMIC_KEYMAP_MACRO_EEPROM_ADDR DYNAMIC_TAP_DANCE_EEPROM_ADDR
#endif // TAP_DANCE_ENABLE

#define DYNAMIC_KEYMAP_MACRO_EEPROM_SIZE    (DYNAMIC_KEYMAP_EEPROM_MAX_ADDR - DYNAMIC_KEYMAP_MACRO_EEPROM_ADDR + 1)

// CH47: refuse to build a layout that overflows the 128-byte DataFlash (the original firmware did)
#if (DYNAMIC_KEYMAP_MACRO_EEPROM_ADDR) > (TOTAL_EEPROM_BYTE_COUNT - 4)
#   error "DataFlash layout overflow: keymap + tap dance do not fit in 128 bytes"
#endif

uint16_t dynamic_keymap_get_keycode(uint8_t layer, uint8_t row, uint8_t column);
void     dynamic_keymap_set_keycode(uint8_t layer, uint8_t row, uint8_t column, uint16_t keycode);
#ifdef ENCODER_ENABLE
uint16_t dynamic_keymap_get_encoder(uint8_t layer, uint8_t encoder_id, bool clockwise);
void     dynamic_keymap_set_encoder(uint8_t layer, uint8_t encoder_id, bool clockwise, uint16_t keycode);
#endif // ENCODER_ENABLE
void dynamic_keymap_reset(void);
uint8_t  dynamic_keymap_read_byte(uint16_t offset);
void     dynamic_keymap_write_byte(uint16_t offset, uint8_t value);
#ifdef KEYMAP_IN_CODE_FLASH
bool     keymap_flash_valid(void);
void     keymap_flash_set_valid(void);
#endif
