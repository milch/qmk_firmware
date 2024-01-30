#pragma once

#include "action.h"

#ifndef MAGIC_PAREN_BUF_LEN
#    define MAGIC_PAREN_BUF_LEN 16
#endif

bool process_magic_paren(uint16_t keycode, keyrecord_t* record, uint16_t magic_paren_code);
