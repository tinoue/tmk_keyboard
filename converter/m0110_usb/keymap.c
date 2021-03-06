/*
Copyright 2011,2012 Jun Wako <wakojun@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/* M0110A Support was contributed by skagon@github */

#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include "usb_keyboard.h"
#include "keycode.h"
#include "print.h"
#include "debug.h"
#include "util.h"
#include "keymap.h"


/* 
 * The keymap works with both M0110 and M0110A keyboards. As you can see, the M0110A is a superset
 * of the M0110 keyboard, with only one exception: 'Enter' in M0110 does not exist
 * on the M0110A, but since it generates a unique scan code which is not used for some other key in
 * the M0110A, they are totally interchangeable.  In fact, the M0110A is functionally (almost)
 * identical to the combination of the M0110 along with the M0120 keypad. The only difference
 * (which is causing some problems as you will read below) is that the M0110+M0120 don't have
 * dedicated arrow keys, while the M0110A does. However, the M0120 did have arrow keys, which
 * doubled as the [comma], [/], [*] and [+] keys, when used with the [Shift] key. The M0110A has
 * substituted the [comma] key with the [=] key, however its scancode is the same.
 *
 * Physical layout:
 * M0110A
 * ,---------------------------------------------------------. ,---------------.
 * |  `|  1|  2|  3|  4|  5|  6|  7|  8|  9|  0|  -|  =|Bcksp| |Clr|  =|  /|  *|
 * |---------------------------------------------------------| |---------------|
 * |Tab  |  Q|  W|  E|  R|  T|  Y|  U|  I|  O|  P|  [|  ]|   | |  7|  8|  9|  -|
 * |-----------------------------------------------------'   | |---------------|
 * |Caps  |  A|  S|  D|  F|  G|  H|  J|  K|  L|  ;|  '|Return| |  4|  5|  6|  +|
 * |---------------------------------------------------------| |---------------|
 * |Shift   |  Z|  X|  C|  V|  B|  N|  M|  ,|  ,|  /|Shft|Up | |  1|  2|  3|   |
 * |---------------------------------------------------------' |-----------|Ent|
 * |Opt  |Mac    |           Space           |  \|Lft|Rgt|Dn | |      0|  .|   |
 * `---------------------------------------------------------' `---------------'
 *
 * M0110                                                       M0120
 * ,---------------------------------------------------------. ,---------------.
 * |  `|  1|  2|  3|  4|  5|  6|  7|  8|  9|  0|  -|  =|Backs| |Clr|  -|  +|  *|
 * |---------------------------------------------------------| |---------------|
 * |Tab  |  Q|  W|  E|  R|  T|  Y|  U|  I|  O|  P|  [|  ]|  \| |  7|  8|  9|  /|
 * |---------------------------------------------------------| |---------------|
 * |Caps  |  A|  S|  D|  F|  G|  H|  J|  K|  L|  ;|  '|Return| |  4|  5|  6|  ,|
 * |---------------------------------------------------------| |---------------|
 * |Shift   |  Z|  X|  C|  V|  B|  N|  M|  ,|  ,|  /|Shift   | |  1|  2|  3|   |
 * `---------------------------------------------------------' |-----------|Ent|
 *      |Opt|Mac |         Space               |Ent |Opt|      |      0|  .|   |
 *      `-----------------------------------------------'      `---------------'
 * With Shift keys on M0120 work as curosor.(-:Left *:Right /:Up ,:Down)
 *
 * NOTE: \ is located differently.
 * NOTE: Enter on M0110 is different from Enter on keypad(M0120 and M0110A).
 * NOTE: Left Shift and right Shift are logically same key.
 * NOTE: Left Option and right Option are logically same key.
 */

/* Keymap definition Macro
 * ,---------------------------------------------------------. ,---------------.
 * |  `|  1|  2|  3|  4|  5|  6|  7|  8|  9|  0|  -|  =|Backs| |Clr|  =|  /|  *|
 * |---------------------------------------------------------| |---------------|
 * |Tab  |  Q|  W|  E|  R|  T|  Y|  U|  I|  O|  P|  [|  ]|   | |  7|  8|  9|  -|
 * |-----------------------------------------------------'   | |---------------|
 * |Caps  |  A|  S|  D|  F|  G|  H|  J|  K|  L|  ;|  '|Return| |  4|  5|  6|  +|
 * |---------------------------------------------------------| |---------------|
 * |Shift   |  Z|  X|  C|  V|  B|  N|  M|  ,|  ,|  /|Shft|Up | |  1|  2|  3|   |
 * |---------------------------------------------------------| |-----------|Ent|
 * |Opt  |Mac    |         Space         |Ent|  \|Lft|Rgt|Dn | |      0|  .|   |
 * `---------------------------------------------------------' `---------------'
 * NOTE: Ent between Space and \ means Enter on M0110.
 */
#define KEYMAP( \
    K32,K12,K13,K14,K15,K17,K16,K1A,K1C,K19,K1D,K1B,K18,K33,  K47,K68,K6D,K62, \
    K30,K0C,K0D,K0E,K0F,K11,K10,K20,K22,K1F,K23,K21,K1E,      K59,K5B,K5C,K4E, \
    K39,K00,K01,K02,K03,K05,K04,K26,K28,K25,K29,K27,    K24,  K56,K57,K58,K66, \
    K38,K06,K07,K08,K09,K0B,K2D,K2E,K2B,K2F,K2C,        K4D,  K53,K54,K55,K4C, \
    K3A,K37,            K31,            K34,K2A,K46,K42,K48,  K52,    K41      \
) { \
    { KC_##K00, KC_##K01, KC_##K02, KC_##K03, KC_##K04, KC_##K05, KC_##K06, KC_##K07 }, \
    { KC_##K08, KC_##K09, KC_NO,    KC_##K0B, KC_##K0C, KC_##K0D, KC_##K0E, KC_##K0F }, \
    { KC_##K10, KC_##K11, KC_##K12, KC_##K13, KC_##K14, KC_##K15, KC_##K16, KC_##K17 }, \
    { KC_##K18, KC_##K19, KC_##K1A, KC_##K1B, KC_##K1C, KC_##K1D, KC_##K1E, KC_##K1F }, \
    { KC_##K20, KC_##K21, KC_##K22, KC_##K23, KC_##K24, KC_##K25, KC_##K26, KC_##K27 }, \
    { KC_##K28, KC_##K29, KC_##K2A, KC_##K2B, KC_##K2C, KC_##K2D, KC_##K2E, KC_##K2F }, \
    { KC_##K30, KC_##K31, KC_##K32, KC_##K33, KC_##K34, KC_NO,    KC_NO,    KC_##K37 }, \
    { KC_##K38, KC_##K39, KC_##K3A, KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_NO    }, \
    { KC_NO,    KC_##K41, KC_##K42, KC_NO,    KC_NO,    KC_NO,    KC_##K46, KC_##K47 }, \
    { KC_##K48, KC_NO,    KC_NO,    KC_NO,    KC_##K4C, KC_##K4D, KC_##K4E, KC_NO    }, \
    { KC_NO,    KC_NO,    KC_##K52, KC_##K53, KC_##K54, KC_##K55, KC_##K56, KC_##K57 }, \
    { KC_##K58, KC_##K59, KC_NO,    KC_##K5B, KC_##K5C, KC_NO,    KC_NO,    KC_NO    }, \
    { KC_NO,    KC_NO,    KC_##K62, KC_NO,    KC_NO,    KC_NO,    KC_##K66, KC_NO    }, \
    { KC_##K68, KC_NO,    KC_NO,    KC_NO,    KC_NO,    KC_##K6D, KC_NO,    KC_NO    }, \
}

#define KEYCODE(layer, row, col) (pgm_read_byte(&keymaps[(layer)][(row)][(col)]))


// Assign Fn key(0-7) to a layer to which switch with the Fn key pressed.
static const uint8_t PROGMEM fn_layer[] = {
    1,              // Fn0
    0,              // Fn1
    0,              // Fn2
    0,              // Fn3
    0,              // Fn4
    0,              // Fn5
    0,              // Fn6
    0               // Fn7
};

// Assign Fn key(0-7) to a keycode sent when release Fn key without use of the layer.
// See layer.c for details.
static const uint8_t PROGMEM fn_keycode[] = {
    KC_NO,          // Fn0
    KC_NO,          // Fn1
    KC_NO,          // Fn2
    KC_NO,          // Fn3
    KC_NO,          // Fn4
    KC_NO,          // Fn5
    KC_NO,          // Fn6
    KC_NO           // Fn7
};

static const uint8_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Default:
     * ,---------------------------------------------------------. ,---------------.
     * |  `|  1|  2|  3|  4|  5|  6|  7|  8|  9|  0|  -|  =|Backs| |Ctl|  =|  /|  *|
     * |---------------------------------------------------------| |---------------|
     * |Tab  |  Q|  W|  E|  R|  T|  Y|  U|  I|  O|  P|  [|  ]|   | |  7|  8|  9|  -|
     * |-----------------------------------------------------'   | |---------------|
     * |Fn0   |  A|  S|  D|  F|  G|  H|  J|  K|  L|  ;|  '|Return| |  4|  5|  6|  +|
     * |---------------------------------------------------------| |---------------|
     * |Shift   |  Z|  X|  C|  V|  B|  N|  M|  ,|  ,|  /|Shft|Up | |  1|  2|  3|   |
     * |---------------------------------------------------------| |-----------|Ent|
     * |Alt  |Gui    |         Space         |Ctl|  \|Lft|Rgt|Dn | |      0|  .|   |
     * `---------------------------------------------------------' `---------------'
     */
    KEYMAP(
    GRV, 1,   2,   3,   4,   5,   6,   7,   8,   9,   0,   MINS,EQL, BSPC,    LCTL,EQL, PSLS,PAST,
    TAB, Q,   W,   E,   R,   T,   Y,   U,   I,   O,   P,   LBRC,RBRC,         P7,  P8,  P9,  PMNS,
    FN0, A,   S,   D,   F,   G,   H,   J,   K,   L,   SCLN,QUOT,     ENT,     P4,  P5,  P6,  PPLS,
    LSFT,Z,   X,   C,   V,   B,   N,   M,   COMM,DOT, SLSH,          UP,      P1,  P2,  P3,  PENT,
    LALT,LGUI,               SPC,                LCTL,BSLS,LEFT,RGHT,DOWN,    P0,       PDOT
    ),
    /* Cursor Layer(WASD, IJKL)
     * ,---------------------------------------------------------. ,---------------.
     * |Esc| F1| F2| F3| F4| F5| F6| F7| F8| F9|F10|F11|F12|Delet| |Nlk|  =|  /|  *|
     * |---------------------------------------------------------| |---------------|
     * |Caps |Hom| Up|PgU|   |   |   |PgU| Up|Hom|Psc|Slk|Pau|   | |  7|  8|  9|  -|
     * |-----------------------------------------------------'   | |---------------|
     * |Fn0   |Lef|Dow|Rig|   |   |   |Lef|Dow|Rig|   |   |Return| |  4|  5|  6|  +|
     * |---------------------------------------------------------| |---------------|
     * |Shift   |End|   |PgD|   |   |   |PgD|   |End|   |Shif|PgU| |  1|  2|  3|   |
     * |---------------------------------------------------------| |-----------|Ent|
     * |Alt  |Gui    |         Space        |Gui |Ins|Hom|End|PgD| |      0|  .|   |
     * `---------------------------------------------------------' `---------------'
     */
    KEYMAP(
    ESC, F1,  F2,  F3,  F4,  F5,  F6,  F7,  F8,  F9,  F10, F11, F12, DEL,     NLCK,EQL, PSLS,PAST,
    CAPS,HOME,UP,  PGUP,NO,  NO,  NO,  PGUP,UP,  HOME,PSCR,SLCK,PAUS,         P7,  P8,  P9,  PMNS,
    FN0, LEFT,DOWN,RGHT,NO,  NO,  NO,  LEFT,DOWN,RGHT,NO,  NO,       ENT,     P4,  P5,  P6,  PPLS,
    LSFT,END, NO,  PGDN,NO,  NO,  NO,  PGDN,NO,  END, NO,            PGUP,    P1,  P2,  P3,  PENT,
    LALT,LGUI,               SPC,                LCTL,INS, HOME,END, PGDN,    P0,       PDOT
    ),
};


uint8_t keymap_get_keycode(uint8_t layer, uint8_t row, uint8_t col)
{
    return KEYCODE(layer, row, col);
}

uint8_t keymap_fn_layer(uint8_t index)
{
    return pgm_read_byte(&fn_layer[index]);
}

uint8_t keymap_fn_keycode(uint8_t index)
{
    return pgm_read_byte(&fn_keycode[index]);
}
