CONVERT_TO = liatris
COMMAND_ENABLE = no
OLED_ENABLE = yes
CAPS_WORD_ENABLE = yes
REPEAT_KEY_ENABLE = yes
DYNAMIC_TAPPING_TERM_ENABLE = yes
TAP_DANCE_ENABLE = yes
COMBO_ENABLE = yes
# CONSOLE_ENABLE = yes

SRC += features/select_word.c
SRC += features/achordion.c
SRC += features/magic_paren.c
SRC += features/num_word.c
SRC += features/tap_dance.c

VPATH += keyboards/gboards
