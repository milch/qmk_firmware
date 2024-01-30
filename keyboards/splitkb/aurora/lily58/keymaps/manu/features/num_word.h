#pragma once
#include "action.h"

bool                       num_word_enabled(void);
__attribute__((weak)) bool num_word_press_user(uint16_t keycode, const keyrecord_t *record);
void                       enable_num_word(void);
void                       disable_num_word(void);
void                       num_word_toggle(void);
bool                       process_num_word(uint16_t keycode, const keyrecord_t *record, uint16_t toggle_keycode);
