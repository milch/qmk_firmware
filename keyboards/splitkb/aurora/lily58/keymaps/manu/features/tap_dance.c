#include "tap_dance.h"
#include "action.h"
#include "action_layer.h"
#include "debug.h"
#include "quantum.h"
#include "repeat_key.h"

td_state_t cur_dance(tap_dance_state_t *state) {
    if (state->count == 1) {
        if (!state->pressed) {
            return TD_SINGLE_TAP;
        } else {
            return TD_SINGLE_HOLD;
        }
    } else {
        return TD_UNKNOWN;
    }
}

void handle_tap(uint16_t keycode) {
    switch (keycode) {
        case QK_REP: {
            dprintf("TD_LT Keycode is REP\n");
            uint16_t kc   = get_last_keycode();
            uint8_t  mods = get_last_mods();
            register_weak_mods(mods);
            tap_code16(kc);
            unregister_weak_mods(mods);
            break;
        }
        case QK_AREP: {
            dprintf("TD_LT Keycode is AREP\n");
            uint16_t kc = get_alt_repeat_key_keycode();
            tap_code16(kc);
            break;
        }
        default: {
            tap_code16(keycode);
            break;
        }
    }
}

void tap_dance_lt_on_each_tap(tap_dance_state_t *state, void *user_data) {
    tap_dance_lt_t *lt = user_data;

    // At this point, its all taps
    if (state->count > 1) {
        handle_tap(lt->kc);
    }
}

void send_mo(uint16_t layer, uint16_t count, bool held) {
    keyrecord_t *last_record = get_last_record();
    keyrecord_t  record      = {
              .event = last_record->event,
#ifndef NO_ACTION_TAPPING
        .tap = {.count = count},
#endif
#if defined(COMBO_ENABLE) || defined(REPEAT_KEY_ENABLE)
        .keycode = MO(layer)
#endif
    };
    record.event.pressed = held;
    process_record(&record);
}

void tap_dance_lt_finished(tap_dance_state_t *state, void *user_data) {
    tap_dance_lt_t *lt = user_data;

    lt->held = cur_dance(state);
    switch (lt->held) {
        case TD_SINGLE_TAP:
            handle_tap(lt->kc);
            break;
        case TD_SINGLE_HOLD:
            dprintf("TD_LT holding layer %d\n", lt->layer);
            send_mo(lt->layer, state->count, true);
            break;
        default:
            break;
    }
}

void tap_dance_lt_reset(tap_dance_state_t *state, void *user_data) {
    dprintln("TD_LT reset");
    tap_dance_lt_t *lt = user_data;

    // If the key was held down and now is released then switch off the layer
    if (lt->held == TD_SINGLE_HOLD) {
        send_mo(lt->layer, state->count, false);
    }
    lt->held = TD_NONE;
}

bool remember_last_key_user(uint16_t keycode, keyrecord_t *record, uint8_t *remembered_mods) {
    switch (keycode) {
        case QK_TAP_DANCE ... QK_TAP_DANCE_MAX:
            return false;
    }

    return true; // Other keys can be repeated.
}
