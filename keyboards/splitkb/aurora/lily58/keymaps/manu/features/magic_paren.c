#include "magic_paren.h"
#include "action_util.h"

typedef struct {
    uint16_t keycode;
    uint8_t  mods;
} KeyAndMods;
static KeyAndMods last_parens_buffer[MAGIC_PAREN_BUF_LEN] = {{.keycode = KC_NO, .mods = 0}};
static uint8_t    start_idx                               = 0;
static uint8_t    cur_idx                                 = 0;

void magic_paren_buf_push(uint16_t keycode, uint8_t mods) {
    last_parens_buffer[cur_idx].keycode = keycode;
    last_parens_buffer[cur_idx].mods    = mods;

    cur_idx = (cur_idx + 1) % MAGIC_PAREN_BUF_LEN;
    if (cur_idx == start_idx) {
        start_idx = (start_idx + 1) % MAGIC_PAREN_BUF_LEN;
    }
}

KeyAndMods* magic_paren_buf_peek(void) {
    if (start_idx == cur_idx) {
        return NULL;
    }
    return &last_parens_buffer[cur_idx];
}

KeyAndMods* magic_paren_buf_pop(void) {
    KeyAndMods* result = magic_paren_buf_peek();
    if (result != NULL) {
        if (cur_idx == 0) {
            cur_idx = MAGIC_PAREN_BUF_LEN;
        }
        cur_idx -= 1;
    }
    return result;
}

bool process_magic_paren(uint16_t keycode, keyrecord_t* record, uint16_t magic_paren_code) {
    const uint8_t mods       = get_mods() | get_oneshot_mods();
    bool          shift_held = mods & MOD_MASK_SHIFT;
    bool          no_mods    = mods == 0;
    KeyAndMods*   peek       = magic_paren_buf_peek();
    switch (keycode) {
        // All these push onto the stack
        case KC_LEFT_BRACKET:
            if (shift_held || no_mods) {
                magic_paren_buf_push(KC_RIGHT_BRACKET, mods);
            }
            break;
        case KC_COMMA:
            if (shift_held || no_mods) {
                magic_paren_buf_push(KC_DOT, mods);
            }
            break;
        case KC_9:
            if (shift_held) {
                magic_paren_buf_push(KC_0, mods);
            }
            break;
        // Same symbol -> pushes or pops
        case KC_QUOTE:
            if (shift_held || no_mods) {
                if (peek && peek->keycode == KC_QUOTE && peek->mods == mods) {
                    magic_paren_buf_pop();
                } else {
                    magic_paren_buf_push(KC_QUOTE, mods);
                }
            }
            break;
        case KC_GRAVE:
            if (no_mods) {
                if (peek && peek->keycode == KC_GRAVE) {
                    // We just pressed `, need to pop it off after pressing again
                    magic_paren_buf_pop();
                } else {
                    magic_paren_buf_push(KC_GRAVE, mods);
                }
            }
            break;
        // Pop off from the stack
        case KC_RIGHT_BRACKET:
        case KC_DOT:
            if (peek && peek->keycode == keycode && peek->mods == mods) {
                magic_paren_buf_pop();
            }
            break;
        case KC_0:
            if (peek && peek->keycode == keycode && shift_held) {
                magic_paren_buf_pop();
            }
            break;
        default: {
            if (keycode == magic_paren_code) {
                KeyAndMods* result;
                if (record->event.pressed) {
                    result = peek;
                } else {
                    result = magic_paren_buf_pop();
                }
                if (result != NULL) {
                    set_mods(result->mods);
                    if (record->event.pressed) {
                        register_code(result->keycode);
                    } else {
                        unregister_code(result->keycode);
                    }
                    clear_mods();
                    register_mods(mods);
                }
                return false;
            }
        }
    }
    return true;
}
