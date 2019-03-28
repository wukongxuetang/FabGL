/*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - <http://www.fabgl.com>
  Copyright (c) 2019 Fabrizio Di Vittorio.
  All rights reserved.

  This file is part of FabGL Library.

  FabGL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabGL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "Arduino.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

#include "keyboard.h"
#include "ps2controller.h"


fabgl::KeyboardClass Keyboard;


#define KBD_CMD_SETLEDS                      0xED
#define KBD_CMD_ECHO                         0xEE
#define KBD_CMD_GETSET_CURRENT_SCANCODE_SET  0xF0
#define KBD_CMD_IDENTIFY                     0xF2
#define KBD_CMD_SET_TYPEMATIC_RATE_AND_DELAY 0xF3
#define KBD_CMD_ENABLE_SCANNING              0xF4
#define KBD_CMD_DISABLE_SCANNING             0xF5
#define KBD_CMD_SET_DEFAULT_PARAMS           0xF6
#define KBD_CMD_RESEND_LAST_BYTE             0xFE
#define KBD_CMD_RESET                        0xFF

#define KBD_REPLY_ERROR1                     0x00
#define KBD_REPLY_ERROR2                     0xFF
#define KBD_REPLY_SELFTEST_OK                0xAA
#define KBD_REPLY_ECHO                       0xEE
#define KBD_REPLY_ACK                        0xFA
#define KBD_REPLY_SELFTEST_FAILED1           0xFC
#define KBD_REPLY_SELFTEST_FAILED2           0xFD
#define KBD_REPLY_RESEND                     0xFE

#define KBD_CMD_RETRY_COUNT                     3
#define KBD_CMD_TIMEOUT                        10



namespace fabgl {



/**************************************************************************************/
/* US LAYOUT                                                                          */
/**************************************************************************************/
const KeyboardLayout USLayout {
  // name
  "US",

  // inherited layout
  NULL,

  // single byte scancodes
  {
    { 0x76, VK_ESCAPE },
    { 0x05, VK_F1 },
    { 0x06, VK_F2 },
    { 0x04, VK_F3 },
    { 0x0C, VK_F4 },
    { 0x03, VK_F5 },
    { 0x0B, VK_F6 },
    { 0x83, VK_F7 },
    { 0x0A, VK_F8 },
    { 0x01, VK_F9 },
    { 0x09, VK_F10 },
    { 0x78, VK_F11 },
    { 0x07, VK_F12 },
    { 0x16, VK_1 },
    { 0x1E, VK_2 },
    { 0x26, VK_3 },
    { 0x25, VK_4 },
    { 0x2E, VK_5 },
    { 0x36, VK_6 },
    { 0x3D, VK_7 },
    { 0x3E, VK_8 },
    { 0x46, VK_9 },
    { 0x45, VK_0 },
    { 0x15, VK_q },
    { 0x1D, VK_w },
    { 0x24, VK_e },
    { 0x2D, VK_r },
    { 0x2C, VK_t },
    { 0x35, VK_y },
    { 0x3C, VK_u },
    { 0x43, VK_i },
    { 0x44, VK_o },
    { 0x4D, VK_p },
    { 0x1C, VK_a },
    { 0x1b, VK_s },
    { 0x23, VK_d },
    { 0x2b, VK_f },
    { 0x34, VK_g },
    { 0x33, VK_h },
    { 0x3B, VK_j },
    { 0x42, VK_k },
    { 0x4B, VK_l },
    { 0x1A, VK_z },
    { 0x22, VK_x },
    { 0x21, VK_c },
    { 0x2A, VK_v },
    { 0x32, VK_b },
    { 0x31, VK_n },
    { 0x3A, VK_m },
    { 0x0E, VK_GRAVEACCENT },
    { 0x4E, VK_MINUS },
    { 0x55, VK_EQUALS },
    { 0x54, VK_LEFTBRACKET },
    { 0x5B, VK_RIGHTBRACKET },
    { 0x5D, VK_BACKSLASH },
    { 0x4C, VK_SEMICOLON },
    { 0x52, VK_QUOTE },
    { 0x41, VK_COMMA },
    { 0x49, VK_PERIOD },
    { 0x4A, VK_SLASH },
    { 0x70, VK_KP_INSERT },
    { 0x69, VK_KP_END },
    { 0x72, VK_KP_DOWN },
    { 0x7A, VK_KP_PAGEDOWN },
    { 0x6B, VK_KP_LEFT },
    { 0x73, VK_KP_CENTER }, // "5" in the keypad
    { 0x74, VK_KP_RIGHT },
    { 0x6C, VK_KP_HOME },
    { 0x75, VK_KP_UP },
    { 0x7D, VK_KP_PAGEUP },
    { 0x71, VK_KP_DELETE },
    { 0x7C, VK_KP_MULTIPLY },
    { 0x79, VK_KP_PLUS },
    { 0x7B, VK_KP_MINUS },
    { 0x66, VK_BACKSPACE },
    { 0x0D, VK_TAB },
    { 0x5A, VK_RETURN },
    { 0x77, VK_NUMLOCK },
    { 0x7E, VK_SCROLLLOCK },
    { 0x58, VK_CAPSLOCK },
    { 0x12, VK_LSHIFT },
    { 0x59, VK_RSHIFT },
    { 0x14, VK_LCTRL },
    { 0x11, VK_LALT },
    { 0x29, VK_SPACE },
    { 0x84, VK_SYSREQ },  // ALT + PRINTSCREEN directly translated by the keyboard
  },

  // extended scancodes (0xE0..)
  {
    { 0x14, VK_RCTRL },
    { 0x11, VK_RALT },
    { 0x1F, VK_LGUI },
    { 0x2F, VK_APPLICATION },
    { 0x70, VK_INSERT },
    { 0x71, VK_DELETE },
    { 0x4A, VK_KP_DIVIDE },
    { 0x5A, VK_KP_ENTER },
    { 0x7D, VK_PAGEUP },
    { 0x7A, VK_PAGEDOWN },
    { 0x6C, VK_HOME },
    { 0x69, VK_END },
    { 0x75, VK_UP },
    { 0x72, VK_DOWN },
    { 0x6B, VK_LEFT },
    { 0x74, VK_RIGHT },
    // print screen is composed by "E0 12 E0 7C", here translated as separated VK_PRINTSCREEN1 and VK_PRINTSCREEN2
    // VK_PRINTSCREEN2 is also generated by "CTRL or SHIFT" + "PRINTSCREEN". So pressing PRINTSCREEN both VK_PRINTSCREEN1 and VK_PRINTSCREEN2
    // are generated, while pressing CTRL+PRINTSCREEN or SHIFT+PRINTSCREEN only VK_PRINTSCREEN2 is generated.
    { 0x12, VK_PRINTSCREEN1 },
    { 0x7C, VK_PRINTSCREEN2 },
    // pressing CTRL + PAUSE will generate "E0 7E" instead of normal Pause sequence. Here is translated to VK_BREAK.
    { 0x7E, VK_BREAK },
  },

  // virtual keys generated by other virtual keys combinations
  //  in_key, { CTRL, ALT, SHIFT, CAPSLOCK, NUMLOCK }, out_key
  {
    { VK_1,            { 0, 0, 1, 0, 0 }, VK_EXCLAIM },      // SHIFT "1" = "!"
    { VK_2,            { 0, 0, 1, 0, 0 }, VK_AT },           // SHIFT "2" = "@"
    { VK_3,            { 0, 0, 1, 0, 0 }, VK_HASH },         // SHIFT "3" = "#"
    { VK_4,            { 0, 0, 1, 0, 0 }, VK_DOLLAR },       // SHIFT "4" = "$"
    { VK_5,            { 0, 0, 1, 0, 0 }, VK_PERCENT },      // SHIFT "5" = "%"
    { VK_6,            { 0, 0, 1, 0, 0 }, VK_CARET },        // SHIFT "6" = "^"
    { VK_7,            { 0, 0, 1, 0, 0 }, VK_AMPERSAND },    // SHIFT "7" = "&"
    { VK_8,            { 0, 0, 1, 0, 0 }, VK_ASTERISK },     // SHIFT "8" = "*"
    { VK_9,            { 0, 0, 1, 0, 0 }, VK_LEFTPAREN },    // SHIFT "9" = "("
    { VK_0,            { 0, 0, 1, 0, 0 }, VK_RIGHTPAREN },   // SHIFT "0" = ")"

    { VK_GRAVEACCENT,  { 0, 0, 1, 0, 0 }, VK_TILDE },        // SHIFT "`" = "~"
    { VK_MINUS,        { 0, 0, 1, 0, 0 }, VK_UNDERSCORE },   // SHIFT "-" = "_"
    { VK_EQUALS,       { 0, 0, 1, 0, 0 }, VK_PLUS },         // SHIFT "=" = "+"
    { VK_LEFTBRACKET,  { 0, 0, 1, 0, 0 }, VK_LEFTBRACE },    // SHIFT "[" = "{"
    { VK_RIGHTBRACKET, { 0, 0, 1, 0, 0 }, VK_RIGHTBRACE },   // SHIFT "]" = "}"
    { VK_BACKSLASH,    { 0, 0, 1, 0, 0 }, VK_VERTICALBAR },  // SHIFT "\" = "|"
    { VK_SEMICOLON,    { 0, 0, 1, 0, 0 }, VK_COLON },        // SHIFT ";" = ":"
    { VK_QUOTE,        { 0, 0, 1, 0, 0 }, VK_QUOTEDBL },     // SHIFT "'" = """
    { VK_COMMA,        { 0, 0, 1, 0, 0 }, VK_LESS },         // SHIFT "," = "<"
    { VK_PERIOD,       { 0, 0, 1, 0, 0 }, VK_GREATER },      // SHIFT "." = ">"
    { VK_SLASH,        { 0, 0, 1, 0, 0 }, VK_QUESTION },     // SHIFT "/" = "?"

    // keypad with NUMLOCK enabled
    { VK_KP_INSERT,    { 0, 0, 0, 0, 1 }, VK_KP_0 },         // NUMLOCK + KP INS      = "0"
    { VK_KP_END,       { 0, 0, 0, 0, 1 }, VK_KP_1 },         // NUMLOCK + KP END      = "1"
    { VK_KP_DOWN,      { 0, 0, 0, 0, 1 }, VK_KP_2 },         // NUMLOCK + KP DOWN     = "2"
    { VK_KP_PAGEDOWN,  { 0, 0, 0, 0, 1 }, VK_KP_3 },         // NUMLOCK + KP PAGEDOWN = "3"
    { VK_KP_LEFT,      { 0, 0, 0, 0, 1 }, VK_KP_4 },         // NUMLOCK + KP LEFT     = "4"
    { VK_KP_CENTER,    { 0, 0, 0, 0, 1 }, VK_KP_5 },         // NUMLOCK + KP CENTER   = "5"
    { VK_KP_RIGHT,     { 0, 0, 0, 0, 1 }, VK_KP_6 },         // NUMLOCK + KP RIGHT    = "6"
    { VK_KP_HOME,      { 0, 0, 0, 0, 1 }, VK_KP_7 },         // NUMLOCK + KP HOME     = "7"
    { VK_KP_UP,        { 0, 0, 0, 0, 1 }, VK_KP_8 },         // NUMLOCK + KP UP       = "8"
    { VK_KP_PAGEUP,    { 0, 0, 0, 0, 1 }, VK_KP_9 },         // NUMLOCK + KP PAGEUP   = "9"
    { VK_KP_DELETE,    { 0, 0, 0, 0, 1 }, VK_KP_PERIOD },    // NUMLOCK + KP DELETE   = "."

    // SHIFT or CAPSLOCK "a".."z" = "A".."Z"
    { VK_a,            { 0, 0, 1, 1, 0 }, VK_A },
    { VK_b,            { 0, 0, 1, 1, 0 }, VK_B },
    { VK_c,            { 0, 0, 1, 1, 0 }, VK_C },
    { VK_d,            { 0, 0, 1, 1, 0 }, VK_D },
    { VK_e,            { 0, 0, 1, 1, 0 }, VK_E },
    { VK_f,            { 0, 0, 1, 1, 0 }, VK_F },
    { VK_g,            { 0, 0, 1, 1, 0 }, VK_G },
    { VK_h,            { 0, 0, 1, 1, 0 }, VK_H },
    { VK_i,            { 0, 0, 1, 1, 0 }, VK_I },
    { VK_j,            { 0, 0, 1, 1, 0 }, VK_J },
    { VK_k,            { 0, 0, 1, 1, 0 }, VK_K },
    { VK_l,            { 0, 0, 1, 1, 0 }, VK_L },
    { VK_m,            { 0, 0, 1, 1, 0 }, VK_M },
    { VK_n,            { 0, 0, 1, 1, 0 }, VK_N },
    { VK_o,            { 0, 0, 1, 1, 0 }, VK_O },
    { VK_p,            { 0, 0, 1, 1, 0 }, VK_P },
    { VK_q,            { 0, 0, 1, 1, 0 }, VK_Q },
    { VK_r,            { 0, 0, 1, 1, 0 }, VK_R },
    { VK_s,            { 0, 0, 1, 1, 0 }, VK_S },
    { VK_t,            { 0, 0, 1, 1, 0 }, VK_T },
    { VK_u,            { 0, 0, 1, 1, 0 }, VK_U },
    { VK_v,            { 0, 0, 1, 1, 0 }, VK_V },
    { VK_w,            { 0, 0, 1, 1, 0 }, VK_W },
    { VK_x,            { 0, 0, 1, 1, 0 }, VK_X },
    { VK_y,            { 0, 0, 1, 1, 0 }, VK_Y },
    { VK_z,            { 0, 0, 1, 1, 0 }, VK_Z },
  }
};


/**************************************************************************************/
/* UK LAYOUT                                                                          */
/**************************************************************************************/
const KeyboardLayout UKLayout {
  // name
  "UK",

  // inherited layout
  &USLayout,

  // single byte scancodes
  {
    { 0x5D, VK_HASH },
    { 0x61, VK_BACKSLASH },
  },

  // extended scancodes (0xE0..)
  {
  },

  // virtual keys generated by other virtual keys combinations
  //  in_key, { CTRL, ALT, SHIFT, CAPSLOCK, NUMLOCK }, out_key
  {
    { VK_GRAVEACCENT, { 0, 0, 1, 0, 0 }, VK_NEGATION },     // SHIFT "`" = "¬"
    { VK_2,           { 0, 0, 1, 0, 0 }, VK_QUOTEDBL },     // SHIFT "2" = """
    { VK_3,           { 0, 0, 1, 0, 0 }, VK_POUND },        // SHIFT "3" = "£"
    { VK_QUOTE,       { 0, 0, 1, 0, 0 }, VK_AT },           // SHIFT "'" = "@"
    { VK_HASH,        { 0, 0, 1, 0, 0 }, VK_TILDE },        // SHIFT "#" = "~"
    { VK_BACKSLASH,   { 0, 0, 1, 0, 0 }, VK_VERTICALBAR },  // SHIFT "\" = "|"
  }
};


/**************************************************************************************/
/* GERMAN LAYOUT                                                                      */
/**************************************************************************************/
const KeyboardLayout GermanLayout {
  // name
  "German",

  // inherited layout
  &USLayout,

  // single byte scancodes
  {
    { 0x0E, VK_CARET },
    { 0x4E, VK_ESZETT },
    { 0x55, VK_ACUTEACCENT },
    { 0x54, VK_UMLAUT_u },
    { 0x5B, VK_PLUS },
    { 0x4C, VK_UMLAUT_o },
    { 0x52, VK_UMLAUT_a },
    { 0x5D, VK_HASH },
    { 0x61, VK_LESS },
    { 0x4A, VK_MINUS },
    { 0x35, VK_z },
    { 0x1A, VK_y },
  },

  // extended scancodes (0xE0..)
  {
  },

  // virtual keys generated by other virtual keys combinations
  //  in_key, { CTRL, ALT, SHIFT, CAPSLOCK, NUMLOCK }, out_key
  {
    { VK_CARET,       { 0, 0, 1, 0, 0 }, VK_DEGREE },       // SHIFT "^" = "°"
    { VK_2,           { 0, 0, 1, 0, 0 }, VK_QUOTEDBL },     // SHIFT "2" = """
    { VK_3,           { 0, 0, 1, 0, 0 }, VK_SECTION },      // SHIFT "3" = "§"
    { VK_6,           { 0, 0, 1, 0, 0 }, VK_AMPERSAND },    // SHIFT "6" = "&"
    { VK_7,           { 0, 0, 1, 0, 0 }, VK_SLASH },        // SHIFT "7" = "/"
    { VK_8,           { 0, 0, 1, 0, 0 }, VK_LEFTPAREN },    // SHIFT "8" = "("
    { VK_9,           { 0, 0, 1, 0, 0 }, VK_RIGHTPAREN },   // SHIFT "9" = ")"
    { VK_0,           { 0, 0, 1, 0, 0 }, VK_EQUALS },       // SHIFT "0" = "="
    { VK_ESZETT,      { 0, 0, 1, 0, 0 }, VK_QUESTION },     // SHIFT "ß" = "?"
    { VK_ACUTEACCENT, { 0, 0, 1, 0, 0 }, VK_GRAVEACCENT },  // SHIFT "´" = "`"
    { VK_PLUS,        { 0, 0, 1, 0, 0 }, VK_ASTERISK },     // SHIFT "+" = "*"
    { VK_HASH,        { 0, 0, 1, 0, 0 }, VK_QUOTE },        // SHIFT "#" = "'"
    { VK_LESS,        { 0, 0, 1, 0, 0 }, VK_GREATER },      // SHIFT "<" = ">"
    { VK_COMMA,       { 0, 0, 1, 0, 0 }, VK_SEMICOLON },    // SHIFT "," = ";"
    { VK_PERIOD,      { 0, 0, 1, 0, 0 }, VK_COLON },        // SHIFT "." = ":"
    { VK_MINUS,       { 0, 0, 1, 0, 0 }, VK_UNDERSCORE },   // SHIFT "-" = "_"
    { VK_7,           { 0, 1, 0, 0, 0 }, VK_LEFTBRACE },    // ALT "7"   = "{"
    { VK_8,           { 0, 1, 0, 0, 0 }, VK_LEFTBRACKET },  // ALT "8"   = "["
    { VK_9,           { 0, 1, 0, 0, 0 }, VK_RIGHTBRACKET }, // ALT "9"   = "]"
    { VK_0,           { 0, 1, 0, 0, 0 }, VK_RIGHTBRACE },   // ALT "0"   = "}"
    { VK_ESZETT,      { 0, 1, 0, 0, 0 }, VK_BACKSLASH },    // ALT "ß"   = "\"
    { VK_q,           { 0, 1, 0, 0, 0 }, VK_AT },           // ALT "q"   = "@"
    { VK_e,           { 0, 1, 0, 0, 0 }, VK_EURO },         // ALT "e"   = "€"
    { VK_PLUS,        { 0, 1, 0, 0, 0 }, VK_TILDE },        // ALT "+"   = "~"
    { VK_LESS,        { 0, 1, 0, 0, 0 }, VK_VERTICALBAR },  // ALT "<"   = "|"
  }
};


/**************************************************************************************/
/* ITALIAN LAYOUT                                                                     */
/**************************************************************************************/
const KeyboardLayout ItalianLayout {
  // name
  "Italian",

  // inherited layout
  &USLayout,

  // single byte scancodes
  {
    { 0x0E, VK_BACKSLASH },
    { 0x4E, VK_QUOTE },
    { 0x55, VK_GRAVE_i },
    { 0x54, VK_GRAVE_e },
    { 0x5B, VK_PLUS },
    { 0x4C, VK_GRAVE_o },
    { 0x52, VK_GRAVE_a },
    { 0x5D, VK_GRAVE_u },
    { 0x61, VK_LESS },
    { 0x4A, VK_MINUS },
  },

  // extended scancodes (0xE0..)
  {
  },

  // virtual keys generated by other virtual keys combinations
  //  in_key, { CTRL, ALT, SHIFT, CAPSLOCK, NUMLOCK }, out_key
  {
    { VK_BACKSLASH, { 0, 0, 1, 0, 0 }, VK_VERTICALBAR },  // SHIFT "\" = "|"
    { VK_2,         { 0, 0, 1, 0, 0 }, VK_QUOTEDBL },     // SHIFT "2" = """
    { VK_3,         { 0, 0, 1, 0, 0 }, VK_POUND },        // SHIFT "3" = "£"
    { VK_6,         { 0, 0, 1, 0, 0 }, VK_AMPERSAND },    // SHIFT "6" = "&"
    { VK_7,         { 0, 0, 1, 0, 0 }, VK_SLASH },        // SHIFT "7" = "/"
    { VK_8,         { 0, 0, 1, 0, 0 }, VK_LEFTPAREN },    // SHIFT "8" = "("
    { VK_9,         { 0, 0, 1, 0, 0 }, VK_RIGHTPAREN },   // SHIFT "9" = ")"
    { VK_0,         { 0, 0, 1, 0, 0 }, VK_EQUALS },       // SHIFT "0" = "="
    { VK_QUOTE,     { 0, 0, 1, 0, 0 }, VK_QUESTION },     // SHIFT "'" = "?"
    { VK_GRAVE_i,   { 0, 0, 1, 0, 0 }, VK_CARET },        // SHIFT "ì" = "^"
    { VK_GRAVE_e,   { 0, 0, 1, 0, 0 }, VK_ACUTE_e },      // SHIFT "è" = "é"
    { VK_PLUS,      { 0, 0, 1, 0, 0 }, VK_ASTERISK },     // SHIFT "+" = "*"
    { VK_GRAVE_o,   { 0, 0, 1, 0, 0 }, VK_CEDILLA_c },    // SHIFT "ò" = "ç"
    { VK_GRAVE_a,   { 0, 0, 1, 0, 0 }, VK_DEGREE },       // SHIFT "à" = "°"
    { VK_GRAVE_u,   { 0, 0, 1, 0, 0 }, VK_SECTION },      // SHIFT "ù" = "§"
    { VK_LESS,      { 0, 0, 1, 0, 0 }, VK_GREATER },      // SHIFT "<" = ">"
    { VK_COMMA,     { 0, 0, 1, 0, 0 }, VK_SEMICOLON },    // SHIFT "," = ";"
    { VK_PERIOD,    { 0, 0, 1, 0, 0 }, VK_COLON },        // SHIFT "." = ":"
    { VK_MINUS,     { 0, 0, 1, 0, 0 }, VK_UNDERSCORE },   // SHIFT "-" = "_"
    { VK_BACKSLASH, { 0, 1, 0, 0, 0 }, VK_GRAVEACCENT },  // ALT "\"   = "`"
    { VK_5,         { 0, 1, 0, 0, 0 }, VK_TILDE },        // ALT "5"   = "~"
    { VK_e,         { 0, 1, 0, 0, 0 }, VK_EURO },         // ALT "e"   = "€"
    { VK_GRAVE_e,   { 0, 1, 0, 0, 0 }, VK_LEFTBRACKET },  // ALT "è"   = "["
    { VK_PLUS,      { 0, 1, 0, 0, 0 }, VK_RIGHTBRACKET }, // ALT "+"   = "]"
    { VK_GRAVE_o,   { 0, 1, 0, 0, 0 }, VK_AT },           // ALT "ò"   = "@"
    { VK_GRAVE_a,   { 0, 1, 0, 0, 0 }, VK_HASH },         // ALT "à"   = "#"
    { VK_GRAVE_e,   { 0, 1, 1, 0, 0 }, VK_LEFTBRACE },    // SHIFT ALT "è" = "{"
    { VK_PLUS,      { 0, 1, 1, 0, 0 }, VK_RIGHTBRACE },   // SHIFT ALT "+" = "}"
  }
};


KeyboardClass::KeyboardClass()
  : m_keyboardAvailable(false)
{
}


void KeyboardClass::begin(bool generateVirtualKeys, bool createVKQueue)
{
  m_CTRL     = false;
  m_ALT      = false;
  m_SHIFT    = false;
  m_CAPSLOCK = false;
  m_NUMLOCK  = false;

  m_numLockLED     = false;
  m_capsLockLED    = false;
  m_scrollLockLED  = false;

  m_SCodeToVKConverterTask = NULL;
  m_virtualKeyQueue        = NULL;

  reset();

  if (generateVirtualKeys || createVKQueue) {
    if (createVKQueue)
      m_virtualKeyQueue = xQueueCreate(FABGLIB_KEYBOARD_VIRTUALKEY_QUEUE_SIZE, sizeof(uint16_t));
    xTaskCreate(&SCodeToVKConverterTask, "", FABGLIB_SCODETOVK_TASK_STACK_SIZE, this, FABGLIB_SCODETOVK_TASK_PRIORITY, &m_SCodeToVKConverterTask);
  }
}


void KeyboardClass::begin(gpio_num_t clkGPIO, gpio_num_t dataGPIO, bool generateVirtualKeys, bool createVKQueue)
{
  PS2Controller.begin(clkGPIO, dataGPIO);
  begin(generateVirtualKeys, createVKQueue);
}


// reset keyboard, set scancode 2 and US layout
bool KeyboardClass::reset()
{
  // sets default layout
  Keyboard.setLayout(&USLayout);

  // tries up to six times for keyboard reset
  for (int i = 0; i < 6; ++i) {
    m_keyboardAvailable = send_cmdReset() && send_cmdSetScancodeSet(2);
    if (m_keyboardAvailable)
      break;
    delay(500);
  }

  return m_keyboardAvailable;
}


void KeyboardClass::getLEDs(bool * numLock, bool * capsLock, bool * scrollLock)
{
  *numLock    = m_numLockLED;
  *capsLock   = m_capsLockLED;
  *scrollLock = m_scrollLockLED;
}


int KeyboardClass::scancodeAvailable()
{
  return PS2Controller.dataAvailable();
}


int KeyboardClass::getNextScancode(int timeOutMS, bool requestResendOnTimeOut)
{
  while (true) {
    int r = PS2Controller.getData(timeOutMS);
    if (r == -1 && requestResendOnTimeOut) {
      PS2Controller.sendData(KBD_CMD_RESEND_LAST_BYTE);
      continue;
    }
    return r;
  }
}


int KeyboardClass::getReplyCode(int timeOutMS)
{
  return PS2Controller.getData(timeOutMS, true);
}


bool KeyboardClass::send(uint8_t cmd, uint8_t expectedReply)
{
  for (int i = 0; i < KBD_CMD_RETRY_COUNT; ++i) {
    PS2Controller.sendData(cmd);
    if (getReplyCode(KBD_CMD_TIMEOUT) != expectedReply)
      continue;
    return true;
  }
  return false;
}


bool KeyboardClass::send_cmdLEDs(bool numLock, bool capsLock, bool scrollLock)
{
  if (!send(KBD_CMD_SETLEDS, KBD_REPLY_ACK))
    return false;
  bool ret = send((scrollLock ? 1 : 0) | (numLock ? 2 : 0) | (capsLock ? 4 : 0), KBD_REPLY_ACK);
  m_numLockLED    = numLock;
  m_capsLockLED   = capsLock;
  m_scrollLockLED = scrollLock;
  return ret;
}


bool KeyboardClass::send_cmdEcho()
{
  return send(KBD_CMD_ECHO, KBD_REPLY_ECHO);
}


bool KeyboardClass::send_cmdGetScancodeSet(uint8_t * result)
{
  if (!send(KBD_CMD_GETSET_CURRENT_SCANCODE_SET, KBD_REPLY_ACK))
    return false;
  if (!send(0, KBD_REPLY_ACK))
    return false;
  *result = getReplyCode(KBD_CMD_TIMEOUT);
  return (*result >= 1 || *result <= 3);
}


bool KeyboardClass::send_cmdSetScancodeSet(uint8_t scancodeSet)
{
  if (!send(KBD_CMD_GETSET_CURRENT_SCANCODE_SET, KBD_REPLY_ACK))
    return false;
  return send(scancodeSet, KBD_REPLY_ACK);
}


// return value is always valid
bool KeyboardClass::send_cmdIdentify(PS2Device * result)
{
  *result = PS2Device::UnknownPS2Device;
  if (!send_cmdDisableScanning())
    return false;
  if (!send(KBD_CMD_IDENTIFY, KBD_REPLY_ACK))
    return false;
  int b1 = getReplyCode(KBD_CMD_TIMEOUT);
  int b2 = getReplyCode(KBD_CMD_TIMEOUT);
  if (b1 == -1 && b2 == -1)
    *result = PS2Device::OldATKeyboard;
  else if (b1 == 0x00 && b2 == -1)
    *result = PS2Device::MouseStandard;
  else if (b1 == 0x03 && b2 == -1)
    *result = PS2Device::MouseWithScrollWheel;
  else if (b1 == 0x04 && b2 == -1)
    *result = PS2Device::Mouse5Buttons;
  else if ((b1 == 0xAB && b2 == 0x41) || (b1 == 0xAB && b2 == 0xC1))
    *result = PS2Device::MF2KeyboardWithTranslation;
  else if (b1 == 0xAB && b2 == 0x83)
    *result = PS2Device::M2Keyboard;
  return send_cmdEnableScanning();
}


bool KeyboardClass::send_cmdDisableScanning()
{
  return send(KBD_CMD_DISABLE_SCANNING, KBD_REPLY_ACK);
}


bool KeyboardClass::send_cmdEnableScanning()
{
  return send(KBD_CMD_ENABLE_SCANNING, KBD_REPLY_ACK);
}


const int16_t REPEATRATES[32] = { 33,  37,  41,  45,  50,  54,  58,  62,  66,  75,  83,  91,
                                 100, 108, 125, 125, 133, 149, 166, 181, 200, 217, 232, 250,
                                 270, 303, 333, 370, 400, 434, 476, 500};


// repeatRateMS  :  33 ms ...  500 ms (in steps as above REPEATRATES table)
// repeatDelayMS : 250 ms ... 1000 ms (in steps of 250 ms)
bool KeyboardClass::send_cmdTypematicRateAndDelay(int repeatRateMS, int repeatDelayMS)
{
  if (!send(KBD_CMD_SET_TYPEMATIC_RATE_AND_DELAY, KBD_REPLY_ACK))
    return false;
  uint8_t byteToSend = 0b01011; // default repeat rate 10.9 characters per seconds (91ms)
  for (int i = 0; i < 32; ++i)
    if (REPEATRATES[i] >= repeatRateMS) {
      byteToSend = i;
      break;
    }
  byteToSend |= (repeatDelayMS / 250 - 1) << 5;
  return send(byteToSend, KBD_REPLY_ACK);
}


bool KeyboardClass::send_cmdSetDefaultParams()
{
  return send(KBD_CMD_SET_DEFAULT_PARAMS, KBD_REPLY_ACK);
}


bool KeyboardClass::send_cmdReset()
{
  if (!send(KBD_CMD_RESET, KBD_REPLY_ACK))
    return false;
  return getReplyCode(500) == KBD_REPLY_SELFTEST_OK; // timout 500ms should be enough for keyboard reset and selft test
}


void KeyboardClass::updateLEDs()
{
  send_cmdLEDs(m_NUMLOCK, m_CAPSLOCK, m_SCROLLLOCK);
}


void KeyboardClass::setLayout(const KeyboardLayout * layout)
{
  m_layout = layout;
}


#if FABGLIB_HAS_VirtualKeyO_STRING
char const * KeyboardClass::virtualKeyToString(VirtualKey virtualKey)
{
  char const * VKTOSTR[] = { "VK_NONE", "VK_SPACE", "VK_0", "VK_1", "VK_2", "VK_3", "VK_4", "VK_5", "VK_6", "VK_7", "VK_8", "VK_9", "VK_KP_0", "VK_KP_1", "VK_KP_2",
                             "VK_KP_3", "VK_KP_4", "VK_KP_5", "VK_KP_6", "VK_KP_7", "VK_KP_8", "VK_KP_9", "VK_a", "VK_b", "VK_c", "VK_d", "VK_e", "VK_f", "VK_g", "VK_h",
                             "VK_i", "VK_j", "VK_k", "VK_l", "VK_m", "VK_n", "VK_o", "VK_p", "VK_q", "VK_r", "VK_s", "VK_t", "VK_u", "VK_v", "VK_w", "VK_x", "VK_y", "VK_z",
                             "VK_A", "VK_B", "VK_C", "VK_D", "VK_E", "VK_F", "VK_G", "VK_H", "VK_I", "VK_J", "VK_K", "VK_L", "VK_M", "VK_N", "VK_O", "VK_P", "VK_Q", "VK_R",
                             "VK_S", "VK_T", "VK_U", "VK_V", "VK_W", "VK_X", "VK_Y", "VK_Z", "VK_GRAVEACCENT", "VK_ACUTEACCENT", "VK_QUOTE", "VK_QUOTEDBL", "VK_EQUALS", "VK_MINUS", "VK_KP_MINUS",
                             "VK_PLUS", "VK_KP_PLUS", "VK_KP_MULTIPLY", "VK_ASTERISK", "VK_BACKSLASH", "VK_KP_DIVIDE", "VK_SLASH", "VK_KP_PERIOD", "VK_PERIOD", "VK_COLON",
                             "VK_COMMA", "VK_SEMICOLON", "VK_AMPERSAND", "VK_VERTICALBAR", "VK_HASH", "VK_AT", "VK_CARET", "VK_DOLLAR", "VK_POUND", "VK_EURO", "VK_PERCENT",
                             "VK_EXCLAIM", "VK_QUESTION", "VK_LEFTBRACE", "VK_RIGHTBRACE", "VK_LEFTBRACKET", "VK_RIGHTBRACKET", "VK_LEFTPAREN", "VK_RIGHTPAREN", "VK_LESS",
                             "VK_GREATER", "VK_UNDERSCORE", "VK_DEGREE", "VK_SECTION", "VK_TILDE", "VK_NEGATION", "VK_LSHIFT", "VK_RSHIFT", "VK_LALT", "VK_RALT", "VK_LCTRL", "VK_RCTRL",
                             "VK_LGUI", "VK_RGUI", "VK_ESCAPE", "VK_PRINTSCREEN1", "VK_PRINTSCREEN2", "VK_SYSREQ", "VK_INSERT", "VK_KP_INSERT", "VK_DELETE", "VK_KP_DELETE", "VK_BACKSPACE", "VK_HOME", "VK_KP_HOME", "VK_END", "VK_KP_END", "VK_PAUSE", "VK_BREAK",
                             "VK_SCROLLLOCK", "VK_NUMLOCK", "VK_CAPSLOCK", "VK_TAB", "VK_RETURN", "VK_KP_ENTER", "VK_APPLICATION", "VK_PAGEUP", "VK_KP_PAGEUP", "VK_PAGEDOWN", "VK_KP_PAGEDOWN", "VK_UP", "VK_KP_UP",
                             "VK_DOWN", "VK_KP_DOWN", "VK_LEFT", "VK_KP_LEFT", "VK_RIGHT", "VK_KP_RIGHT", "VK_KP_CENTER", "VK_F1", "VK_F2", "VK_F3", "VK_F4", "VK_F5", "VK_F6", "VK_F7", "VK_F8", "VK_F9", "VK_F10", "VK_F11", "VK_F12",
                             "VK_GRAVE_a", "VK_GRAVE_e", "VK_ACUTE_e", "VK_GRAVE_i", "VK_GRAVE_o", "VK_GRAVE_u", "VK_CEDILLA_c", "VK_ESZETT", "VK_UMLAUT_u",
                             "VK_UMLAUT_o", "VK_UMLAUT_a" };
  return VKTOSTR[virtualKey];
}
#endif


// -1 = virtual key cannot be translated to ASCII
int KeyboardClass::virtualKeyToASCII(VirtualKey virtualKey)
{
  switch (virtualKey) {
    case VK_SPACE:
      return m_CTRL ? ASCII_NUL : ASCII_SPC;   // CTRL + SPACE = NUL, otherwise 0x20

    case VK_0 ... VK_9:
      return virtualKey - VK_0 + '0';

    case VK_KP_0 ... VK_KP_9:
      return virtualKey - VK_KP_0 + '0';

    case VK_a ... VK_z:
      return virtualKey - VK_a + (m_CTRL ? ASCII_SOH : 'a');  // CTRL + letter = SOH (a) ...SUB (z), otherwise the lower letter

    case VK_A ... VK_Z:
      return virtualKey - VK_A + (m_CTRL ? ASCII_SOH : 'A');  // CTRL + letter = SOH (A) ...SUB (Z), otherwise the lower letter

    case VK_GRAVE_a:
      return 0xE0;  // à

    case VK_GRAVE_e:
      return 0xE8;  // è

    case VK_ACUTE_e:
      return 0xE9;  // é

    case VK_GRAVE_i:
      return 0xEC;  // ì

    case VK_GRAVE_o:
      return 0xF2;  // ò

    case VK_GRAVE_u:
      return 0xF9;  // ù

    case VK_CEDILLA_c:
      return 0xE7;  // ç

    case VK_ESZETT:
      return 0xDF;  // ß

    case VK_UMLAUT_u:
      return 0xFC;  // ü

    case VK_UMLAUT_o:
      return 0xF6;  // ö

    case VK_UMLAUT_a:
      return 0xE4;  // ä

    case VK_GRAVEACCENT:
      return 0x60;  // "`"

    case VK_ACUTEACCENT:
      return 0xB4;  // "´"

    case VK_QUOTE:
      return '\'';  // "'"

    case VK_QUOTEDBL:
      return '"';

    case VK_EQUALS:
      return '=';

    case VK_MINUS:
    case VK_KP_MINUS:
      return '-';

    case VK_PLUS:
    case VK_KP_PLUS:
      return '+';

    case VK_KP_MULTIPLY:
    case VK_ASTERISK:
      return '*';

    case VK_BACKSLASH:
      return m_CTRL ? ASCII_FS : '\\';  // CTRL \ = FS, otherwise '\'

    case VK_KP_DIVIDE:
    case VK_SLASH:
      return '/';

    case VK_KP_PERIOD:
    case VK_PERIOD:
      return '.';

    case VK_COLON:
      return ':';

    case VK_COMMA:
      return ',';

    case VK_SEMICOLON:
      return ';';

    case VK_AMPERSAND:
      return '&';

    case VK_VERTICALBAR:
      return '|';

    case VK_HASH:
      return '#';

    case VK_AT:
      return '@';

    case VK_CARET:
      return '^';

    case VK_DOLLAR:
      return '$';

    case VK_POUND:
      return 0xA3;  // '£'

    case VK_EURO:
      return 0xEE;  // '€' - non 8 bit ascii

    case VK_PERCENT:
      return '%';

    case VK_EXCLAIM:
      return '!';

    case VK_QUESTION:
      return m_CTRL ? ASCII_US : '?'; // CTRL ? = US, otherwise '?'

    case VK_LEFTBRACE:
      return '{';

    case VK_RIGHTBRACE:
      return '}';

    case VK_LEFTBRACKET:
      return m_CTRL ? ASCII_ESC : '['; // CTRL [ = ESC, otherwise '['

    case VK_RIGHTBRACKET:
      return m_CTRL ? ASCII_GS : ']'; // CTRL ] = GS, otherwise ']'

    case VK_LEFTPAREN:
      return '(';

    case VK_RIGHTPAREN:
      return ')';

    case VK_LESS:
      return '<';

    case VK_GREATER:
      return '>';

    case VK_UNDERSCORE:
      return '_';

    case VK_DEGREE:
      return 0xB0;  // '°'

    case VK_SECTION:
      return 0xA7;  // "§"

    case VK_TILDE:
      return m_CTRL ? ASCII_RS : '~';   // CTRL ~ = RS, otherwise "~"

    case VK_NEGATION:
      return 0xAC;  // "¬"

    case VK_BACKSPACE:
      return ASCII_BS;

    case VK_DELETE:
    case VK_KP_DELETE:
      return ASCII_DEL;

    case VK_RETURN:
    case VK_KP_ENTER:
      return ASCII_CR;

    case VK_TAB:
      return ASCII_HT;

    case VK_ESCAPE:
      return ASCII_ESC;

    case VK_SCROLLLOCK:
      return m_SCROLLLOCK ? ASCII_XOFF : ASCII_XON;

    default:
      return -1;
  }
}


VirtualKey KeyboardClass::scancodeToVK(uint8_t scancode, bool isExtended, KeyboardLayout const * layout)
{
  VirtualKey vk = VK_NONE;

  if (layout == NULL)
    layout = m_layout;

  VirtualKeyDef const * def = isExtended ? layout->exScancodeToVK : layout->scancodeToVK;
  for (; def->scancode; ++def)
    if (def->scancode == scancode) {
      vk = def->virtualKey;
      break;
    }

  if (vk == VK_NONE && layout->inherited)
    vk = scancodeToVK(scancode, isExtended, layout->inherited);

  return vk;
}


VirtualKey KeyboardClass::VKtoAlternateVK(VirtualKey in_vk, KeyboardLayout const * layout)
{
  VirtualKey vk = VK_NONE;

  if (layout == NULL)
    layout = m_layout;

  for (AltVirtualKeyDef const * def = layout->alternateVK; def->reqVirtualKey != VK_NONE; ++def) {
    if (def->reqVirtualKey == in_vk && def->ctrl == m_CTRL &&
                                       def->alt == m_ALT &&
                                       (def->shift == m_SHIFT || (def->capslock && def->capslock == m_CAPSLOCK)) &&
                                       def->numlock == m_NUMLOCK) {
      vk = def->virtualKey;
      break;
    }
  }

  if (vk == VK_NONE && layout->inherited)
    vk = VKtoAlternateVK(in_vk, layout->inherited);

  return vk == VK_NONE ? in_vk : vk;
}


VirtualKey KeyboardClass::blockingGetVirtualKey(bool * keyDown)
{
  VirtualKey vk = VK_NONE;
  bool kdown = true;
  do {
    int scode = getNextScancode();
    if (scode == 0xE0) {
      // two bytes scancode
      scode = getNextScancode(100, true);
      if (scode == 0xF0) {
        // two bytes scancode key up
        scode = getNextScancode(100, true);
        vk = scancodeToVK(scode, true);
        kdown = false;
      } else {
        // two bytes scancode key down
        vk = scancodeToVK(scode, true);
      }
    } else if (scode == 0xE1) {
      // special case: "PAUSE" : 0xE1, 0x14, 0x77, 0xE1, 0xF0, 0x14, 0xF0, 0x77
      const uint8_t PAUSECODES[] = {0x14, 0x77, 0xE1, 0xF0, 0x14, 0xF0, 0x77};
      int i;
      for (i = 0; i < 7; ++i) {
        scode = getNextScancode(100, true);
        if (scode != PAUSECODES[i])
          break;
      }
      if (i == 7)
        vk = VK_PAUSE;
    } else if (scode == 0xF0) {
      // key up
      scode = getNextScancode(100, true);
      vk = scancodeToVK(scode, false);
      kdown = false;
    } else {
      // one byte scancode key down
      vk = scancodeToVK(scode, false);
    }
  } while (false);

  if (vk != VK_NONE) {

    // alternate VK (virtualkeys modified by shift, alt, ...)
    vk = VKtoAlternateVK(vk);

    // update shift, alt, ctrl, capslock, numlock and scrollock states and LEDs
    switch (vk) {
      case VK_LCTRL:
      case VK_RCTRL:
        m_CTRL = kdown;
        break;
      case VK_LALT:
      case VK_RALT:
        m_ALT = kdown;
        break;
      case VK_LSHIFT:
      case VK_RSHIFT:
        m_SHIFT = kdown;
        break;
      case VK_CAPSLOCK:
        if (!kdown) // state changed at keyup
          m_CAPSLOCK = !m_CAPSLOCK;
        updateLEDs();
        break;
      case VK_NUMLOCK:
        if (!kdown) // state changed at keyup
          m_NUMLOCK = !m_NUMLOCK;
        updateLEDs();
        break;
      case VK_SCROLLLOCK:
        if (!kdown) // state changed at keyup
          m_SCROLLLOCK = !m_SCROLLLOCK;
        updateLEDs();
        break;
      default:
        break;
    }

  }

  if (keyDown)
    *keyDown = kdown;

  return vk;
}


void KeyboardClass::SCodeToVKConverterTask(void * pvParameters)
{
  while (true) {
    bool keyDown;
    VirtualKey vk = Keyboard.blockingGetVirtualKey(&keyDown);
    if (keyDown)
      Keyboard.m_VKMap[(int)vk >> 3] |= 1 << ((int)vk & 7);
    else
      Keyboard.m_VKMap[(int)vk >> 3] &= ~(1 << ((int)vk & 7));
    if (Keyboard.m_virtualKeyQueue) {
      uint16_t code = (uint16_t)vk | (keyDown ? 0x8000 : 0);
      xQueueSendToBack(Keyboard.m_virtualKeyQueue, &code, portMAX_DELAY);
    }
  }
}


void KeyboardClass::suspendVirtualKeyGeneration(bool value)
{
  if (value)
    vTaskSuspend(m_SCodeToVKConverterTask);
  else
    vTaskResume(m_SCodeToVKConverterTask);
}


bool KeyboardClass::isVKDown(VirtualKey virtualKey)
{
  return m_VKMap[(int)virtualKey >> 3] & (1 << ((int)virtualKey & 7));
}


VirtualKey KeyboardClass::getNextVirtualKey(bool * keyDown, int timeOutMS)
{
  VirtualKey vk = VK_NONE;
  if (m_SCodeToVKConverterTask) {
    uint16_t code;
    if (xQueueReceive(m_virtualKeyQueue, &code, timeOutMS < 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeOutMS)) == pdTRUE) {
      vk = (VirtualKey) (code & 0x7FFF);
      if (keyDown)
        *keyDown = code & 0x8000;
    }
  }
  return vk;
}


int KeyboardClass::virtualKeyAvailable()
{
  return m_virtualKeyQueue ? uxQueueMessagesWaiting(m_virtualKeyQueue) : 0;
}



} // end of namespace
