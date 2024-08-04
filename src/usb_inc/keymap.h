#pragma once

#include <stdint.h>

// Keyboard Modifiers
enum KeyboardMods {
    MOD_LEFT_CTRL = (1 << 8),
    MOD_LEFT_SHIFT = (1 << 9),
    MOD_LEFT_ALT = (1 << 10),
    MOD_LEFT_GUI = (1 << 11),
    MOD_RIGHT_CTRL = (1 << 12),
    MOD_RIGHT_SHIFT = (1 << 13),
    MOD_RIGHT_ALT = (1 << 14),
    MOD_RIGHT_GUI = (uint16_t)(1 << 15),
};

// Keyboard Leds
enum KeyboardLeds {
    LED_NUM_LOCK = (1 << 0),
    LED_CAPS_LOCK = (1 << 1),
    LED_SCROLL_LOCK = (1 << 2),
    LED_COMPOSE = (1 << 3),
    LED_KANA = (1 << 4),
    LED_POWER = (1 << 5),
    LED_SHIFT = (1 << 6),
    LED_DO_NOT_DISTURB = (1 << 7),
};

enum KeyboardKeycode {
    KEY_RESERVED = 0,
    KEY_ERROR_ROLLOVER = 1,
    KEY_POST_FAIL = 2,
    KEY_ERROR_UNDEFINED = 3,
    KEY_A = 4,
    KEY_B = 5,
    KEY_C = 6,
    KEY_D = 7,
    KEY_E = 8,
    KEY_F = 9,
    KEY_G = 10,
    KEY_H = 11,
    KEY_I = 12,
    KEY_J = 13,
    KEY_K = 14,
    KEY_L = 15,
    KEY_M = 16,
    KEY_N = 17,
    KEY_O = 18,
    KEY_P = 19,
    KEY_Q = 20,
    KEY_R = 21,
    KEY_S = 22,
    KEY_T = 23,
    KEY_U = 24,
    KEY_V = 25,
    KEY_W = 26,
    KEY_X = 27,
    KEY_Y = 28,
    KEY_Z = 29,
    KEY_1 = 30,
    KEY_2 = 31,
    KEY_3 = 32,
    KEY_4 = 33,
    KEY_5 = 34,
    KEY_6 = 35,
    KEY_7 = 36,
    KEY_8 = 37,
    KEY_9 = 38,
    KEY_0 = 39,
    KEY_ENTER = 40,
    KEY_RETURN = 40,  // Alias
    KEY_ESC = 41,
    KEY_BACKSPACE = 42,
    KEY_TAB = 43,
    KEY_SPACE = 44,
    KEY_MINUS = 45,
    KEY_EQUAL = 46,
    KEY_LEFT_BRACE = 47,
    KEY_RIGHT_BRACE = 48,
    KEY_BACKSLASH = 49,
    KEY_NON_US_NUM = 50,
    KEY_SEMICOLON = 51,
    KEY_QUOTE = 52,
    KEY_TILDE = 53,
    KEY_COMMA = 54,
    KEY_PERIOD = 55,
    KEY_SLASH = 56,
    KEY_CAPS_LOCK = 0x39,
    KEY_F1 = 0x3A,
    KEY_F2 = 0x3B,
    KEY_F3 = 0x3C,
    KEY_F4 = 0x3D,
    KEY_F5 = 0x3E,
    KEY_F6 = 0x3F,
    KEY_F7 = 0x40,
    KEY_F8 = 0x41,
    KEY_F9 = 0x42,
    KEY_F10 = 0x43,
    KEY_F11 = 0x44,
    KEY_F12 = 0x45,
    KEY_PRINT = 0x46,
    KEY_PRINTSCREEN = 0x46,  // Alias
    KEY_SCROLL_LOCK = 0x47,
    KEY_PAUSE = 0x48,
    KEY_INSERT = 0x49,
    KEY_HOME = 0x4A,
    KEY_PAGE_UP = 0x4B,
    KEY_DELETE = 0x4C,
    KEY_END = 0x4D,
    KEY_PAGE_DOWN = 0x4E,
    KEY_RIGHT_ARROW = 0x4F,
    KEY_LEFT_ARROW = 0x50,
    KEY_DOWN_ARROW = 0x51,
    KEY_UP_ARROW = 0x52,
    KEY_RIGHT = 0x4F,  // Alias
    KEY_LEFT = 0x50,   // Alias
    KEY_DOWN = 0x51,   // Alias
    KEY_UP = 0x52,     // Alias
    KEY_NUM_LOCK = 0x53,
    KEYPAD_DIVIDE = 0x54,
    KEYPAD_MULTIPLY = 0x55,
    KEYPAD_SUBTRACT = 0x56,
    KEYPAD_ADD = 0x57,
    KEYPAD_ENTER = 0x58,
    KEYPAD_1 = 0x59,
    KEYPAD_2 = 0x5A,
    KEYPAD_3 = 0x5B,
    KEYPAD_4 = 0x5C,
    KEYPAD_5 = 0x5D,
    KEYPAD_6 = 0x5E,
    KEYPAD_7 = 0x5F,
    KEYPAD_8 = 0x60,
    KEYPAD_9 = 0x61,
    KEYPAD_0 = 0x62,
    KEYPAD_DOT = 0x63,
    KEY_NON_US = 0x64,
    KEY_APPLICATION = 0x65,  // Context menu/right click
    KEY_MENU = 0x65,         // Alias

    // Most of the following keys will only work with Linux or not at all.
    // F13+ keys are mostly used for laptop functions like ECO key.
    KEY_POWER = 0x66,       // PowerOff (Ubuntu)
    KEY_PAD_EQUALS = 0x67,  // Dont confuse with KEYPAD_EQUAL_SIGN
    KEY_F13 = 0x68,         // Tools (Ubunutu)
    KEY_F14 = 0x69,         // Launch5 (Ubuntu)
    KEY_F15 = 0x6A,         // Launch6 (Ubuntu)
    KEY_F16 = 0x6B,         // Launch7 (Ubuntu)
    KEY_F17 = 0x6C,         // Launch8 (Ubuntu)
    KEY_F18 = 0x6D,         // Launch9 (Ubuntu)
    KEY_F19 = 0x6E,         // Disabled (Ubuntu)
    KEY_F20 = 0x6F,         // AudioMicMute (Ubuntu)
    KEY_F21 = 0x70,         // Touchpad toggle (Ubuntu)
    KEY_F22 = 0x71,         // TouchpadOn (Ubuntu)
    KEY_F23 = 0x72,         // TouchpadOff Ubuntu)
    KEY_F24 = 0x73,         // Disabled (Ubuntu)
    KEY_EXECUTE = 0x74,     // Open (Ubuntu)
    KEY_HELP = 0x75,        // Help (Ubuntu)
    KEY_MENU2 = 0x76,       // Disabled (Ubuntu)
    KEY_SELECT = 0x77,      // Disabled (Ubuntu)
    KEY_STOP = 0x78,        // Cancel (Ubuntu)
    KEY_AGAIN = 0x79,       // Redo (Ubuntu)
    KEY_UNDO = 0x7A,        // Undo (Ubuntu)
    KEY_CUT = 0x7B,         // Cut (Ubuntu)
    KEY_COPY = 0x7C,        // Copy (Ubuntu)
    KEY_PASTE = 0x7D,       // Paste (Ubuntu)
    KEY_FIND = 0x7E,        // Find (Ubuntu)
    KEY_MUTE = 0x7F,
    KEY_VOLUME_MUTE = 0x7F,  // Alias
    KEY_VOLUME_UP = 0x80,
    KEY_VOLUME_DOWN = 0x81,
    KEY_LOCKING_CAPS_LOCK = 0x82,    // Disabled (Ubuntu)
    KEY_LOCKING_NUM_LOCK = 0x83,     // Disabled (Ubuntu)
    KEY_LOCKING_SCROLL_LOCK = 0x84,  // Disabled (Ubuntu)
    KEYPAD_COMMA = 0x85,             // .
    KEYPAD_EQUAL_SIGN = 0x86,        // Disabled (Ubuntu), Dont confuse with KEYPAD_EQUAL
    KEY_INTERNATIONAL1 = 0x87,       // Disabled (Ubuntu)
    KEY_INTERNATIONAL2 = 0x88,       // Hiragana Katakana (Ubuntu)
    KEY_INTERNATIONAL3 = 0x89,       // Disabled (Ubuntu)
    KEY_INTERNATIONAL4 = 0x8A,       // Henkan (Ubuntu)
    KEY_INTERNATIONAL5 = 0x8B,       // Muhenkan (Ubuntu)
    KEY_INTERNATIONAL6 = 0x8C,       // Disabled (Ubuntu)
    KEY_INTERNATIONAL7 = 0x8D,       // Disabled (Ubuntu)
    KEY_INTERNATIONAL8 = 0x8E,       // Disabled (Ubuntu)
    KEY_INTERNATIONAL9 = 0x8F,       // Disabled (Ubuntu)
    KEY_LANG1 = 0x90,                // Disabled (Ubuntu)
    KEY_LANG2 = 0x91,                // Disabled (Ubuntu)
    KEY_LANG3 = 0x92,                // Katana (Ubuntu)
    KEY_LANG4 = 0x93,                // Hiragana (Ubuntu)
    KEY_LANG5 = 0x94,                // Disabled (Ubuntu)
    KEY_LANG6 = 0x95,                // Disabled (Ubuntu)
    KEY_LANG7 = 0x96,                // Disabled (Ubuntu)
    KEY_LANG8 = 0x97,                // Disabled (Ubuntu)
    KEY_LANG9 = 0x98,                // Disabled (Ubuntu)
    KEY_ALTERNATE_ERASE = 0x99,      // Disabled (Ubuntu)
    KEY_SYSREQ_ATTENTION = 0x9A,     // Disabled (Ubuntu)
    KEY_CANCEL = 0x9B,               // Disabled (Ubuntu)
    KEY_CLEAR = 0x9C,                // Delete (Ubuntu)
    KEY_PRIOR = 0x9D,                // Disabled (Ubuntu)
    KEY_RETURN2 = 0x9E,              // Disabled (Ubuntu), Do not confuse this with KEY_ENTER
    KEY_SEPARATOR = 0x9F,            // Disabled (Ubuntu)
    KEY_OUT = 0xA0,                  // Disabled (Ubuntu)
    KEY_OPER = 0xA1,                 // Disabled (Ubuntu)
    KEY_CLEAR_AGAIN = 0xA2,          // Disabled (Ubuntu)
    KEY_CRSEL_PROPS = 0xA3,          // Disabled (Ubuntu)
    KEY_EXSEL = 0xA4,                // Disabled (Ubuntu)

    KEY_PAD_00 = 0xB0,                // Disabled (Ubuntu)
    KEY_PAD_000 = 0xB1,               // Disabled (Ubuntu)
    KEY_THOUSANDS_SEPARATOR = 0xB2,   // Disabled (Ubuntu)
    KEY_DECIMAL_SEPARATOR = 0xB3,     // Disabled (Ubuntu)
    KEY_CURRENCY_UNIT = 0xB4,         // Disabled (Ubuntu)
    KEY_CURRENCY_SUB_UNIT = 0xB5,     // Disabled (Ubuntu)
    KEYPAD_LEFT_BRACE = 0xB6,         // (
    KEYPAD_RIGHT_BRACE = 0xB7,        // )
    KEYPAD_LEFT_CURLY_BRACE = 0xB8,   // Disabled (Ubuntu)
    KEYPAD_RIGHT_CURLY_BRACE = 0xB9,  // Disabled (Ubuntu)
    KEYPAD_TAB = 0xBA,                // Disabled (Ubuntu)
    KEYPAD_BACKSPACE = 0xBB,          // Disabled (Ubuntu)
    KEYPAD_A = 0xBC,                  // Disabled (Ubuntu)
    KEYPAD_B = 0xBD,                  // Disabled (Ubuntu)
    KEYPAD_C = 0xBE,                  // Disabled (Ubuntu)
    KEYPAD_D = 0xBF,                  // Disabled (Ubuntu)
    KEYPAD_E = 0xC0,                  // Disabled (Ubuntu)
    KEYPAD_F = 0xC1,                  // Disabled (Ubuntu)
    KEYPAD_XOR = 0xC2,                // Disabled (Ubuntu)
    KEYPAD_CARET = 0xC3,              // Disabled (Ubuntu)
    KEYPAD_PERCENT = 0xC4,            // Disabled (Ubuntu)
    KEYPAD_LESS_THAN = 0xC5,          // Disabled (Ubuntu)
    KEYPAD_GREATER_THAN = 0xC6,       // Disabled (Ubuntu)
    KEYPAD_AMPERSAND = 0xC7,          // Disabled (Ubuntu)
    KEYPAD_DOUBLEAMPERSAND = 0xC8,    // Disabled (Ubuntu)
    KEYPAD_PIPE = 0xC9,               // Disabled (Ubuntu)
    KEYPAD_DOUBLEPIPE = 0xCA,         // Disabled (Ubuntu)
    KEYPAD_COLON = 0xCB,              // Disabled (Ubuntu)
    KEYPAD_POUND_SIGN = 0xCC,         // Disabled (Ubuntu)
    KEYPAD_SPACE = 0xCD,              // Disabled (Ubuntu)
    KEYPAD_AT_SIGN = 0xCE,            // Disabled (Ubuntu)
    KEYPAD_EXCLAMATION_POINT = 0xCF,  // Disabled (Ubuntu)
    KEYPAD_MEMORY_STORE = 0xD0,       // Disabled (Ubuntu)
    KEYPAD_MEMORY_RECALL = 0xD1,      // Disabled (Ubuntu)
    KEYPAD_MEMORY_CLEAR = 0xD2,       // Disabled (Ubuntu)
    KEYPAD_MEMORY_ADD = 0xD3,         // Disabled (Ubuntu)
    KEYPAD_MEMORY_SUBTRACT = 0xD4,    // Disabled (Ubuntu)
    KEYPAD_MEMORY_MULTIPLY = 0xD5,    // Disabled (Ubuntu)
    KEYPAD_MEMORY_DIVIDE = 0xD6,      // Disabled (Ubuntu)
    KEYPAD_PLUS_MINUS = 0xD7,         // Disabled (Ubuntu)
    KEYPAD_CLEAR = 0xD8,              // Delete (Ubuntu)
    KEYPAD_CLEAR_ENTRY = 0xD9,        // Disabled (Ubuntu)
    KEYPAD_BINARY = 0xDA,             // Disabled (Ubuntu)
    KEYPAD_OCTAL = 0xDB,              // Disabled (Ubuntu)
    KEYPAD_DECIMAL = 0xDC,            // Disabled (Ubuntu)
    KEYPAD_HEXADECIMAL = 0xDD,        // Disabled (Ubuntu)

    KEY_LEFT_CTRL = 0xE0,
    KEY_LEFT_SHIFT = 0xE1,
    KEY_LEFT_ALT = 0xE2,
    KEY_LEFT_GUI = 0xE3,
    KEY_LEFT_WINDOWS = 0xE3,  // Alias
    KEY_RIGHT_CTRL = 0xE4,
    KEY_RIGHT_SHIFT = 0xE5,
    KEY_RIGHT_ALT = 0xE6,
    KEY_RIGHT_GUI = 0xE7,
    KEY_RIGHT_WINDOWS = 0xE7,  // Alias

    // Keyboard HID mappings

    //  Reserved (no_event_indicated)
    HID_KEYBOARD_ERROR_ROLLOVER = 0x01,
    HID_KEYBOARD_POST_FAIL = 0x02,
    HID_KEYBOARD_ERROR_UNDEFINED = 0x03,
    HID_KEYBOARD_A_AND_A = 0x04,
    HID_KEYBOARD_B_AND_B = 0x05,
    HID_KEYBOARD_C_AND_C = 0x06,
    HID_KEYBOARD_D_AND_D = 0x07,
    HID_KEYBOARD_E_AND_E = 0x08,
    HID_KEYBOARD_F_AND_F = 0x09,
    HID_KEYBOARD_G_AND_G = 0x0A,
    HID_KEYBOARD_H_AND_H = 0x0B,
    HID_KEYBOARD_I_AND_I = 0x0C,
    HID_KEYBOARD_J_AND_J = 0x0D,
    HID_KEYBOARD_K_AND_K = 0x0E,
    HID_KEYBOARD_L_AND_L = 0x0F,
    HID_KEYBOARD_M_AND_M = 0x10,
    HID_KEYBOARD_N_AND_N = 0x11,
    HID_KEYBOARD_O_AND_O = 0x12,
    HID_KEYBOARD_P_AND_P = 0x13,
    HID_KEYBOARD_Q_AND_Q = 0x14,
    HID_KEYBOARD_R_AND_R = 0x15,
    HID_KEYBOARD_S_AND_S = 0x16,
    HID_KEYBOARD_T_AND_T = 0x17,
    HID_KEYBOARD_U_AND_U = 0x18,
    HID_KEYBOARD_V_AND_V = 0x19,
    HID_KEYBOARD_W_AND_W = 0x1A,
    HID_KEYBOARD_X_AND_X = 0x1B,
    HID_KEYBOARD_Y_AND_Y = 0x1C,
    HID_KEYBOARD_Z_AND_Z = 0x1D,
    HID_KEYBOARD_1_AND_EXCLAMATION_POINT = 0x1E,
    HID_KEYBOARD_2_AND_AT = 0x1F,
    HID_KEYBOARD_3_AND_POUND = 0x20,
    HID_KEYBOARD_4_AND_DOLLAR = 0x21,
    HID_KEYBOARD_5_AND_PERCENT = 0x22,
    HID_KEYBOARD_6_AND_CARAT = 0x23,
    HID_KEYBOARD_7_AND_AMPERSAND = 0x24,
    HID_KEYBOARD_8_AND_ASTERISK = 0x25,
    HID_KEYBOARD_9_AND_LEFT_PAREN = 0x26,
    HID_KEYBOARD_0_AND_RIGHT_PAREN = 0x27,
    HID_KEYBOARD_ENTER = 0x28,  // (MARKED AS ENTER_SLASH_RETURN)
    HID_KEYBOARD_ESCAPE = 0x29,
    HID_KEYBOARD_DELETE = 0x2A,  // (BACKSPACE)
    HID_KEYBOARD_TAB = 0x2B,
    HID_KEYBOARD_SPACEBAR = 0x2C,
    HID_KEYBOARD_MINUS_AND_UNDERSCORE = 0x2D,  // (UNDERSCORE)
    HID_KEYBOARD_EQUALS_AND_PLUS = 0x2E,
    HID_KEYBOARD_LEFT_BRACKET_AND_LEFT_CURLY_BRACE = 0x2F,
    HID_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_CURLY_BRACE = 0x30,
    HID_KEYBOARD_BACKSLASH_AND_PIPE = 0x31,
    HID_KEYBOARD_NON_US_POUND_AND_TILDE = 0x32,
    HID_KEYBOARD_SEMICOLON_AND_COLON = 0x33,
    HID_KEYBOARD_QUOTE_AND_DOUBLEQUOTE = 0x34,
    HID_KEYBOARD_GRAVE_ACCENT_AND_TILDE = 0x35,
    HID_KEYBOARD_COMMA_AND_LESS_THAN = 0x36,
    HID_KEYBOARD_PERIOD_AND_GREATER_THAN = 0x37,
    HID_KEYBOARD_SLASH_AND_QUESTION_MARK = 0x38,
    HID_KEYBOARD_CAPS_LOCK = 0x39,
    HID_KEYBOARD_F1 = 0x3A,
    HID_KEYBOARD_F2 = 0x3B,
    HID_KEYBOARD_F3 = 0x3C,
    HID_KEYBOARD_F4 = 0x3D,
    HID_KEYBOARD_F5 = 0x3E,
    HID_KEYBOARD_F6 = 0x3F,
    HID_KEYBOARD_F7 = 0x40,
    HID_KEYBOARD_F8 = 0x41,
    HID_KEYBOARD_F9 = 0x42,
    HID_KEYBOARD_F10 = 0x43,
    HID_KEYBOARD_F11 = 0x44,
    HID_KEYBOARD_F12 = 0x45,
    HID_KEYBOARD_PRINTSCREEN = 0x46,
    HID_KEYBOARD_SCROLL_LOCK = 0x47,
    HID_KEYBOARD_PAUSE = 0x48,
    HID_KEYBOARD_INSERT = 0x49,
    HID_KEYBOARD_HOME = 0x4A,
    HID_KEYBOARD_PAGE_UP = 0x4B,
    HID_KEYBOARD_DELETE_FORWARD = 0x4C,
    HID_KEYBOARD_END = 0x4D,
    HID_KEYBOARD_PAGE_DOWN = 0x4E,
    HID_KEYBOARD_RIGHTARROW = 0x4F,
    HID_KEYBOARD_LEFTARROW = 0x50,
    HID_KEYBOARD_DOWNARROW = 0x51,
    HID_KEYBOARD_UPARROW = 0x52,
    HID_KEYPAD_NUM_LOCK_AND_CLEAR = 0x53,
    HID_KEYPAD_DIVIDE = 0x54,
    HID_KEYPAD_MULTIPLY = 0x55,
    HID_KEYPAD_SUBTRACT = 0x56,
    HID_KEYPAD_ADD = 0x57,
    HID_KEYPAD_ENTER = 0x58,
    HID_KEYPAD_1_AND_END = 0x59,
    HID_KEYPAD_2_AND_DOWN_ARROW = 0x5A,
    HID_KEYPAD_3_AND_PAGE_DOWN = 0x5B,
    HID_KEYPAD_4_AND_LEFT_ARROW = 0x5C,
    HID_KEYPAD_5 = 0x5D,
    HID_KEYPAD_6_AND_RIGHT_ARROW = 0x5E,
    HID_KEYPAD_7_AND_HOME = 0x5F,
    HID_KEYPAD_8_AND_UP_ARROW = 0x60,
    HID_KEYPAD_9_AND_PAGE_UP = 0x61,
    HID_KEYPAD_0_AND_INSERT = 0x62,
    HID_KEYPAD_PERIOD_AND_DELETE = 0x63,
    HID_KEYBOARD_NON_US_BACKSLASH_AND_PIPE = 0x64,
    HID_KEYBOARD_APPLICATION = 0x65,
    HID_KEYBOARD_POWER = 0x66,
    HID_KEYPAD_EQUALS = 0x67,
    HID_KEYBOARD_F13 = 0x68,
    HID_KEYBOARD_F14 = 0x69,
    HID_KEYBOARD_F15 = 0x6A,
    HID_KEYBOARD_F16 = 0x6B,
    HID_KEYBOARD_F17 = 0x6C,
    HID_KEYBOARD_F18 = 0x6D,
    HID_KEYBOARD_F19 = 0x6E,
    HID_KEYBOARD_F20 = 0x6F,
    HID_KEYBOARD_F21 = 0x70,
    HID_KEYBOARD_F22 = 0x71,
    HID_KEYBOARD_F23 = 0x72,
    HID_KEYBOARD_F24 = 0x73,
    HID_KEYBOARD_EXECUTE = 0x74,
    HID_KEYBOARD_HELP = 0x75,
    HID_KEYBOARD_MENU = 0x76,
    HID_KEYBOARD_SELECT = 0x77,
    HID_KEYBOARD_STOP = 0x78,
    HID_KEYBOARD_AGAIN = 0x79,
    HID_KEYBOARD_UNDO = 0x7A,
    HID_KEYBOARD_CUT = 0x7B,
    HID_KEYBOARD_COPY = 0x7C,
    HID_KEYBOARD_PASTE = 0x7D,
    HID_KEYBOARD_FIND = 0x7E,
    HID_KEYBOARD_MUTE = 0x7F,
    HID_KEYBOARD_VOLUME_UP = 0x80,
    HID_KEYBOARD_VOLUME_DOWN = 0x81,
    HID_KEYBOARD_LOCKING_CAPS_LOCK = 0x82,
    HID_KEYBOARD_LOCKING_NUM_LOCK = 0x83,
    HID_KEYBOARD_LOCKING_SCROLL_LOCK = 0x84,
    HID_KEYPAD_COMMA = 0x85,
    HID_KEYPAD_EQUAL_SIGN = 0x86,
    HID_KEYBOARD_INTERNATIONAL1 = 0x87,
    HID_KEYBOARD_INTERNATIONAL2 = 0x88,
    HID_KEYBOARD_INTERNATIONAL3 = 0x89,
    HID_KEYBOARD_INTERNATIONAL4 = 0x8A,
    HID_KEYBOARD_INTERNATIONAL5 = 0x8B,
    HID_KEYBOARD_INTERNATIONAL6 = 0x8C,
    HID_KEYBOARD_INTERNATIONAL7 = 0x8D,
    HID_KEYBOARD_INTERNATIONAL8 = 0x8E,
    HID_KEYBOARD_INTERNATIONAL9 = 0x8F,
    HID_KEYBOARD_LANG1 = 0x90,
    HID_KEYBOARD_LANG2 = 0x91,
    HID_KEYBOARD_LANG3 = 0x92,
    HID_KEYBOARD_LANG4 = 0x93,
    HID_KEYBOARD_LANG5 = 0x94,
    HID_KEYBOARD_LANG6 = 0x95,
    HID_KEYBOARD_LANG7 = 0x96,
    HID_KEYBOARD_LANG8 = 0x97,
    HID_KEYBOARD_LANG9 = 0x98,
    HID_KEYBOARD_ALTERNATE_ERASE = 0x99,
    HID_KEYBOARD_SYSREQ_SLASH_ATTENTION = 0x9A,
    HID_KEYBOARD_CANCEL = 0x9B,
    HID_KEYBOARD_CLEAR = 0x9C,
    HID_KEYBOARD_PRIOR = 0x9D,
    HID_KEYBOARD_RETURN = 0x9E,
    HID_KEYBOARD_SEPARATOR = 0x9F,
    HID_KEYBOARD_OUT = 0xA0,
    HID_KEYBOARD_OPER = 0xA1,
    HID_KEYBOARD_CLEAR_SLASH_AGAIN = 0xA2,
    HID_KEYBOARD_CRSEL_SLASH_PROPS = 0xA3,
    HID_KEYBOARD_EXSEL = 0xA4,
    // Reserved 0xA5-AF
    HID_KEYPAD_00 = 0xB0,
    HID_KEYPAD_000 = 0xB1,
    HID_THOUSANDS_SEPARATOR = 0xB2,
    HID_DECIMAL_SEPARATOR = 0xB3,
    HID_CURRENCY_UNIT = 0xB4,
    HID_CURRENCY_SUBUNIT = 0xB5,
    HID_KEYPAD_LEFT_PAREN = 0xB6,
    HID_KEYPAD_RIGHT_PAREN = 0xB7,
    HID_KEYPAD_LEFT_CURLY_BRACE = 0xB8,
    HID_KEYPAD_RIGHT_CURLY_BRACE = 0xB9,
    HID_KEYPAD_TAB = 0xBA,
    HID_KEYPAD_BACKSPACE = 0xBB,
    HID_KEYPAD_A = 0xBC,
    HID_KEYPAD_B = 0xBD,
    HID_KEYPAD_C = 0xBE,
    HID_KEYPAD_D = 0xBF,
    HID_KEYPAD_E = 0xC0,
    HID_KEYPAD_F = 0xC1,
    HID_KEYPAD_XOR = 0xC2,
    HID_KEYPAD_CARAT = 0xC3,
    HID_KEYPAD_PERCENT = 0xC4,
    HID_KEYPAD_LESS_THAN = 0xC5,
    HID_KEYPAD_GREATER_THAN = 0xC6,
    HID_KEYPAD_AMPERSAND = 0xC7,
    HID_KEYPAD_DOUBLEAMPERSAND = 0xC8,
    HID_KEYPAD_PIPE = 0xC9,
    HID_KEYPAD_DOUBLEPIPE = 0xCA,
    HID_KEYPAD_COLON = 0xCB,
    HID_KEYPAD_POUND_SIGN = 0xCC,
    HID_KEYPAD_SPACE = 0xCD,
    HID_KEYPAD_AT_SIGN = 0xCE,
    HID_KEYPAD_EXCLAMATION_POINT = 0xCF,
    HID_KEYPAD_MEMORY_STORE = 0xD0,
    HID_KEYPAD_MEMORY_RECALL = 0xD1,
    HID_KEYPAD_MEMORY_CLEAR = 0xD2,
    HID_KEYPAD_MEMORY_ADD = 0xD3,
    HID_KEYPAD_MEMORY_SUBTRACT = 0xD4,
    HID_KEYPAD_MEMORY_MULTIPLY = 0xD5,
    HID_KEYPAD_MEMORY_DIVIDE = 0xD6,
    HID_KEYPAD_PLUS_SLASH_MINUS = 0xD7,
    HID_KEYPAD_CLEAR = 0xD8,
    HID_KEYPAD_CLEAR_ENTRY = 0xD9,
    HID_KEYPAD_BINARY = 0xDA,
    HID_KEYPAD_OCTAL = 0xDB,
    HID_KEYPAD_DECIMAL = 0xDC,
    HID_KEYPAD_HEXADECIMAL = 0xDD,

    //  0xDE-0xDF    - RESERVED
    HID_KEYBOARD_LEFT_CONTROL = 0xE0,
    HID_KEYBOARD_LEFT_SHIFT = 0xE1,
    HID_KEYBOARD_LEFT_ALT = 0xE2,
    HID_KEYBOARD_LEFT_GUI = 0xE3,
    HID_KEYBOARD_RIGHT_CONTROL = 0xE4,
    HID_KEYBOARD_RIGHT_SHIFT = 0xE5,
    HID_KEYBOARD_RIGHT_ALT = 0xE6,
    HID_KEYBOARD_RIGHT_GUI = 0xE7,
};

static const uint16_t _asciimap[] = {
    KEY_RESERVED,   // NUL
    KEY_RESERVED,   // SOH
    KEY_RESERVED,   // STX
    KEY_RESERVED,   // ETX
    KEY_RESERVED,   // EOT
    KEY_RESERVED,   // ENQ
    KEY_RESERVED,   // ACK
    KEY_RESERVED,   // BEL
    KEY_BACKSPACE,  // BS   Backspace
    KEY_TAB,        // TAB  Tab
    KEY_ENTER,      // LF   Enter
    KEY_RESERVED,   // VT
    KEY_RESERVED,   // FF
    KEY_RESERVED,   // CR
    KEY_RESERVED,   // SO
    KEY_RESERVED,   // SI
    KEY_RESERVED,   // DEL
    KEY_RESERVED,   // DC1
    KEY_RESERVED,   // DC2
    KEY_RESERVED,   // DC3
    KEY_RESERVED,   // DC4
    KEY_RESERVED,   // NAK
    KEY_RESERVED,   // SYN
    KEY_RESERVED,   // ETB
    KEY_RESERVED,   // CAN
    KEY_RESERVED,   // EM
    KEY_RESERVED,   // SUB
    KEY_RESERVED,   // ESC
    KEY_RESERVED,   // FS
    KEY_RESERVED,   // GS
    KEY_RESERVED,   // RS
    KEY_RESERVED,   // US

    KEY_SPACE,                         // ' ' Space
    KEY_1 | MOD_LEFT_SHIFT,            // !
    KEY_QUOTE | MOD_LEFT_SHIFT,        // "
    KEY_3 | MOD_LEFT_SHIFT,            // #
    KEY_4 | MOD_LEFT_SHIFT,            // $
    KEY_5 | MOD_LEFT_SHIFT,            // %
    KEY_7 | MOD_LEFT_SHIFT,            // &
    KEY_QUOTE,                         // '
    KEY_9 | MOD_LEFT_SHIFT,            // (
    KEY_0 | MOD_LEFT_SHIFT,            // )
    KEY_8 | MOD_LEFT_SHIFT,            // *
    KEY_EQUAL | MOD_LEFT_SHIFT,        // +
    KEY_COMMA,                         // ,
    KEY_MINUS,                         // -
    KEY_PERIOD,                        // .
    KEY_SLASH,                         // /
    KEY_0,                             // 0
    KEY_1,                             // 1
    KEY_2,                             // 2
    KEY_3,                             // 3
    KEY_4,                             // 4
    KEY_5,                             // 5
    KEY_6,                             // 6
    KEY_7,                             // 7
    KEY_8,                             // 8
    KEY_9,                             // 9
    KEY_SEMICOLON | MOD_LEFT_SHIFT,    // :
    KEY_SEMICOLON,                     // ;
    KEY_COMMA | MOD_LEFT_SHIFT,        // <
    KEY_EQUAL,                         // =
    KEY_PERIOD | MOD_LEFT_SHIFT,       // >
    KEY_SLASH | MOD_LEFT_SHIFT,        // ?
    KEY_2 | MOD_LEFT_SHIFT,            // @
    KEY_A | MOD_LEFT_SHIFT,            // A
    KEY_B | MOD_LEFT_SHIFT,            // B
    KEY_C | MOD_LEFT_SHIFT,            // C
    KEY_D | MOD_LEFT_SHIFT,            // D
    KEY_E | MOD_LEFT_SHIFT,            // E
    KEY_F | MOD_LEFT_SHIFT,            // F
    KEY_G | MOD_LEFT_SHIFT,            // G
    KEY_H | MOD_LEFT_SHIFT,            // H
    KEY_I | MOD_LEFT_SHIFT,            // I
    KEY_J | MOD_LEFT_SHIFT,            // J
    KEY_K | MOD_LEFT_SHIFT,            // K
    KEY_L | MOD_LEFT_SHIFT,            // L
    KEY_M | MOD_LEFT_SHIFT,            // M
    KEY_N | MOD_LEFT_SHIFT,            // N
    KEY_O | MOD_LEFT_SHIFT,            // O
    KEY_P | MOD_LEFT_SHIFT,            // P
    KEY_Q | MOD_LEFT_SHIFT,            // Q
    KEY_R | MOD_LEFT_SHIFT,            // R
    KEY_S | MOD_LEFT_SHIFT,            // S
    KEY_T | MOD_LEFT_SHIFT,            // T
    KEY_U | MOD_LEFT_SHIFT,            // U
    KEY_V | MOD_LEFT_SHIFT,            // V
    KEY_W | MOD_LEFT_SHIFT,            // W
    KEY_X | MOD_LEFT_SHIFT,            // X
    KEY_Y | MOD_LEFT_SHIFT,            // Y
    KEY_Z | MOD_LEFT_SHIFT,            // Z
    KEY_LEFT_BRACE,                    // [
    KEY_BACKSLASH,                     // bslash
    KEY_RIGHT_BRACE,                   // ]
    KEY_6 | MOD_LEFT_SHIFT,            // ^
    KEY_MINUS | MOD_LEFT_SHIFT,        // _
    KEY_TILDE,                         // `
    KEY_A,                             // a
    KEY_B,                             // b
    KEY_C,                             // c
    KEY_D,                             // d
    KEY_E,                             // e
    KEY_F,                             // f
    KEY_G,                             // g
    KEY_H,                             // h
    KEY_I,                             // i
    KEY_J,                             // j
    KEY_K,                             // k
    KEY_L,                             // l
    KEY_M,                             // m
    KEY_N,                             // n
    KEY_O,                             // o
    KEY_P,                             // p
    KEY_Q,                             // q
    KEY_R,                             // r
    KEY_S,                             // s
    KEY_T,                             // t
    KEY_U,                             // u
    KEY_V,                             // v
    KEY_W,                             // w
    KEY_X,                             // x
    KEY_Y,                             // y
    KEY_Z,                             // z
    KEY_LEFT_BRACE | MOD_LEFT_SHIFT,   // {
    KEY_BACKSLASH | MOD_LEFT_SHIFT,    // |
    KEY_RIGHT_BRACE | MOD_LEFT_SHIFT,  // }
    KEY_TILDE | MOD_LEFT_SHIFT,        // ~
    KEY_RESERVED,                      // DEL
    // 7-bit ASCII codes end here

    // The following characters belong to ISO-8859-15
    // ! The first 16 values here are used for the numpad
    KEYPAD_0, KEYPAD_1, KEYPAD_2, KEYPAD_3, KEYPAD_4, KEYPAD_5, KEYPAD_6, KEYPAD_7, KEYPAD_8,
    KEYPAD_9, KEYPAD_ADD, KEYPAD_SUBTRACT, KEYPAD_MULTIPLY, KEYPAD_DIVIDE, KEYPAD_ENTER, KEYPAD_DOT,
    // KEY_RESERVED,  // 128 - Unused
    // KEY_RESERVED,  // 129 - Unused
    // KEY_RESERVED,  // 130 - Unused
    // KEY_RESERVED,  // 131 - Unused
    // KEY_RESERVED,  // 132 - Unused
    // KEY_RESERVED,  // 133 - Unused
    // KEY_RESERVED,  // 134 - Unused
    // KEY_RESERVED,  // 135 - Unused
    // KEY_RESERVED,  // 136 - Unused
    // KEY_RESERVED,  // 137 - Unused
    // KEY_RESERVED,  // 138 - Unused
    // KEY_RESERVED,  // 139 - Unused
    // KEY_RESERVED,  // 140 - Unused
    // KEY_RESERVED,  // 141 - Unused
    // KEY_RESERVED,  // 142 - Unused
    // KEY_RESERVED,  // 143 - Unused
    KEY_RESERVED,  // 144 - Unused
    KEY_RESERVED,  // 145 - Unused
    KEY_RESERVED,  // 146 - Unused
    KEY_RESERVED,  // 147 - Unused
    KEY_RESERVED,  // 148 - Unused
    KEY_RESERVED,  // 149 - Unused
    KEY_RESERVED,  // 150 - Unused
    KEY_RESERVED,  // 151 - Unused
    KEY_RESERVED,  // 152 - Unused
    KEY_RESERVED,  // 153 - Unused
    KEY_RESERVED,  // 154 - Unused
    KEY_RESERVED,  // 155 - Unused
    KEY_RESERVED,  // 156 - Unused
    KEY_RESERVED,  // 157 - Unused
    KEY_RESERVED,  // 158 - Unused
    KEY_RESERVED,  // 159 - Unused
    KEY_RESERVED,  // 160 - Non-breaking Space
    KEY_RESERVED,  // 161 - Inverted Exclamation Mark
    KEY_RESERVED,  // 162 - Cent
    KEY_RESERVED,  // 163 - British Pound Sign
    KEY_RESERVED,  // 164 - Euro Sign
    KEY_RESERVED,  // 165 - Yen
    KEY_RESERVED,  // 166 - Capital 's' Inverted Circumflex
    KEY_RESERVED,  // 167 - Section Sign
    KEY_RESERVED,  // 168 - 's' Inverted Circumflex
    KEY_RESERVED,  // 169 - Copyright Sign
    KEY_RESERVED,  // 170 - Superscript 'a'
    KEY_RESERVED,  // 171 - Open Guillemet
    KEY_RESERVED,  // 172 - Logic Negation
    KEY_RESERVED,  // 173 - Soft Hypen
    KEY_RESERVED,  // 174 - Registered Trademark
    KEY_RESERVED,  // 175 - Macron
    KEY_RESERVED,  // 176 - Degree Symbol
    KEY_RESERVED,  // 177 - Plus-Minus
    KEY_RESERVED,  // 178 - Superscript '2'
    KEY_RESERVED,  // 179 - Superscript '3'
    KEY_RESERVED,  // 180 - Capital 'z' Inverted Circumflex
    KEY_RESERVED,  // 181 - Micro Symbol
    KEY_RESERVED,  // 182 - Paragraph Mark
    KEY_RESERVED,  // 183 - Interpunct
    KEY_RESERVED,  // 184 - 'z' Inverted Circumflex
    KEY_RESERVED,  // 185 - Superscript '1'
    KEY_RESERVED,  // 186 - Ordinal Indicator
    KEY_RESERVED,  // 187 - Closed Guillemet
    KEY_RESERVED,  // 188 - Capital 'oe'
    KEY_RESERVED,  // 189 - 'oe'
    KEY_RESERVED,  // 190 - Capital 'y' Umlaut
    KEY_RESERVED,  // 191 - Inverted Question Mark
    KEY_RESERVED,  // 192 - Capital 'a' Grave
    KEY_RESERVED,  // 193 - Capital 'a' Acute
    KEY_RESERVED,  // 194 - Capital 'a' Circumflex
    KEY_RESERVED,  // 195 - Capital 'a' Tilde
    KEY_RESERVED,  // 196 - Capital 'a' Umlaut
    KEY_RESERVED,  // 197 - Capital 'a' Circle
    KEY_RESERVED,  // 198 - Capital 'ae'
    KEY_RESERVED,  // 199 - Capital 'c' Cedilla
    KEY_RESERVED,  // 200 - Capital 'e' Grave
    KEY_RESERVED,  // 201 - Capital 'e' Acute
    KEY_RESERVED,  // 202 - Capital 'e' Circumflex
    KEY_RESERVED,  // 203 - Capital 'e' Umlaut
    KEY_RESERVED,  // 204 - Capital 'i' Grave
    KEY_RESERVED,  // 205 - Capital 'i' Acute
    KEY_RESERVED,  // 206 - Capital 'i' Circumflex
    KEY_RESERVED,  // 207 - Capital 'i' Umlaut
    KEY_RESERVED,  // 208 - Capital Eth
    KEY_RESERVED,  // 207 - Capital 'n' Tilde
    KEY_RESERVED,  // 210 - Capital 'o' Grave
    KEY_RESERVED,  // 211 - Capital 'o' Acute
    KEY_RESERVED,  // 212 - Capital 'o' Circumflex
    KEY_RESERVED,  // 213 - Capital 'o' Tilde
    KEY_RESERVED,  // 214 - Capital 'o' Umlaut
    KEY_RESERVED,  // 215 - Multiplication Sign
    KEY_RESERVED,  // 216 - Capital 'o' Barred
    KEY_RESERVED,  // 217 - Capital 'u' Grave
    KEY_RESERVED,  // 218 - Capital 'u' Acute
    KEY_RESERVED,  // 219 - Capital 'u' Circumflex
    KEY_RESERVED,  // 220 - Capital 'u' Umlaut
    KEY_RESERVED,  // 221 - Capital 'y' Acute
    KEY_RESERVED,  // 222 - Capital Thorn
    KEY_RESERVED,  // 223 - Eszett
    KEY_RESERVED,  // 224 - 'a' Grave
    KEY_RESERVED,  // 225 - 'a' Acute
    KEY_RESERVED,  // 226 - 'a' Circumflex
    KEY_RESERVED,  // 227 - 'a' Tilde
    KEY_RESERVED,  // 228 - 'a' Umlaut
    KEY_RESERVED,  // 229 - 'a' Circle
    KEY_RESERVED,  // 230 - 'ae'
    KEY_RESERVED,  // 231 - 'c' Cedilla
    KEY_RESERVED,  // 232 - 'e' Grave
    KEY_RESERVED,  // 233 - 'e' Acute
    KEY_RESERVED,  // 234 - 'e' Circumflex
    KEY_RESERVED,  // 235 - 'e' Umlaut
    KEY_RESERVED,  // 236 - 'i' Grave
    KEY_RESERVED,  // 237 - 'i' Acute
    KEY_RESERVED,  // 238 - 'i' Circumflex
    KEY_RESERVED,  // 239 - 'i' Umlaut
    KEY_RESERVED,  // 240 - Eth
    KEY_RESERVED,  // 241 - 'n' Tilde
    KEY_RESERVED,  // 242 - 'o' Grave
    KEY_RESERVED,  // 243 - 'o' Acute
    KEY_RESERVED,  // 244 - 'o' Circumflex
    KEY_RESERVED,  // 245 - 'o' Tilde
    KEY_RESERVED,  // 246 - 'o' Umlaut
    KEY_RESERVED,  // 247 - Multiplication Sign
    KEY_RESERVED,  // 248 - 'o' Barred
    KEY_RESERVED,  // 249 - 'u' Grave
    KEY_RESERVED,  // 250 - 'u' Acute
    KEY_RESERVED,  // 251 - 'u' Circumflex
    KEY_RESERVED,  // 252 - 'u' Umlaut
    KEY_RESERVED,  // 253 - 'y' Acute
    KEY_RESERVED,  // 254 - Thorn
    KEY_RESERVED,  // 255 - 'y' Umlaut
};

enum ConsumerKeycode : uint16_t {
    // Some keys might only work with linux
    CONSUMER_POWER = 0x30,
    CONSUMER_SLEEP = 0x32,

    MEDIA_RECORD = 0xB2,
    MEDIA_FAST_FORWARD = 0xB3,
    MEDIA_REWIND = 0xB4,
    MEDIA_NEXT = 0xB5,
    MEDIA_PREVIOUS = 0xB6,
    MEDIA_PREV = 0xB6,  // Alias
    MEDIA_STOP = 0xB7,
    MEDIA_PLAY_PAUSE = 0xCD,
    MEDIA_PAUSE = 0xB0,

    MEDIA_VOLUME_MUTE = 0xE2,
    MEDIA_VOL_MUTE = 0xE2,  // Alias
    MEDIA_VOLUME_UP = 0xE9,
    MEDIA_VOL_UP = 0xE9,  // Alias
    MEDIA_VOLUME_DOWN = 0xEA,
    MEDIA_VOL_DOWN = 0xEA,  // Alias

    CONSUMER_BRIGHTNESS_UP = 0x006F,
    CONSUMER_BRIGHTNESS_DOWN = 0x0070,

    CONSUMER_SCREENSAVER = 0x19e,

    CONSUMER_PROGRAMMABLE_BUTTON_CONFIGURATION = 0x182,
    CONSUMER_CONTROL_CONFIGURATION = 0x183,
    CONSUMER_EMAIL_READER = 0x18A,
    CONSUMER_CALCULATOR = 0x192,
    CONSUMER_EXPLORER = 0x194,

    CONSUMER_BROWSER_HOME = 0x223,
    CONSUMER_BROWSER_BACK = 0x224,
    CONSUMER_BROWSER_FORWARD = 0x225,
    CONSUMER_BROWSER_REFRESH = 0x227,
    CONSUMER_BROWSER_BOOKMARKS = 0x22A,

    // Consumer_Page_(0x0C)	0x15
    HID_CONSUMER_UNASSIGNED = 0x00,
    HID_CONSUMER_NUMERIC_KEY_PAD = 0x02,       // HID type NARY
    HID_CONSUMER_PROGRAMMABLE_BUTTONS = 0x03,  // HID type NARY
    HID_CONSUMER_MICROPHONE_CA = 0x04,
    HID_CONSUMER_HEADPHONE_CA = 0x05,
    HID_CONSUMER_GRAPHIC_EQUALIZER_CA = 0x06,
    // Reserved	= 0x07-1F
    HID_CONSUMER_PLUS_10 = 0x20,      // HID type OSC
    HID_CONSUMER_PLUS_100 = 0x21,     // HID type OSC
    HID_CONSUMER_AM_SLASH_PM = 0x22,  // HID type OSC
    // Reserved	= 0x23-3F
    HID_CONSUMER_POWER = 0x30,             // HID type OOC
    HID_CONSUMER_RESET = 0x31,             // HID type OSC
    HID_CONSUMER_SLEEP = 0x32,             // HID type OSC
    HID_CONSUMER_SLEEP_AFTER = 0x33,       // HID type OSC
    HID_CONSUMER_SLEEP_MODE = 0x34,        // HID type RTC
    HID_CONSUMER_ILLUMINATION = 0x35,      // HID type OOC
    HID_CONSUMER_FUNCTION_BUTTONS = 0x36,  // HID type NARY
    // Reserved	= 0x37-3F
    HID_CONSUMER_MENU = 0x40,                 // HID type OOC
    HID_CONSUMER_MENU_PICK = 0x41,            // HID type OSC
    HID_CONSUMER_MENU_UP = 0x42,              // HID type OSC
    HID_CONSUMER_MENU_DOWN = 0x43,            // HID type OSC
    HID_CONSUMER_MENU_LEFT = 0x44,            // HID type OSC
    HID_CONSUMER_MENU_RIGHT = 0x45,           // HID type OSC
    HID_CONSUMER_MENU_ESCAPE = 0x46,          // HID type OSC
    HID_CONSUMER_MENU_VALUE_INCREASE = 0x47,  // HID type OSC
    HID_CONSUMER_MENU_VALUE_DECREASE = 0x48,  // HID type OSC
    // Reserved 0x49-5F
    HID_CONSUMER_DATA_ON_SCREEN = 0x60,         // HID type OOC
    HID_CONSUMER_CLOSED_CAPTION = 0x61,         // HID type OOC
    HID_CONSUMER_CLOSED_CAPTION_SELECT = 0x62,  // HID type OSC
    HID_CONSUMER_VCR_SLASH_TV = 0x63,           // HID type OOC
    HID_CONSUMER_BROADCAST_MODE = 0x64,         // HID type OSC
    HID_CONSUMER_SNAPSHOT = 0x65,               // HID type OSC
    HID_CONSUMER_STILL = 0x66,                  // HID type OSC
    // Reserved 0x67-7F
    HID_CONSUMER_SELECTION = 0x80,                   // HID type NARY
    HID_CONSUMER_ASSIGN_SELECTION = 0x81,            // HID type OSC
    HID_CONSUMER_MODE_STEP = 0x82,                   // HID type OSC
    HID_CONSUMER_RECALL_LAST = 0x83,                 // HID type OSC
    HID_CONSUMER_ENTER_CHANNEL = 0x84,               // HID type OSC
    HID_CONSUMER_ORDER_MOVIE = 0x85,                 // HID type OSC
    HID_CONSUMER_CHANNEL = 0x86,                     // HID type LC
    HID_CONSUMER_MEDIA_SELECTION = 0x87,             // HID type NARY
    HID_CONSUMER_MEDIA_SELECT_COMPUTER = 0x88,       // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_TV = 0x89,             // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_WWW = 0x8A,            // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_DVD = 0x8B,            // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_TELEPHONE = 0x8C,      // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_PROGRAM_GUIDE = 0x8D,  // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_VIDEO_PHONE = 0x8E,    // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_GAMES = 0x8F,          // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_MESSAGES = 0x90,       // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_CD = 0x91,             // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_VCR = 0x92,            // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_TUNER = 0x93,          // HID type SEL
    HID_CONSUMER_QUIT = 0x94,                        // HID type OSC
    HID_CONSUMER_HELP = 0x95,                        // HID type OOC
    HID_CONSUMER_MEDIA_SELECT_TAPE = 0x96,           // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_CABLE = 0x97,          // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_SATELLITE = 0x98,      // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_SECURITY = 0x99,       // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_HOME = 0x9A,           // HID type SEL
    HID_CONSUMER_MEDIA_SELECT_CALL = 0x9B,           // HID type SEL
    HID_CONSUMER_CHANNEL_INCREMENT = 0x9C,           // HID type OSC
    HID_CONSUMER_CHANNEL_DECREMENT = 0x9D,           // HID type OSC
    HID_CONSUMER_MEDIA_SELECT_SAP = 0x9E,            // HID type SEL
    // Reserved 0x9F
    HID_CONSUMER_VCR_PLUS = 0xA0,  // HID type OSC
    HID_CONSUMER_ONCE = 0xA1,      // HID type OSC
    HID_CONSUMER_DAILY = 0xA2,     // HID type OSC
    HID_CONSUMER_WEEKLY = 0xA3,    // HID type OSC
    HID_CONSUMER_MONTHLY = 0xA4,   // HID type OSC
    // Reserved 0xA5-AF
    HID_CONSUMER_PLAY = 0xB0,                 // HID type OOC
    HID_CONSUMER_PAUSE = 0xB1,                // HID type OOC
    HID_CONSUMER_RECORD = 0xB2,               // HID type OOC
    HID_CONSUMER_FAST_FORWARD = 0xB3,         // HID type OOC
    HID_CONSUMER_REWIND = 0xB4,               // HID type OOC
    HID_CONSUMER_SCAN_NEXT_TRACK = 0xB5,      // HID type OSC
    HID_CONSUMER_SCAN_PREVIOUS_TRACK = 0xB6,  // HID type OSC
    HID_CONSUMER_STOP = 0xB7,                 // HID type OSC
    HID_CONSUMER_EJECT = 0xB8,                // HID type OSC
    HID_CONSUMER_RANDOM_PLAY = 0xB9,          // HID type OOC
    HID_CONSUMER_SELECT_DISC = 0xBA,          // HID type NARY
    HID_CONSUMER_ENTER_DISC_MC = 0xBB,
    HID_CONSUMER_REPEAT = 0xBC,                 // HID type OSC
    HID_CONSUMER_TRACKING = 0xBD,               // HID type LC
    HID_CONSUMER_TRACK_NORMAL = 0xBE,           // HID type OSC
    HID_CONSUMER_SLOW_TRACKING = 0xBF,          // HID type LC
    HID_CONSUMER_FRAME_FORWARD = 0xC0,          // HID type RTC
    HID_CONSUMER_FRAME_BACK = 0xC1,             // HID type RTC
    HID_CONSUMER_MARK = 0xC2,                   // HID type OSC
    HID_CONSUMER_CLEAR_MARK = 0xC3,             // HID type OSC
    HID_CONSUMER_REPEAT_FROM_MARK = 0xC4,       // HID type OOC
    HID_CONSUMER_RETURN_TO_MARK = 0xC5,         // HID type OSC
    HID_CONSUMER_SEARCH_MARK_FORWARD = 0xC6,    // HID type OSC
    HID_CONSUMER_SEARCH_MARK_BACKWARDS = 0xC7,  // HID type OSC
    HID_CONSUMER_COUNTER_RESET = 0xC8,          // HID type OSC
    HID_CONSUMER_SHOW_COUNTER = 0xC9,           // HID type OSC
    HID_CONSUMER_TRACKING_INCREMENT = 0xCA,     // HID type RTC
    HID_CONSUMER_TRACKING_DECREMENT = 0xCB,     // HID type RTC
    HID_CONSUMER_STOP_SLASH_EJECT = 0xCC,       // HID type OSC
    HID_CONSUMER_PLAY_SLASH_PAUSE = 0xCD,       // HID type OSC
    HID_CONSUMER_PLAY_SLASH_SKIP = 0xCE,        // HID type OSC
    // Reserved 0xCF-DF
    HID_CONSUMER_VOLUME = 0xE0,            // HID type LC
    HID_CONSUMER_BALANCE = 0xE1,           // HID type LC
    HID_CONSUMER_MUTE = 0xE2,              // HID type OOC
    HID_CONSUMER_BASS = 0xE3,              // HID type LC
    HID_CONSUMER_TREBLE = 0xE4,            // HID type LC
    HID_CONSUMER_BASS_BOOST = 0xE5,        // HID type OOC
    HID_CONSUMER_SURROUND_MODE = 0xE6,     // HID type OSC
    HID_CONSUMER_LOUDNESS = 0xE7,          // HID type OOC
    HID_CONSUMER_MPX = 0xE8,               // HID type OOC
    HID_CONSUMER_VOLUME_INCREMENT = 0xE9,  // HID type RTC
    HID_CONSUMER_VOLUME_DECREMENT = 0xEA,  // HID type RTC
    // Reserved 0xEB-EF
    HID_CONSUMER_SPEED_SELECT = 0xF0,    // HID type OSC
    HID_CONSUMER_PLAYBACK_SPEED = 0xF1,  // HID type NARY
    HID_CONSUMER_STANDARD_PLAY = 0xF2,   // HID type SEL
    HID_CONSUMER_LONG_PLAY = 0xF3,       // HID type SEL
    HID_CONSUMER_EXTENDED_PLAY = 0xF4,   // HID type SEL
    HID_CONSUMER_SLOW = 0xF5,            // HID type OSC
    // Reserved 0xF6-FF
    HID_CONSUMER_FAN_ENABLE = 0x100,                // HID type OOC
    HID_CONSUMER_FAN_SPEED = 0x101,                 // HID type LC
    HID_CONSUMER_LIGHT_ENABLE = 0x102,              // HID type OOC
    HID_CONSUMER_LIGHT_ILLUMINATION_LEVEL = 0x103,  // HID type LC
    HID_CONSUMER_CLIMATE_CONTROL_ENABLE = 0x104,    // HID type OOC
    HID_CONSUMER_ROOM_TEMPERATURE = 0x105,          // HID type LC
    HID_CONSUMER_SECURITY_ENABLE = 0x106,           // HID type OOC
    HID_CONSUMER_FIRE_ALARM = 0x107,                // HID type OSC
    HID_CONSUMER_POLICE_ALARM = 0x108,              // HID type OSC
    HID_CONSUMER_PROXIMITY = 0x109,                 // HID type LC
    HID_CONSUMER_MOTION = 0x10A,                    // HID type OSC
    HID_CONSUMER_DURESS_ALARM = 0x10B,              // HID type OSC
    HID_CONSUMER_HOLDUP_ALARM = 0x10C,              // HID type OSC
    HID_CONSUMER_MEDICAL_ALARM = 0x10D,             // HID type OSC
    // Reserved 0x10E-14F
    HID_CONSUMER_BALANCE_RIGHT = 0x150,     // HID type RTC
    HID_CONSUMER_BALANCE_LEFT = 0x151,      // HID type RTC
    HID_CONSUMER_BASS_INCREMENT = 0x152,    // HID type RTC
    HID_CONSUMER_BASS_DECREMENT = 0x153,    // HID type RTC
    HID_CONSUMER_TREBLE_INCREMENT = 0x154,  // HID type RTC
    HID_CONSUMER_TREBLE_DECREMENT = 0x155,  // HID type RTC
    // Reserved 0x156-15F
    HID_CONSUMER_SPEAKER_SYSTEM = 0x160,                     // HID type CL
    HID_CONSUMER_CHANNEL_LEFT = 0x161,                       // HID type CL
    HID_CONSUMER_CHANNEL_RIGHT = 0x162,                      // HID type CL
    HID_CONSUMER_CHANNEL_CENTER = 0x163,                     // HID type CL
    HID_CONSUMER_CHANNEL_FRONT = 0x164,                      // HID type CL
    HID_CONSUMER_CHANNEL_CENTER_FRONT = 0x165,               // HID type CL
    HID_CONSUMER_CHANNEL_SIDE = 0x166,                       // HID type CL
    HID_CONSUMER_CHANNEL_SURROUND = 0x167,                   // HID type CL
    HID_CONSUMER_CHANNEL_LOW_FREQUENCY_ENHANCEMENT = 0x168,  // HID type CL
    HID_CONSUMER_CHANNEL_TOP = 0x169,                        // HID type CL
    HID_CONSUMER_CHANNEL_UNKNOWN = 0x16A,                    // HID type CL
    // Reserved 0x16B-16F
    HID_CONSUMER_SUB_CHANNEL = 0x170,                // HID type LC
    HID_CONSUMER_SUB_CHANNEL_INCREMENT = 0x171,      // HID type OSC
    HID_CONSUMER_SUB_CHANNEL_DECREMENT = 0x172,      // HID type OSC
    HID_CONSUMER_ALTERNATE_AUDIO_INCREMENT = 0x173,  // HID type OSC
    HID_CONSUMER_ALTERNATE_AUDIO_DECREMENT = 0x174,  // HID type OSC
    // Reserved 0x175-17F
    HID_CONSUMER_APPLICATION_LAUNCH_BUTTONS = 0x180,                 // HID type NARY
    HID_CONSUMER_AL_LAUNCH_BUTTON_CONFIGURATION_TOOL = 0x181,        // HID type SEL
    HID_CONSUMER_AL_PROGRAMMABLE_BUTTON_CONFIGURATION = 0x182,       // HID type SEL
    HID_CONSUMER_AL_CONSUMER_CONTROL_CONFIGURATION = 0x183,          // HID type SEL
    HID_CONSUMER_AL_WORD_PROCESSOR = 0x184,                          // HID type SEL
    HID_CONSUMER_AL_TEXT_EDITOR = 0x185,                             // HID type SEL
    HID_CONSUMER_AL_SPREADSHEET = 0x186,                             // HID type SEL
    HID_CONSUMER_AL_GRAPHICS_EDITOR = 0x187,                         // HID type SEL
    HID_CONSUMER_AL_PRESENTATION_APP = 0x188,                        // HID type SEL
    HID_CONSUMER_AL_DATABASE_APP = 0x189,                            // HID type SEL
    HID_CONSUMER_AL_EMAIL_READER = 0x18A,                            // HID type SEL
    HID_CONSUMER_AL_NEWSREADER = 0x18B,                              // HID type SEL
    HID_CONSUMER_AL_VOICEMAIL = 0x18C,                               // HID type SEL
    HID_CONSUMER_AL_CONTACTS_SLASH_ADDRESS_BOOK = 0x18D,             // HID type SEL
    HID_CONSUMER_AL_CALENDAR_SLASH_SCHEDULE = 0x18E,                 // HID type SEL
    HID_CONSUMER_AL_TASK_SLASH_PROJECT_MANAGER = 0x18F,              // HID type SEL
    HID_CONSUMER_AL_LOG_SLASH_JOURNAL_SLASH_TIMECARD = 0x190,        // HID type SEL
    HID_CONSUMER_AL_CHECKBOOK_SLASH_FINANCE = 0x191,                 // HID type SEL
    HID_CONSUMER_AL_CALCULATOR = 0x192,                              // HID type SEL
    HID_CONSUMER_AL_A_SLASH_V_CAPTURE_SLASH_PLAYBACK = 0x193,        // HID type SEL
    HID_CONSUMER_AL_LOCAL_MACHINE_BROWSER = 0x194,                   // HID type SEL
    HID_CONSUMER_AL_LAN_SLASH_WAN_BROWSER = 0x195,                   // HID type SEL
    HID_CONSUMER_AL_INTERNET_BROWSER = 0x196,                        // HID type SEL
    HID_CONSUMER_AL_REMOTE_NETWORKING_SLASH_ISP_CONNECT = 0x197,     // HID type SEL
    HID_CONSUMER_AL_NETWORK_CONFERENCE = 0x198,                      // HID type SEL
    HID_CONSUMER_AL_NETWORK_CHAT = 0x199,                            // HID type SEL
    HID_CONSUMER_AL_TELEPHONY_SLASH_DIALER = 0x19A,                  // HID type SEL
    HID_CONSUMER_AL_LOGON = 0x19B,                                   // HID type SEL
    HID_CONSUMER_AL_LOGOFF = 0x19C,                                  // HID type SEL
    HID_CONSUMER_AL_LOGON_SLASH_LOGOFF = 0x19D,                      // HID type SEL
    HID_CONSUMER_AL_TERMINAL_LOCK_SLASH_SCREENSAVER = 0x19E,         // HID type SEL
    HID_CONSUMER_AL_CONTROL_PANEL = 0x19F,                           // HID type SEL
    HID_CONSUMER_AL_COMMAND_LINE_PROCESSOR_SLASH_RUN = 0x1A0,        // HID type SEL
    HID_CONSUMER_AL_PROCESS_SLASH_TASK_MANAGER = 0x1A1,              // HID type SEL
    HID_CONSUMER_AL_SELECT_TASK_SLASH_APPLICATION = 0x1A2,           // HID type SEL
    HID_CONSUMER_AL_NEXT_TASK_SLASH_APPLICATION = 0x1A3,             // HID type SEL
    HID_CONSUMER_AL_PREVIOUS_TASK_SLASH_APPLICATION = 0x1A4,         // HID type SEL
    HID_CONSUMER_AL_PREEMPTIVE_HALT_TASK_SLASH_APPLICATION = 0x1A5,  // HID type SEL
    HID_CONSUMER_AL_INTEGRATED_HELP_CENTER = 0x1A6,                  // HID type SEL
    HID_CONSUMER_AL_DOCUMENTS = 0x1A7,                               // HID type SEL
    HID_CONSUMER_AL_THESAURUS = 0x1A8,                               // HID type SEL
    HID_CONSUMER_AL_DICTIONARY = 0x1A9,                              // HID type SEL
    HID_CONSUMER_AL_DESKTOP = 0x1AA,                                 // HID type SEL
    HID_CONSUMER_AL_SPELL_CHECK = 0x1AB,                             // HID type SEL
    HID_CONSUMER_AL_GRAMMAR_CHECK = 0x1AC,                           // HID type SEL
    HID_CONSUMER_AL_WIRELESS_STATUS = 0x1AD,                         // HID type SEL
    HID_CONSUMER_AL_KEYBOARD_LAYOUT = 0x1AE,                         // HID type SEL
    HID_CONSUMER_AL_VIRUS_PROTECTION = 0x1AF,                        // HID type SEL
    HID_CONSUMER_AL_ENCRYPTION = 0x1B0,                              // HID type SEL
    HID_CONSUMER_AL_SCREEN_SAVER = 0x1B1,                            // HID type SEL
    HID_CONSUMER_AL_ALARMS = 0x1B2,                                  // HID type SEL
    HID_CONSUMER_AL_CLOCK = 0x1B3,                                   // HID type SEL
    HID_CONSUMER_AL_FILE_BROWSER = 0x1B4,                            // HID type SEL
    HID_CONSUMER_AL_POWER_STATUS = 0x1B5,                            // HID type SEL
    HID_CONSUMER_AL_IMAGE_BROWSER = 0x1B6,                           // HID type SEL
    HID_CONSUMER_AL_AUDIO_BROWSER = 0x1B7,                           // HID type SEL
    HID_CONSUMER_AL_MOVIE_BROWSER = 0x1B8,                           // HID type SEL
    HID_CONSUMER_AL_DIGITAL_RIGHTS_MANAGER = 0x1B9,                  // HID type SEL
    HID_CONSUMER_AL_DIGITAL_WALLET = 0x1BA,                          // HID type SEL
    // _Reserved 0x1BB
    HID_CONSUMER_AL_INSTANT_MESSAGING = 0x1BC,                                // HID type SEL
    HID_CONSUMER_AL_OEM_FEATURES_SLASH__TIPS_SLASH_TUTORIAL_BROWSER = 0x1BD,  // HID type SEL
    HID_CONSUMER_AL_OEM_HELP = 0x1BE,                                         // HID type SEL
    HID_CONSUMER_AL_ONLINE_COMMUNITY = 0x1BF,                                 // HID type SEL
    HID_CONSUMER_AL_ENTERTAINMENT_CONTENT_BROWSER = 0x1C0,                    // HID type SEL
    HID_CONSUMER_AL_ONLINE_SHOPPING_BROWSER = 0x1C1,                          // HID type SEL
    HID_CONSUMER_AL_SMARTCARD_INFORMATION_SLASH_HELP = 0x1C2,                 // HID type SEL
    HID_CONSUMER_AL_MARKET_MONITOR_SLASH_FINANCE_BROWSER = 0x1C3,             // HID type SEL
    HID_CONSUMER_AL_CUSTOMIZED_CORPORATE_NEWS_BROWSER = 0x1C4,                // HID type SEL
    HID_CONSUMER_AL_ONLINE_ACTIVITY_BROWSER = 0x1C5,                          // HID type SEL
    HID_CONSUMER_AL_RESEARCH_SLASH_SEARCH_BROWSER = 0x1C6,                    // HID type SEL
    HID_CONSUMER_AL_AUDIO_PLAYER = 0x1C7,                                     // HID type SEL
    // Reserved 0x1C8-1FF
    HID_CONSUMER_GENERIC_GUI_APPLICATION_CONTROLS = 0x200,  // HID type NARY
    HID_CONSUMER_AC_NEW = 0x201,                            // HID type SEL
    HID_CONSUMER_AC_OPEN = 0x202,                           // HID type SEL
    HID_CONSUMER_AC_CLOSE = 0x203,                          // HID type SEL
    HID_CONSUMER_AC_EXIT = 0x204,                           // HID type SEL
    HID_CONSUMER_AC_MAXIMIZE = 0x205,                       // HID type SEL
    HID_CONSUMER_AC_MINIMIZE = 0x206,                       // HID type SEL
    HID_CONSUMER_AC_SAVE = 0x207,                           // HID type SEL
    HID_CONSUMER_AC_PRINT = 0x208,                          // HID type SEL
    HID_CONSUMER_AC_PROPERTIES = 0x209,                     // HID type SEL
    HID_CONSUMER_AC_UNDO = 0x21A,                           // HID type SEL
    HID_CONSUMER_AC_COPY = 0x21B,                           // HID type SEL
    HID_CONSUMER_AC_CUT = 0x21C,                            // HID type SEL
    HID_CONSUMER_AC_PASTE = 0x21D,                          // HID type SEL
    HID_CONSUMER_AC_SELECT_ALL = 0x21E,                     // HID type SEL
    HID_CONSUMER_AC_FIND = 0x21F,                           // HID type SEL
    HID_CONSUMER_AC_FIND_AND_REPLACE = 0x220,               // HID type SEL
    HID_CONSUMER_AC_SEARCH = 0x221,                         // HID type SEL
    HID_CONSUMER_AC_GO_TO = 0x222,                          // HID type SEL
    HID_CONSUMER_AC_HOME = 0x223,                           // HID type SEL
    HID_CONSUMER_AC_BACK = 0x224,                           // HID type SEL
    HID_CONSUMER_AC_FORWARD = 0x225,                        // HID type SEL
    HID_CONSUMER_AC_STOP = 0x226,                           // HID type SEL
    HID_CONSUMER_AC_REFRESH = 0x227,                        // HID type SEL
    HID_CONSUMER_AC_PREVIOUS_LINK = 0x228,                  // HID type SEL
    HID_CONSUMER_AC_NEXT_LINK = 0x229,                      // HID type SEL
    HID_CONSUMER_AC_BOOKMARKS = 0x22A,                      // HID type SEL
    HID_CONSUMER_AC_HISTORY = 0x22B,                        // HID type SEL
    HID_CONSUMER_AC_SUBSCRIPTIONS = 0x22C,                  // HID type SEL
    HID_CONSUMER_AC_ZOOM_IN = 0x22D,                        // HID type SEL
    HID_CONSUMER_AC_ZOOM_OUT = 0x22E,                       // HID type SEL
    HID_CONSUMER_AC_ZOOM = 0x22F,                           // HID type LC
    HID_CONSUMER_AC_FULL_SCREEN_VIEW = 0x230,               // HID type SEL
    HID_CONSUMER_AC_NORMAL_VIEW = 0x231,                    // HID type SEL
    HID_CONSUMER_AC_VIEW_TOGGLE = 0x232,                    // HID type SEL
    HID_CONSUMER_AC_SCROLL_UP = 0x233,                      // HID type SEL
    HID_CONSUMER_AC_SCROLL_DOWN = 0x234,                    // HID type SEL
    HID_CONSUMER_AC_SCROLL = 0x235,                         // HID type LC
    HID_CONSUMER_AC_PAN_LEFT = 0x236,                       // HID type SEL
    HID_CONSUMER_AC_PAN_RIGHT = 0x237,                      // HID type SEL
    HID_CONSUMER_AC_PAN = 0x238,                            // HID type LC
    HID_CONSUMER_AC_NEW_WINDOW = 0x239,                     // HID type SEL
    HID_CONSUMER_AC_TILE_HORIZONTALLY = 0x23A,              // HID type SEL
    HID_CONSUMER_AC_TILE_VERTICALLY = 0x23B,                // HID type SEL
    HID_CONSUMER_AC_FORMAT = 0x23C,                         // HID type SEL
    HID_CONSUMER_AC_EDIT = 0x23D,                           // HID type SEL
    HID_CONSUMER_AC_BOLD = 0x23E,                           // HID type SEL
    HID_CONSUMER_AC_ITALICS = 0x23F,                        // HID type SEL
    HID_CONSUMER_AC_UNDERLINE = 0x240,                      // HID type SEL
    HID_CONSUMER_AC_STRIKETHROUGH = 0x241,                  // HID type SEL
    HID_CONSUMER_AC_SUBSCRIPT = 0x242,                      // HID type SEL
    HID_CONSUMER_AC_SUPERSCRIPT = 0x243,                    // HID type SEL
    HID_CONSUMER_AC_ALL_CAPS = 0x244,                       // HID type SEL
    HID_CONSUMER_AC_ROTATE = 0x245,                         // HID type SEL
    HID_CONSUMER_AC_RESIZE = 0x246,                         // HID type SEL
    HID_CONSUMER_AC_FLIP_HORIZONTAL = 0x247,                // HID type SEL
    HID_CONSUMER_AC_FLIP_VERTICAL = 0x248,                  // HID type SEL
    HID_CONSUMER_AC_MIRROR_HORIZONTAL = 0x249,              // HID type SEL
    HID_CONSUMER_AC_MIRROR_VERTICAL = 0x24A,                // HID type SEL
    HID_CONSUMER_AC_FONT_SELECT = 0x24B,                    // HID type SEL
    HID_CONSUMER_AC_FONT_COLOR = 0x24C,                     // HID type SEL
    HID_CONSUMER_AC_FONT_SIZE = 0x24D,                      // HID type SEL
    HID_CONSUMER_AC_JUSTIFY_LEFT = 0x24E,                   // HID type SEL
    HID_CONSUMER_AC_JUSTIFY_CENTER_H = 0x24F,               // HID type SEL
    HID_CONSUMER_AC_JUSTIFY_RIGHT = 0x250,                  // HID type SEL
    HID_CONSUMER_AC_JUSTIFY_BLOCK_H = 0x251,                // HID type SEL
    HID_CONSUMER_AC_JUSTIFY_TOP = 0x252,                    // HID type SEL
    HID_CONSUMER_AC_JUSTIFY_CENTER_V = 0x253,               // HID type SEL
    HID_CONSUMER_AC_JUSTIFY_BOTTOM = 0x254,                 // HID type SEL
    HID_CONSUMER_AC_JUSTIFY_BLOCK_V = 0x255,                // HID type SEL
    HID_CONSUMER_AC_INDENT_DECREASE = 0x256,                // HID type SEL
    HID_CONSUMER_AC_INDENT_INCREASE = 0x257,                // HID type SEL
    HID_CONSUMER_AC_NUMBERED_LIST = 0x258,                  // HID type SEL
    HID_CONSUMER_AC_RESTART_NUMBERING = 0x259,              // HID type SEL
    HID_CONSUMER_AC_BULLETED_LIST = 0x25A,                  // HID type SEL
    HID_CONSUMER_AC_PROMOTE = 0x25B,                        // HID type SEL
    HID_CONSUMER_AC_DEMOTE = 0x25C,                         // HID type SEL
    HID_CONSUMER_AC_YES = 0x25D,                            // HID type SEL
    HID_CONSUMER_AC_NO = 0x25E,                             // HID type SEL
    HID_CONSUMER_AC_CANCEL = 0x25F,                         // HID type SEL
    HID_CONSUMER_AC_CATALOG = 0x260,                        // HID type SEL
    HID_CONSUMER_AC_BUY_SLASH_CHECKOUT = 0x261,             // HID type SEL
    HID_CONSUMER_AC_ADD_TO_CART = 0x262,                    // HID type SEL
    HID_CONSUMER_AC_EXPAND = 0x263,                         // HID type SEL
    HID_CONSUMER_AC_EXPAND_ALL = 0x264,                     // HID type SEL
    HID_CONSUMER_AC_COLLAPSE = 0x265,                       // HID type SEL
    HID_CONSUMER_AC_COLLAPSE_ALL = 0x266,                   // HID type SEL
    HID_CONSUMER_AC_PRINT_PREVIEW = 0x267,                  // HID type SEL
    HID_CONSUMER_AC_PASTE_SPECIAL = 0x268,                  // HID type SEL
    HID_CONSUMER_AC_INSERT_MODE = 0x269,                    // HID type SEL
    HID_CONSUMER_AC_DELETE = 0x26A,                         // HID type SEL
    HID_CONSUMER_AC_LOCK = 0x26B,                           // HID type SEL
    HID_CONSUMER_AC_UNLOCK = 0x26C,                         // HID type SEL
    HID_CONSUMER_AC_PROTECT = 0x26D,                        // HID type SEL
    HID_CONSUMER_AC_UNPROTECT = 0x26E,                      // HID type SEL
    HID_CONSUMER_AC_ATTACH_COMMENT = 0x26F,                 // HID type SEL
    HID_CONSUMER_AC_DELETE_COMMENT = 0x270,                 // HID type SEL
    HID_CONSUMER_AC_VIEW_COMMENT = 0x271,                   // HID type SEL
    HID_CONSUMER_AC_SELECT_WORD = 0x272,                    // HID type SEL
    HID_CONSUMER_AC_SELECT_SENTENCE = 0x273,                // HID type SEL
    HID_CONSUMER_AC_SELECT_PARAGRAPH = 0x274,               // HID type SEL
    HID_CONSUMER_AC_SELECT_COLUMN = 0x275,                  // HID type SEL
    HID_CONSUMER_AC_SELECT_ROW = 0x276,                     // HID type SEL
    HID_CONSUMER_AC_SELECT_TABLE = 0x277,                   // HID type SEL
    HID_CONSUMER_AC_SELECT_OBJECT = 0x278,                  // HID type SEL
    HID_CONSUMER_AC_REDO_SLASH_REPEAT = 0x279,              // HID type SEL
    HID_CONSUMER_AC_SORT = 0x27A,                           // HID type SEL
    HID_CONSUMER_AC_SORT_ASCENDING = 0x27B,                 // HID type SEL
    HID_CONSUMER_AC_SORT_DESCENDING = 0x27C,                // HID type SEL
    HID_CONSUMER_AC_FILTER = 0x27D,                         // HID type SEL
    HID_CONSUMER_AC_SET_CLOCK = 0x27E,                      // HID type SEL
    HID_CONSUMER_AC_VIEW_CLOCK = 0x27F,                     // HID type SEL
    HID_CONSUMER_AC_SELECT_TIME_ZONE = 0x280,               // HID type SEL
    HID_CONSUMER_AC_EDIT_TIME_ZONES = 0x281,                // HID type SEL
    HID_CONSUMER_AC_SET_ALARM = 0x282,                      // HID type SEL
    HID_CONSUMER_AC_CLEAR_ALARM = 0x283,                    // HID type SEL
    HID_CONSUMER_AC_SNOOZE_ALARM = 0x284,                   // HID type SEL
    HID_CONSUMER_AC_RESET_ALARM = 0x285,                    // HID type SEL
    HID_CONSUMER_AC_SYNCHRONIZE = 0x286,                    // HID type SEL
    HID_CONSUMER_AC_SEND_SLASH_RECEIVE = 0x287,             // HID type SEL
    HID_CONSUMER_AC_SEND_TO = 0x288,                        // HID type SEL
    HID_CONSUMER_AC_REPLY = 0x289,                          // HID type SEL
    HID_CONSUMER_AC_REPLY_ALL = 0x28A,                      // HID type SEL
    HID_CONSUMER_AC_FORWARD_MSG = 0x28B,                    // HID type SEL
    HID_CONSUMER_AC_SEND = 0x28C,                           // HID type SEL
    HID_CONSUMER_AC_ATTACH_FILE = 0x28D,                    // HID type SEL
    HID_CONSUMER_AC_UPLOAD = 0x28E,                         // HID type SEL
    HID_CONSUMER_AC_DOWNLOAD_SAVE_TARGET_AS = 0x28F,        // HID type SEL
    HID_CONSUMER_AC_SET_BORDERS = 0x290,                    // HID type SEL
    HID_CONSUMER_AC_INSERT_ROW = 0x291,                     // HID type SEL
    HID_CONSUMER_AC_INSERT_COLUMN = 0x292,                  // HID type SEL
    HID_CONSUMER_AC_INSERT_FILE = 0x293,                    // HID type SEL
    HID_CONSUMER_AC_INSERT_PICTURE = 0x294,                 // HID type SEL
    HID_CONSUMER_AC_INSERT_OBJECT = 0x295,                  // HID type SEL
    HID_CONSUMER_AC_INSERT_SYMBOL = 0x296,                  // HID type SEL
    HID_CONSUMER_AC_SAVE_AND_CLOSE = 0x297,                 // HID type SEL
    HID_CONSUMER_AC_RENAME = 0x298,                         // HID type SEL
    HID_CONSUMER_AC_MERGE = 0x299,                          // HID type SEL
    HID_CONSUMER_AC_SPLIT = 0x29A,                          // HID type SEL
    HID_CONSUMER_AC_DISRIBUTE_HORIZONTALLY = 0x29B,         // HID type SEL
    HID_CONSUMER_AC_DISTRIBUTE_VERTICALLY = 0x29C,          // HID type SEL
};
