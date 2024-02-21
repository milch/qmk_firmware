#include "debug.h"
#include "info_config.h"
#include "keyboard.h"
#include "keycodes.h"
#include "keymap_colemak.h"
#include "keymap_us.h"
#include "modifiers.h"
#include "process_tap_dance.h"
#include "quantum.h"
#include "quantum_keycodes.h"
#include QMK_KEYBOARD_H

#include "layers.h"
#include "features/select_word.h"
#include "features/achordion.h"
#include "features/magic_paren.h"
#include "features/num_word.h"
#include "features/tap_dance.h"

void matrix_scan_user() {
    achordion_task();
    select_word_task();
}

enum custom_keycodes {
    ARROW = SAFE_RANGE,
    FAT_ARROW,
    SELECT_WORD,
    MAGIC_PAREN,
    NW_TOGG,
    UPDIR,
    SCOPE,
    FENCE,
};

#define HOME_A LSFT_T(KC_A)
#define HOME_S LCTL_T(KC_S)
#define HOME_D LALT_T(KC_D)
#define HOME_F LGUI_T(KC_F)

#define CM_H_A HOME_A
#define CM_H_R HOME_S
#define CM_H_S HOME_D
#define CM_H_T HOME_F

#define HOME_J LGUI_T(KC_J)
#define HOME_K LALT_T(KC_K)
#define HOME_L LCTL_T(KC_L)
#define HOME_SC LSFT_T(KC_SCLN)

#define CM_H_N HOME_J
#define CM_H_E HOME_K
#define CM_H_I HOME_L
#define CM_H_O HOME_SC

#define HOME_Z LT(NUM, KC_Z)
#define HOME_SLSH LT(NUM, KC_SLSH)

#define THUMB_SPC LT(NAV, KC_SPACE)
#define THUMB_ENT LT(NAV, KC_ENTER)
#define THUMB_REP TD(LT_SYM_REP)
#define THUMB_AREP TD(LT_SYM_AREP)

#include "g/keymap_combo.h"

#define HYPER_0 LCAG(KC_0)
#define HYPER_1 LCAG(KC_1)
#define HYPER_2 LCAG(KC_2)
#define HYPER_3 LCAG(KC_3)
#define HYPER_4 LCAG(KC_4)
#define HYPER_5 LCAG(KC_5)
#define HYPER_6 LCAG(KC_6)
#define HYPER_7 LCAG(KC_7)
#define HYPER_8 LCAG(KC_8)
#define HYPER_9 LCAG(KC_9)

// clang-format off
tap_dance_action_t tap_dance_actions[] = {
    [LT_SYM_REP]  = ACTION_TAP_DANCE_LT(SYM, QK_REP),
    [LT_SYM_AREP] = ACTION_TAP_DANCE_LT(SYM, QK_AREP),
};
// clang-format on

bool is_thumb_key(keypos_t pos) {
    return (pos.row == 4 && pos.col == 5) || (pos.row == 9 && pos.col == 5);
}

uint16_t achordion_streak_timeout(uint16_t keycode) {
    return TAPPING_TERM;
}

uint16_t achordion_timeout(uint16_t keycode) {
    switch (keycode) {
        case THUMB_ENT:
        case THUMB_SPC:
        case THUMB_REP:
        case THUMB_AREP:
            return 0;
    }

    return 800;
}

bool achordion_chord(uint16_t tap_hold_keycode, keyrecord_t* tap_hold_record, uint16_t other_keycode, keyrecord_t* other_record) {
    dprintf("Achordion tap hold row: %d, col: %d\n", tap_hold_record->event.key.row, tap_hold_record->event.key.col);
    dprintf("Achordion other row: %d, col: %d\n", other_record->event.key.row, other_record->event.key.col);
    if (is_thumb_key(tap_hold_record->event.key)) {
        return true;
    }

    // Otherwise, follow the opposite hands rule.
    return achordion_opposite_hands(tap_hold_record, other_record);
}

void keyboard_pre_init_user(void) {
    // Set our LED pin as output
    setPinOutput(24);
    // Turn the LED off
    // (Due to technical reasons, high is off and low is on)
    writePinHigh(24);
}

static uint8_t find_alt_keycode(const uint8_t (*table)[2], uint8_t table_size_bytes, uint8_t target) {
    const uint8_t* keycodes = (const uint8_t*)table;
    for (uint8_t i = 0; i < table_size_bytes; ++i) {
        if (target == pgm_read_byte(keycodes + i)) {
            // Xor (i ^ 1) the index to get the other element in the pair.
            return pgm_read_byte(keycodes + (i ^ 1));
        }
    }
    return KC_NO;
}

uint16_t get_colemak_bigram(uint16_t other_keycode) {
    switch (other_keycode) {
        case CM_D:
            return CM_G;
        case CM_E:
            return CM_U;
        case CM_G:
            return CM_T;
        case CM_H:
            return CM_L;
        case CM_N:
            return CM_K;
        case CM_P:
            return CM_T;
        case CM_S:
            return CM_C;
        case CM_U:
            return CM_E;
    }

    return KC_NO;
}

// This is adapted from the core implementation, just changing KC to CM
uint16_t get_alt_repeat_key_keycode_user(uint16_t keycode, uint8_t mods) {
    uint16_t alt_keycode = 0;
    if (IS_QK_BASIC(QK_MOD_TAP_GET_TAP_KEYCODE(keycode))) {
        if ((mods & (MOD_LCTL | MOD_LALT | MOD_LGUI))) {
            // The last key was pressed with a modifier other than Shift.
            // The following maps
            //   mod + F <-> mod + B
            // and a few others, supporting several core hotkeys used in
            // Emacs, Vim, less, and other programs.
            static const uint8_t pairs[][2] PROGMEM = {
                {CM_F, CM_B}, // Forward / Backward.
                {CM_N, CM_P}, // Next / Previous.
                {CM_A, CM_E}, // Home / End.
                {CM_O, CM_I}, // Older / Newer in Vim jump list.
                {CM_D, CM_U}, // Down / Up.
            };
            alt_keycode = find_alt_keycode(pairs, sizeof(pairs), keycode);
        } else {
            // The last key was pressed with no mods or only Shift. The
            // following map a few more Vim hotkeys.
            static const uint8_t pairs[][2] PROGMEM = {
                {CM_J, CM_K}, // Down / Up.
                {CM_H, CM_L}, // Left / Right.
                // These two lines map W and E to B, and B to W.
                {CM_W, CM_B}, // Forward / Backward by word.
                {CM_S, CM_T}, // Forward / Backward history in SurfingKeys
            };
            alt_keycode = find_alt_keycode(pairs, sizeof(pairs), keycode);
            if (!alt_keycode) {
                alt_keycode = get_colemak_bigram(keycode);
            }
        }

        if (!alt_keycode) {
            static const uint8_t pairs[][2] PROGMEM = {
                {CM_LBRC, CM_RBRC}, // Brackets
            };
            // The following key pairs are considered with any mods.
            alt_keycode = find_alt_keycode(pairs, sizeof(pairs), keycode);
        }

        if (alt_keycode) {
            // Combine basic keycode with mods.
            return (mods << 8) | alt_keycode;
        }
    }

    return KC_TRANSPARENT; // Default implementation
}

bool caps_word_press_user(uint16_t keycode) {
    switch (keycode) {
        case CM_A:
        case CM_B:
        case CM_C:
        case CM_D:
        case CM_E:
        case CM_F:
        case CM_G:
        case CM_H:
        case CM_I:
        case CM_J:
        case CM_K:
        case CM_L:
        case CM_M:
        case CM_N:
        case CM_O:
        case CM_P:
        case CM_Q:
        case CM_R:
        case CM_S:
        case CM_T:
        case CM_U:
        case CM_V:
        case CM_W:
        case CM_X:
        case CM_Y:
        case CM_Z:
            add_weak_mods(MOD_BIT(KC_LSFT)); // Apply shift to next key.
            return true;

        // Keycodes that continue Caps Word, without shifting.
        case CM_1 ... CM_0:
        case KC_BSPC:
        case KC_DEL:
        case CM_MINS:
        case CM_UNDS:
        case THUMB_REP:
        case THUMB_AREP:
            return true;

        // Deactivate Caps Word.
        case CM_SCLN:
        default:
            return false;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    if (!process_achordion(keycode, record)) return false;
    if (!process_select_word(keycode, record, SELECT_WORD)) return false;
    if (!process_magic_paren(keycode, record, MAGIC_PAREN)) return false;
    if (!process_num_word(keycode, record, NW_TOGG)) return false;

    switch (keycode) {
        case KC_BSPC: { // Shift+backspace = delete
            static uint16_t registered_key = KC_NO;
            if (record->event.pressed) { // On key press.
                const uint8_t mods = get_mods();
#ifndef NO_ACTION_ONESHOT
                uint8_t shift_mods = (mods | get_oneshot_mods()) & MOD_MASK_SHIFT;
#else
                uint8_t shift_mods = mods & MOD_MASK_SHIFT;
#endif                            // NO_ACTION_ONESHOT
                if (shift_mods) { // At least one shift key is held.
                    registered_key = KC_DEL;
                    // If one shift is held, clear it from the mods. But if both
                    // shifts are held, leave as is to send Shift + Del.
                    if (shift_mods != MOD_MASK_SHIFT) {
#ifndef NO_ACTION_ONESHOT
                        del_oneshot_mods(MOD_MASK_SHIFT);
#endif // NO_ACTION_ONESHOT
                        unregister_mods(MOD_MASK_SHIFT);
                    }
                } else {
                    registered_key = KC_BSPC;
                }

                register_code(registered_key);
                set_mods(mods);
            } else { // On key release.
                unregister_code(registered_key);
            }
            return false;
        }
        case ARROW: {
            const uint8_t mods         = get_mods();
            const uint8_t oneshot_mods = get_oneshot_mods();
            if (record->event.pressed) {
                if ((mods | oneshot_mods) & MOD_MASK_SHIFT) { // Is shift held?
                    // Temporarily delete shift.
                    del_oneshot_mods(MOD_MASK_SHIFT);
                    unregister_mods(MOD_MASK_SHIFT);
                    SEND_STRING("=>");
                    register_mods(mods); // Restore mods.
                } else {
                    SEND_STRING("->");
                }
            }
            return false;
        }
        case FAT_ARROW:
            if (record->event.pressed) {
                SEND_STRING("=>");
            }
            return false;
        case UPDIR:
            if (record->event.pressed) {
                SEND_STRING("../");
            }
            break;
        case SCOPE:
            if (record->event.pressed) {
                tap_code16(CM_COLN);
                tap_code16(CM_COLN);
            }
            break;
        case FENCE:
            if (record->event.pressed) {
                SEND_STRING("```");
            }
            break;
    }

    return true;
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
//    ┌─────────┬────────┬────────┬────────┬───────────────────┬───────────┐                           ┌────────────┬────────┬─────────┬────────┬───────────┬─────────┐
//    │    `    │   1    │   2    │   3    │         4         │     5     │                           │     6      │   7    │    8    │   9    │     0     │    =    │
//    ├─────────┼────────┼────────┼────────┼───────────────────┼───────────┤                           ├────────────┼────────┼─────────┼────────┼───────────┼─────────┤
//    │   tab   │   q    │   w    │   e    │         r         │     t     │                           │     y      │   u    │    i    │   o    │     p     │    -    │
//    ├─────────┼────────┼────────┼────────┼───────────────────┼───────────┤                           ├────────────┼────────┼─────────┼────────┼───────────┼─────────┤
//    │   esc   │ HOME_A │ HOME_S │ HOME_D │      HOME_F       │     g     │                           │     h      │ HOME_J │ HOME_K  │ HOME_L │  HOME_SC  │    '    │
//    ├─────────┼────────┼────────┼────────┼───────────────────┼───────────┼───────────┐   ┌───────────┼────────────┼────────┼─────────┼────────┼───────────┼─────────┤
//    │ CW_TOGG │ HOME_Z │   x    │   c    │         v         │     b     │     [     │   │     ]     │     n      │   m    │    ,    │   .    │ HOME_SLSH │ NW_TOGG │
//    └─────────┴────────┴────────┼────────┼───────────────────┼───────────┼───────────┤   ├───────────┼────────────┼────────┼─────────┼────────┴───────────┴─────────┘
//                                │ QK_REP │ MO(WORKSPACE_NAV) │ THUMB_REP │ THUMB_SPC │   │ THUMB_ENT │ THUMB_AREP │  bspc  │ QK_AREP │
//                                └────────┴───────────────────┴───────────┴───────────┘   └───────────┴────────────┴────────┴─────────┘
[BASE] = LAYOUT(
  KC_GRV  , KC_1   , KC_2   , KC_3   , KC_4              , KC_5      ,                             KC_6       , KC_7    , KC_8    , KC_9   , KC_0      , KC_EQL ,
  KC_TAB  , KC_Q   , KC_W   , KC_E   , KC_R              , KC_T      ,                             KC_Y       , KC_U    , KC_I    , KC_O   , KC_P      , KC_MINS,
  KC_ESC  , HOME_A , HOME_S , HOME_D , HOME_F            , KC_G      ,                             KC_H       , HOME_J  , HOME_K  , HOME_L , HOME_SC   , KC_QUOT,
  CW_TOGG , HOME_Z , KC_X   , KC_C   , KC_V              , KC_B      , KC_LBRC   ,     KC_RBRC   , KC_N       , KC_M    , KC_COMM , KC_DOT , HOME_SLSH , NW_TOGG,
                              QK_REP , MO(WORKSPACE_NAV) , THUMB_REP , THUMB_SPC ,     THUMB_ENT , THUMB_AREP , KC_BSPC , QK_AREP
),

//    ┌─────────┬──────┬─────────────┬──────┬──────┬─────┐                                             ┌──────┬──────┬──────┬──────┬──────┬──────┐
//    │ DB_TOGG │ bRID │    bRIU     │  no  │  no  │ no  │                                             │ mprv │ mply │ mnxt │ mute │ vold │ volu │
//    ├─────────┼──────┼─────────────┼──────┼──────┼─────┤                                             ├──────┼──────┼──────┼──────┼──────┼──────┤
//    │ DT_PRNT │  no  │ SELECT_WORD │  no  │  no  │ no  │                                             │ down │ rght │ ms_u │ btn1 │ btn2 │  no  │
//    ├─────────┼──────┼─────────────┼──────┼──────┼─────┤                                             ├──────┼──────┼──────┼──────┼──────┼──────┤
//    │  DT_UP  │ lsft │    lctl     │ lalt │ lgui │     │                                             │ left │ ms_l │ ms_d │ ms_r │  no  │  no  │
//    ├─────────┼──────┼─────────────┼──────┼──────┼─────┼────────────────────┐   ┌────────────────────┼──────┼──────┼──────┼──────┼──────┼──────┤
//    │ DT_DOWN │      │             │      │      │     │                    │   │                    │  up  │      │      │      │      │      │
//    └─────────┴──────┴─────────────┼──────┼──────┼─────┼────────────────────┤   ├────────────────────┼──────┼──────┼──────┼──────┴──────┴──────┘
//                                   │      │      │     │ OSL(WORKSPACE_NAV) │   │ OSL(WORKSPACE_NAV) │      │      │      │
//                                   └──────┴──────┴─────┴────────────────────┘   └────────────────────┴──────┴──────┴──────┘
[NAV] = LAYOUT(
  DB_TOGG , KC_BRID , KC_BRIU     , XXXXXXX , XXXXXXX , XXXXXXX ,                                               KC_MPRV , KC_MPLY  , KC_MNXT  , KC_MUTE    , KC_VOLD    , KC_VOLU,
  DT_PRNT , XXXXXXX , SELECT_WORD , XXXXXXX , XXXXXXX , XXXXXXX ,                                               KC_DOWN , KC_RIGHT , KC_MS_UP , KC_MS_BTN1 , KC_MS_BTN2 , XXXXXXX,
  DT_UP   , KC_LSFT , KC_LCTL     , KC_LALT , KC_LGUI , _______ ,                                               KC_LEFT , KC_MS_L  , KC_MS_D  , KC_MS_R    , XXXXXXX    , XXXXXXX,
  DT_DOWN , _______ , _______     , _______ , _______ , _______ , _______            ,     _______            , KC_UP   , _______  , _______  , _______    , _______    , _______,
                                    _______ , _______ , _______ , OSL(WORKSPACE_NAV) ,     OSL(WORKSPACE_NAV) , _______ , _______  , _______
),

//    ┌─────┬────────────┬────────────┬────────────┬────────────┬────────────┐                             ┌────────────┬────────────┬────────────┬────────────┬────────────┬─────┐
//    │     │            │            │            │            │            │                             │            │            │            │            │            │     │
//    ├─────┼────────────┼────────────┼────────────┼────────────┼────────────┤                             ├────────────┼────────────┼────────────┼────────────┼────────────┼─────┤
//    │     │ S(HYPER_1) │ S(HYPER_2) │ S(HYPER_3) │ S(HYPER_4) │ S(HYPER_5) │                             │ S(HYPER_6) │ S(HYPER_7) │ S(HYPER_8) │ S(HYPER_9) │ S(HYPER_0) │     │
//    ├─────┼────────────┼────────────┼────────────┼────────────┼────────────┤                             ├────────────┼────────────┼────────────┼────────────┼────────────┼─────┤
//    │     │  HYPER_1   │  HYPER_2   │  HYPER_3   │  HYPER_4   │  HYPER_5   │                             │  HYPER_6   │  HYPER_7   │  HYPER_8   │  HYPER_9   │  HYPER_0   │     │
//    ├─────┼────────────┼────────────┼────────────┼────────────┼────────────┼────────────┐   ┌────────────┼────────────┼────────────┼────────────┼────────────┼────────────┼─────┤
//    │     │            │            │            │            │            │ LCTL(left) │   │ RCTL(rght) │            │            │            │            │            │     │
//    └─────┴────────────┴────────────┼────────────┼────────────┼────────────┼────────────┤   ├────────────┼────────────┼────────────┼────────────┼────────────┴────────────┴─────┘
//                                    │            │            │            │            │   │            │            │            │            │
//                                    └────────────┴────────────┴────────────┴────────────┘   └────────────┴────────────┴────────────┴────────────┘
[WORKSPACE_NAV] = LAYOUT(
  _______ , _______    , _______    , _______    , _______    , _______    ,                                      _______    , _______    , _______    , _______    , _______    , _______,
  _______ , S(HYPER_1) , S(HYPER_2) , S(HYPER_3) , S(HYPER_4) , S(HYPER_5) ,                                      S(HYPER_6) , S(HYPER_7) , S(HYPER_8) , S(HYPER_9) , S(HYPER_0) , _______,
  _______ , HYPER_1    , HYPER_2    , HYPER_3    , HYPER_4    , HYPER_5    ,                                      HYPER_6    , HYPER_7    , HYPER_8    , HYPER_9    , HYPER_0    , _______,
  _______ , _______    , _______    , _______    , _______    , _______    , LCTL(KC_LEFT) ,     RCTL(KC_RIGHT) , _______    , _______    , _______    , _______    , _______    , _______,
                                      _______    , _______    , _______    , _______       ,     _______        , _______    , _______    , _______
),

//    ┌───────────┬─────┬─────┬─────┬─────┬─────┐               ┌─────┬─────────┬─────────┬─────┬─────┬─────────┐
//    │           │     │     │     │     │     │               │     │         │         │     │     │         │
//    ├───────────┼─────┼─────┼─────┼─────┼─────┤               ├─────┼─────────┼─────────┼─────┼─────┼─────────┤
//    │   ARROW   │  '  │  <  │  >  │  "  │  ^  │               │  #  │    $    │    {    │  }  │  %  │  SCOPE  │
//    ├───────────┼─────┼─────┼─────┼─────┼─────┤               ├─────┼─────────┼─────────┼─────┼─────┼─────────┤
//    │ FAT_ARROW │  _  │  -  │  +  │  =  │  &  │               │  |  │ CM_COLN │ CM_LBRC │  ]  │  `  │  FENCE  │
//    ├───────────┼─────┼─────┼─────┼─────┼─────┼─────┐   ┌─────┼─────┼─────────┼─────────┼─────┼─────┼─────────┤
//    │  CW_TOGG  │  ,  │  /  │  *  │  \  │  !  │     │   │     │  ~  │    @    │    (    │  )  │  ?  │ NW_TOGG │
//    └───────────┴─────┴─────┼─────┼─────┼─────┼─────┤   ├─────┼─────┼─────────┼─────────┼─────┴─────┴─────────┘
//                            │     │     │     │     │   │     │     │         │         │
//                            └─────┴─────┴─────┴─────┘   └─────┴─────┴─────────┴─────────┘
[SYM] = LAYOUT(
  _______   , _______ , _______  , _______ , _______ , _______ ,                         _______ , _______ , _______ , _______ , _______ , _______,
  ARROW     , KC_QUOT , KC_LABK  , KC_RABK , KC_DQUO , KC_CIRC ,                         KC_HASH , KC_DLR  , KC_LCBR , KC_RCBR , KC_PERC , SCOPE  ,
  FAT_ARROW , KC_UNDS , KC_MINS  , KC_PLUS , KC_EQL  , KC_AMPR ,                         KC_PIPE , CM_COLN , CM_LBRC , KC_RBRC , KC_GRV  , FENCE  ,
  CW_TOGG   , KC_COMM , KC_SLASH , KC_ASTR , KC_BSLS , KC_EXLM , _______ ,     _______ , KC_TILD , KC_AT   , KC_LPRN , KC_RPRN , KC_QUES , NW_TOGG,
                                   _______ , _______ , _______ , _______ ,     _______ , _______ , _______ , _______
),

//    ┌─────┬─────┬─────┬─────┬─────┬─────┐               ┌─────┬─────┬─────┬─────┬─────┬─────┐
//    │     │ f1  │ f2  │ f3  │ f4  │ f5  │               │ f6  │ f7  │ f8  │ f9  │ f10 │ f11 │
//    ├─────┼─────┼─────┼─────┼─────┼─────┤               ├─────┼─────┼─────┼─────┼─────┼─────┤
//    │     │     │  w  │     │     │  t  │               │  y  │     │     │     │     │     │
//    ├─────┼─────┼─────┼─────┼─────┼─────┤               ├─────┼─────┼─────┼─────┼─────┼─────┤
//    │     │  1  │  2  │  3  │  4  │  5  │               │  6  │  7  │  8  │  9  │  0  │     │
//    ├─────┼─────┼─────┼─────┼─────┼─────┼─────┐   ┌─────┼─────┼─────┼─────┼─────┼─────┼─────┤
//    │     │     │     │     │     │  b  │     │   │     │  n  │     │     │     │     │     │
//    └─────┴─────┴─────┼─────┼─────┼─────┼─────┤   ├─────┼─────┼─────┼─────┼─────┴─────┴─────┘
//                      │     │     │     │     │   │     │     │     │     │
//                      └─────┴─────┴─────┴─────┘   └─────┴─────┴─────┴─────┘
[NUM] = LAYOUT(
  _______ , KC_F1   , KC_F2   , KC_F3   , KC_F4   , KC_F5   ,                         KC_F6   , KC_F7   , KC_F8   , KC_F9   , KC_F10  , KC_F11 ,
  _______ , _______ , KC_W    , _______ , _______ , KC_T    ,                         KC_Y    , _______ , _______ , _______ , _______ , _______,
  _______ , KC_1    , KC_2    , KC_3    , KC_4    , KC_5    ,                         KC_6    , KC_7    , KC_8    , KC_9    , KC_0    , _______,
  _______ , _______ , _______ , _______ , _______ , KC_B    , _______ ,     _______ , KC_N    , _______ , _______ , _______ , _______ , _______,
                                _______ , _______ , _______ , _______ ,     _______ , _______ , _______ , _______
)
};
//clang-format on
