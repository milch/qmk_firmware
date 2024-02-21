#pragma once

#include "process_tap_dance.h"

// Tap Dance keycodes
enum td_keycodes {
    LT_SYM_REP, // MO(SYM) when held, REPEAT when tapped
    LT_SYM_AREP // MO(SYM) when held, ALTERNATE_REPEAT when tapped
};

typedef struct {
    uint16_t layer;
    uint16_t kc;
    uint16_t held;
} tap_dance_lt_t;

// Define a type containing as many tapdance states as you need
typedef enum { TD_NONE, TD_UNKNOWN, TD_SINGLE_TAP, TD_SINGLE_HOLD, TD_DOUBLE_TAP } td_state_t;

// Function to determine the current tapdance state
td_state_t cur_dance(tap_dance_state_t *state);

// `finished` and `reset` functions for each tapdance keycode
void tap_dance_lt_on_each_tap(tap_dance_state_t *state, void *user_data);
void tap_dance_lt_finished(tap_dance_state_t *state, void *user_data);
void tap_dance_lt_reset(tap_dance_state_t *state, void *user_data);

#define ACTION_TAP_DANCE_LT(layer, kc) \
    { .fn = {tap_dance_lt_on_each_tap, tap_dance_lt_finished, tap_dance_lt_reset}, .user_data = (void *)&((tap_dance_lt_t){layer, kc, 0}), }
