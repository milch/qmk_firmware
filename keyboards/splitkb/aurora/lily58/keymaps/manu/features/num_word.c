#include "quantum_keycodes.h"
#include "keycodes.h"
#include "action_layer.h"
#include "layers.h"
#include "keymap_colemak.h"

static bool num_word_on = false;

bool num_word_enabled(void) {
    return num_word_on;
}

void enable_num_word(void) {
    if (num_word_on) return;
    num_word_on = true;
    layer_on(NUM);
}

void disable_num_word(void) {
    if (!num_word_on) return;
    num_word_on = false;
    layer_off(NUM);
}

void num_word_toggle(void) {
    if (num_word_on) {
        disable_num_word();
    } else {
        enable_num_word();
    }
}

__attribute__((weak)) bool num_word_press_user(uint16_t keycode, const keyrecord_t *record) {
    if (num_word_on) {
        switch (keycode) {
            // Keycodes which should not disable num word mode.
            case KC_1 ... KC_0:
            case CM_EQL:
            case CM_MINS:
            case CM_DOT:

            // Numpad keycodes
            case KC_P1 ... KC_P0:
            case KC_PSLS ... KC_PPLS:
            case KC_PDOT:

            // Misc
            case CM_UNDS:
            case KC_BSPC:
            case KC_DEL:
            case CM_PLUS:
                return false;

            default:
                if (record->event.pressed) {
                    return true;
                }
                break;
        }
        return false;
    }
    return false;
}

bool process_num_word(uint16_t keycode, const keyrecord_t *record, uint16_t toggle_keycode) {
    if (keycode == toggle_keycode) {
        if (record->event.pressed) {
            num_word_toggle();
        }
        return true;
    }

    if (num_word_on) {
        if (record->event.pressed) {
            // Get the base keycode of a mod or layer tap key
            switch (keycode) {
                case QK_MOMENTARY ... QK_MOMENTARY_MAX:
                case QK_TAP_DANCE ... QK_TAP_DANCE_MAX:
                case QK_MOD_TAP ... QK_MOD_TAP_MAX:
                case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
#ifndef NO_ACTION_TAPPING
                    // Earlier return if this has not been considered tapped yet
                    if (record->tap.count == 0) return true;
#endif
                    keycode = keycode & 0xFF;
                    break;
                default:
                    break;
            }
            // check if the case modes have been terminated
            if (num_word_press_user(keycode, record)) {
                disable_num_word();
            }

            return true;
        }
        return true;
    }
    return true;
}
