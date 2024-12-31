#ifndef __LED_DRIVERL_H
#define __LED_DRIVERL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef enum {
    RECEIVED_ON = 0x01,
    RECEIVED_OFF,
    RECEIVED_KEY,
} Received_Command;

typedef enum {
    COLOR_RED        = 0b111110000000000000000000,
    COLOR_GREEN      = 0b000000001111100000000000,
    COLOR_BLUE       = 0b000000000000000011111000,
    COLOR_WHITE      = 0b111110001111100011111000,
    COLOR_ORANGE     = 0b111110000000100000000000,
    COLOR_AQUAMARINE = 0b000000001111100000001000,
    COLOR_AQUA       = 0b000000001111100011111000,
    COLOR_YELLOW     = 0b111110000110000000000000,
    COLOR_CYAN       = 0b000000000011100011111000,
    COLOR_PINK       = 0b111110000000000000101000,
    COLOR_PURPLE     = 0b111110000000000011111000,
    COLOR_NONE       = 0b000000000000000000000000,
} Color_TypeDef;

typedef enum {
    MODE_STEADY,
    MODE_FADE,
    MODE_TWINKLE,
    MODE_SHINING,
    MODE_JUMP,
    MODE_CHASING,
    MODE_BLOOMFADE,
    MODE_WAVE,
    MODE_BREATHING,
    MODE_NUM,
} Mode_TypeDef;

typedef enum {
    FUNC_0,
    FUNC_1,
    FUNC_NUM,
} Func_TypeDef;

void control_led(Received_Command cmd);
void send_led_data();
void gen_fade_table();

#ifdef __cplusplus
}
#endif

#endif /* __LED_DRIVERL_H */
