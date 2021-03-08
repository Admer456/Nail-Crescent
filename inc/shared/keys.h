#ifndef __SHARED_KEYS_H__
#define __SHARED_KEYS_H__

//
// these are the key numbers that should be passed to Key_Event
//
#define K_BACKSPACE     8
#define K_TAB           9
#define K_ENTER         13
#define K_PAUSE         19
#define K_ESCAPE        27
#define K_SPACE         32
#define K_DEL           127

// normal keys should be passed as lowercased ascii
#define K_ASCIIFIRST    32
#define K_ASCIILAST     127

#define K_UPARROW       128
#define K_DOWNARROW     129
#define K_LEFTARROW     130
#define K_RIGHTARROW    131

#define K_ALT           132
#define K_CTRL          133
#define K_SHIFT         134
#define K_F1            135
#define K_F2            136
#define K_F3            137
#define K_F4            138
#define K_F5            139
#define K_F6            140
#define K_F7            141
#define K_F8            142
#define K_F9            143
#define K_F10           144
#define K_F11           145
#define K_F12           146
#define K_INS           147
#define K_PGDN          148
#define K_PGUP          149
#define K_HOME          150
#define K_END           151

#define K_102ND         152

#define K_NUMLOCK       153
#define K_CAPSLOCK      154
#define K_SCROLLOCK     155
#define K_LWINKEY       156
#define K_RWINKEY       157
#define K_MENU          158
#define K_PRINTSCREEN   159

#define K_KP_HOME       160
#define K_KP_UPARROW    161
#define K_KP_PGUP       162
#define K_KP_LEFTARROW  163
#define K_KP_5          164
#define K_KP_RIGHTARROW 165
#define K_KP_END        166
#define K_KP_DOWNARROW  167
#define K_KP_PGDN       168
#define K_KP_ENTER      169
#define K_KP_INS        170
#define K_KP_DEL        171
#define K_KP_SLASH      172
#define K_KP_MINUS      173
#define K_KP_PLUS       174
#define K_KP_MULTIPLY   175

// these come paired with legacy K_ALT/K_CTRL/K_SHIFT events
#define K_LALT          180
#define K_RALT          181
#define K_LCTRL         182
#define K_RCTRL         183
#define K_LSHIFT        184
#define K_RSHIFT        185

// mouse buttons generate virtual keys
#define K_MOUSEFIRST    200
#define K_MOUSE1        200
#define K_MOUSE2        201
#define K_MOUSE3        202
#define K_MOUSE4        203
#define K_MOUSE5        204
#define K_MOUSE6        205
#define K_MOUSE7        206
#define K_MOUSE8        207

// mouse wheel generates virtual keys
#define K_MWHEELDOWN    210
#define K_MWHEELUP      211
#define K_MWHEELRIGHT   212
#define K_MWHEELLEFT    213
#define K_MOUSELAST     213

typedef enum keydest_e {
    KEY_GAME = 0,
    KEY_CONSOLE = (1 << 0),
    KEY_MESSAGE = (1 << 1),
    KEY_MENU = (1 << 2),
    KEY_INGAME_MENU = (1 << 3)  // IngameMenu: Used for libRmlUI contexts during actual gameplay.
} keydest_t;
// CPP: C enum, commented out.
//typedef enum keydest_e {
//    KEY_GAME = 0,
//    KEY_CONSOLE = (1 << 0),
//    KEY_MESSAGE = (1 << 1),
//    KEY_MENU = (1 << 2)
//} keydest_t;

typedef qboolean(*keywaitcb_t)(void* arg, int key);

#endif // __SHARED_KEYS_H__