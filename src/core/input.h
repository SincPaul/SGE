//
// Created by Geisthardt on 03.03.2025.
//

#ifndef INPUT_H
#define INPUT_H


extern int *key_states;
extern int *last_key_states;
extern int *mouse_states;
extern int *last_mouse_states;
extern struct mouse_pos last_mouse_pos;
extern struct mouse_pos delta_mouse_pos;

typedef enum keys {
    // Control characters (0x00-0x1F)
    KEY_NULL        = 0x00,   // Null character
    KEY_ESCAPE      = 0x1B,   // Escape key
    KEY_ENTER       = 0x0D,   // Carriage return
    KEY_TAB         = 0x09,   // Horizontal tab
    KEY_BACKSPACE   = 0x08,   // Backspace
    KEY_DELETE      = 0x7F,   // Delete (not ASCII but common)

    // Whitespace and basic symbols
    KEY_SPACE       = 0x20,   // ' ' Spacebar

    // Numbers (0-9)
    KEY_0           = 0x30,   // '0'
    KEY_1           = 0x31,   // '1'
    KEY_2           = 0x32,   // '2'
    KEY_3           = 0x33,   // '3'
    KEY_4           = 0x34,   // '4'
    KEY_5           = 0x35,   // '5'
    KEY_6           = 0x36,   // '6'
    KEY_7           = 0x37,   // '7'
    KEY_8           = 0x38,   // '8'
    KEY_9           = 0x39,   // '9'

    // Uppercase letters (A-Z)
    KEY_A           = 0x41,
    KEY_B           = 0x42,
    KEY_C           = 0x43,
    KEY_D           = 0x44,
    KEY_E           = 0x45,
    KEY_F           = 0x46,
    KEY_G           = 0x47,
    KEY_H           = 0x48,
    KEY_I           = 0x49,
    KEY_J           = 0x4A,
    KEY_K           = 0x4B,
    KEY_L           = 0x4C,
    KEY_M           = 0x4D,
    KEY_N           = 0x4E,
    KEY_O           = 0x4F,
    KEY_P           = 0x50,
    KEY_Q           = 0x51,
    KEY_R           = 0x52,
    KEY_S           = 0x53,
    KEY_T           = 0x54,
    KEY_U           = 0x55,
    KEY_V           = 0x56,
    KEY_W           = 0x57,
    KEY_X           = 0x58,
    KEY_Y           = 0x59,
    KEY_Z           = 0x5A,

    // Symbols (US keyboard layout)
    KEY_EXCLAIM      = 0x21,   // '!'
    KEY_DQUOTE       = 0x22,   // '"'
    KEY_HASH         = 0x23,   // '#'
    KEY_DOLLAR       = 0x24,   // '$'
    KEY_PERCENT      = 0x25,   // '%'
    KEY_AMPERSAND    = 0x26,   // '&'
    KEY_SQUOTE       = 0x27,   // '''
    KEY_LPAREN       = 0x28,   // '('
    KEY_RPAREN       = 0x29,   // ')'
    KEY_ASTERISK     = 0x2A,   // '*'
    KEY_PLUS         = 0x2B,   // '+'
    KEY_COMMA        = 0x2C,   // ','
    KEY_MINUS        = 0x2D,   // '-'
    KEY_PERIOD       = 0x2E,   // '.'
    KEY_SLASH        = 0x2F,   // '/'
    KEY_COLON        = 0x3A,   // ':'
    KEY_SEMICOLON    = 0x3B,   // ';'
    KEY_LESS         = 0x3C,   // '<'
    KEY_EQUAL        = 0x3D,   // '='
    KEY_GREATER      = 0x3E,   // '>'
    KEY_QUESTION     = 0x3F,   // '?'
    KEY_AT           = 0x40,   // '@'
    KEY_LBRACKET     = 0x5B,   // '['
    KEY_BACKSLASH    = 0x5C,   // '\'
    KEY_RBRACKET     = 0x5D,   // ']'
    KEY_CARET        = 0x5E,   // '^'
    KEY_UNDERSCORE   = 0x5F,   // '_'
    KEY_BACKTICK     = 0x60,   // '`'

    // Modifier keys
        KEY_SHIFT       = 0x10, //windows shift
        KEY_CTRL        = 0x11,
    KEY_LSHIFT       = 0xA0,   // Left Shift
    KEY_RSHIFT       = 0xA1,   // Right Shift
    KEY_LCTRL        = 0xA2,   // Left Control
    KEY_RCTRL        = 0xA3,   // Right Control
    KEY_LALT         = 0xA4,   // Left Alt (Menu)
    KEY_RALT         = 0xA5,   // Right Alt (Menu)
    KEY_CAPSLOCK     = 0x14,   // Caps Lock

    // Function keys
    KEY_F1           = 0x70,
    KEY_F2           = 0x71,
    KEY_F3           = 0x72,
    KEY_F4           = 0x73,
    KEY_F5           = 0x74,
    KEY_F6           = 0x75,
    KEY_F7           = 0x76,
    KEY_F8           = 0x77,
    KEY_F9           = 0x78,
    KEY_F10          = 0x79,
    KEY_F11          = 0x7A,
    KEY_F12          = 0x7B,

    // Navigation keys
    KEY_INSERT       = 0x2D,   // Insert
    KEY_HOME         = 0x24,   // Home
    KEY_END          = 0x23,   // End
    KEY_PAGEUP       = 0x21,   // Page Up
    KEY_PAGEDOWN     = 0x22,   // Page Down

    // Arrow keys
    KEY_ARROW_LEFT   = 0x25,   // Left arrow
    KEY_ARROW_UP     = 0x26,   // Up arrow
    KEY_ARROW_RIGHT  = 0x27,   // Right arrow
    KEY_ARROW_DOWN   = 0x28,   // Down arrow

    // NumPad keys (for completeness)
    KEY_NUMPAD0      = 0x60,
    KEY_NUMPAD1      = 0x61,
    KEY_NUMPAD2      = 0x62,
    KEY_NUMPAD3      = 0x63,
    KEY_NUMPAD4      = 0x64,
    KEY_NUMPAD5      = 0x65,
    KEY_NUMPAD6      = 0x66,
    KEY_NUMPAD7      = 0x67,
    KEY_NUMPAD8      = 0x68,
    KEY_NUMPAD9      = 0x69,
    KEY_NUMPAD_MULT  = 0x6A,   // *
    KEY_NUMPAD_ADD   = 0x6B,   // +
    KEY_NUMPAD_SUB   = 0x6D,   // -
    KEY_NUMPAD_DEC   = 0x6E,   // .
    KEY_NUMPAD_DIV   = 0x6F,   // /

} keys;

typedef enum mouse_buttons {
        MBUTTON_LEFT   = 0x01,
        MBUTTON_RIGHT  = 0x02,
        MBUTTON_MIDDLE = 0x04
} mouse_buttons;


typedef struct mouse_pos {
        int x;
        int y;
} mouse_pos;

//void on_button_press(keys key, event event);

int is_key_down(keys key); // returns 1 if pressed
int is_key_pressed(keys key); //returns 1 only if it changed from not pressed to pressed
int is_shift_active();

int is_mouse_down(mouse_buttons buttons);
int was_mouse_down(mouse_buttons buttons);
int is_mouse_pressed(mouse_buttons button);

void update_key_states();

mouse_pos get_mouse_position();
mouse_pos get_delta_mouse_position();

void enable_input_tracking();


#endif //INPUT_H
