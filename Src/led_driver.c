#include "led_driver.h"

extern TIM_HandleTypeDef update_htim, delay_htim;

#define FADE_STEP (128)
float fade_table[FADE_STEP];
const Color_TypeDef colors_of_7[7] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_CYAN, COLOR_PURPLE, COLOR_WHITE};
#define COLOR_NUM (11)
const Color_TypeDef colors[COLOR_NUM] = {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_AQUA,
    COLOR_PURPLE,
    COLOR_WHITE,
    COLOR_ORANGE,
    COLOR_AQUAMARINE,
    COLOR_CYAN,
    COLOR_PINK,
};

uint8_t shut_down = 1;
uint8_t data_to_send[6];
uint8_t data_to_send_size;
uint8_t initial_sned;

uint8_t mode_index = MODE_WAVE;
uint8_t func_index;
uint8_t color_index;
uint8_t auto_color_index;

void gen_fade_table()
{
    for (uint8_t i = 0; i < FADE_STEP; i++) {
        fade_table[i] = i * 1.0 / FADE_STEP;
    }
}

void Send_AS1828_8bits(uint8_t temp_cmd)
{
    for (int8_t i = 0; i < 8; i++) {
        if (((temp_cmd << i) & 0x80) == 0) {
            HAL_GPIO_WritePin(CTRL_PIN_PORT, CTRL_PIN, 0);
            delay_us(&delay_htim, 100);
            HAL_GPIO_WritePin(CTRL_PIN_PORT, CTRL_PIN, 1);
            delay_us(&delay_htim, 100);
        } else {
            HAL_GPIO_WritePin(CTRL_PIN_PORT, CTRL_PIN, 0);
            delay_us(&delay_htim, 100);
            HAL_GPIO_WritePin(CTRL_PIN_PORT, CTRL_PIN, 1);
            delay_us(&delay_htim, 300);
        }
    }
}

void Send_AS1828_Latch(void)
{
    HAL_GPIO_WritePin(CTRL_PIN_PORT, CTRL_PIN, 0);
    delay_us(&delay_htim, 100);
    HAL_GPIO_WritePin(CTRL_PIN_PORT, CTRL_PIN, 1);
    delay_us(&delay_htim, 700);
}

void send_one_frame(uint8_t *pData, uint8_t size)
{
    for (uint8_t i = 0; i < size; i++) {
        Send_AS1828_8bits(*(pData + i));
    }
    Send_AS1828_Latch();
}

void set_color(uint8_t color_start, Color_TypeDef color)
{
    data_to_send[color_start]     = (color & 0xff0000) >> 16;
    data_to_send[color_start + 1] = (color & 0x00ff00) >> 8;
    data_to_send[color_start + 2] = (color & 0x0000ff) >> 0;
}

void switch_mode()
{
    if (++mode_index == MODE_NUM) {
        mode_index = 0;
    }
    initial_sned = 1;
}

void switch_color()
{
    if (mode_index == MODE_CHASING) {
        if (++color_index == 7) {
            color_index = 0;
        }
    } else {
        if (++color_index == COLOR_NUM) {
            color_index = 0;
        }
    }

    initial_sned = 1;
}

void switch_func()
{
    if (++func_index == FUNC_NUM) {
        func_index = 0;
    }
    initial_sned = 1;
}

uint8_t need_break()
{
    if (initial_sned || shut_down) return 1;
    return 0;
}

void send_led_data()
{
    if (shut_down) {
        HAL_GPIO_WritePin(CTRL_PIN_PORT, CTRL_PIN, 0);
        return;
    }

    if (initial_sned) {
        initial_sned = 0;
    }

    uint8_t r;
    uint8_t g;
    uint8_t b;

    switch (mode_index) {
        case MODE_STEADY:
            data_to_send_size = 5;
            data_to_send[0]   = 0b01001001;
            for (uint8_t i = 0; i < 8; i++) {
                data_to_send[1] = 0b0111000 + i;
                set_color(2, colors[color_index]);
                send_one_frame(data_to_send, data_to_send_size);
            }
            while (!need_break()) {
            }

            break;
        case MODE_FADE:

            // turn off
            data_to_send_size = 4;
            data_to_send[0]   = 0b00011001;
            set_color(1, COLOR_NONE);
            send_one_frame(data_to_send, data_to_send_size);

            if (func_index == 0) {
                r = (colors[color_index] & 0xff0000) >> 16;
                g = (colors[color_index] & 0x00ff00) >> 8;
                b = (colors[color_index] & 0x0000ff) >> 0;
            } else {
                if (++auto_color_index >= 7) {
                    auto_color_index = 0;
                }
                r = (colors[auto_color_index] & 0xff0000) >> 16;
                g = (colors[auto_color_index] & 0x00ff00) >> 8;
                b = (colors[auto_color_index] & 0x0000ff) >> 0;
            }

            for (uint8_t i = 0; i < FADE_STEP; i++) {
                uint8_t _r = r * fade_table[i];
                uint8_t _g = g * fade_table[i];
                uint8_t _b = b * fade_table[i];

                data_to_send[1] = _r;
                data_to_send[2] = _g;
                data_to_send[3] = _b;
                send_one_frame(data_to_send, data_to_send_size);
                if (need_break()) return;
            }

            for (int8_t i = FADE_STEP - 1; i >= 0; i--) {
                uint8_t _r = r * fade_table[i];
                uint8_t _g = g * fade_table[i];
                uint8_t _b = b * fade_table[i];

                data_to_send[1] = _r;
                data_to_send[2] = _g;
                data_to_send[3] = _b;
                send_one_frame(data_to_send, data_to_send_size);
                if (need_break()) return;
            }

            data_to_send[0] = 0b00011001;
            set_color(1, COLOR_NONE);
            send_one_frame(data_to_send, data_to_send_size);
            HAL_Delay(10);

            break;
        case MODE_TWINKLE:
            data_to_send_size               = 6;
            data_to_send[0]                 = 0b00110001;
            data_to_send[1]                 = 0b00000001;
            const uint16_t twinkle_delay_ms = 50;

            for (uint8_t mode_step_run_cnt = 0; mode_step_run_cnt < 21; mode_step_run_cnt++) {
                if (need_break()) return;

                switch (mode_step_run_cnt) {
                    case 1:
                    case 3:
                    case 5:
                    case 7:
                    case 9:
                        data_to_send[2] = 0b00000000;
                        set_color(3, colors[color_index]);
                        send_one_frame(data_to_send, data_to_send_size);
                        HAL_Delay(twinkle_delay_ms);
                        break;
                    case 2:
                    case 4:
                    case 6:
                    case 8:
                    case 10:
                    case 12:
                    case 14:
                    case 16:
                    case 18:
                    case 20:
                        data_to_send[2] = 0b00000000;
                        set_color(3, COLOR_NONE);
                        send_one_frame(data_to_send, data_to_send_size);

                        data_to_send[2] = 0b00000001;
                        send_one_frame(data_to_send, data_to_send_size);
                        HAL_Delay(twinkle_delay_ms);
                        break;
                    case 11:
                    case 13:
                    case 15:
                    case 17:
                    case 19:
                        data_to_send[2] = 0b00000001;
                        set_color(3, colors[color_index]);
                        send_one_frame(data_to_send, data_to_send_size);
                        HAL_Delay(twinkle_delay_ms);
                        break;
                    default:
                        break;
                }
            }
            break;
        case MODE_SHINING:
            data_to_send_size = 5;
            data_to_send[0]   = 0b01001001;
            if (func_index == 0) {
                for (uint8_t i = 0; i < 16; i++) {
                    if (need_break()) return;
                    data_to_send[1] = ((3 + i) % 16) + 0b11110000;
                    set_color(2, colors[color_index]);
                    send_one_frame(data_to_send, data_to_send_size);

                    data_to_send[1] = 0b11110000 + i;
                    set_color(2, COLOR_NONE);
                    send_one_frame(data_to_send, data_to_send_size);
                }
            } else {
                for (uint8_t i = 0; i < 16; i++) {
                    if (need_break()) return;
                    data_to_send[1] = ((3 + i) % 16) + 0b11110000;
                    set_color(2, COLOR_NONE);
                    send_one_frame(data_to_send, data_to_send_size);

                    data_to_send[1] = 0b11110000 + i;
                    set_color(2, colors[color_index]);
                    send_one_frame(data_to_send, data_to_send_size);
                }
            }
            break;
        case MODE_JUMP:
            data_to_send_size            = 5;
            data_to_send[0]              = 0b01001001;
            const uint16_t jump_delay_ms = 1000;

            for (uint8_t i = 0; i < 8; i++) {
                data_to_send[1] = 0b0111000 + i;
                set_color(2, colors[auto_color_index]);
                send_one_frame(data_to_send, data_to_send_size);
            }

            if (++auto_color_index >= 7) {
                auto_color_index = 0;
            }

            for (uint8_t i = 0; i < jump_delay_ms / 100; i++) {
                if (need_break()) return;
                HAL_Delay(100);
            }

            break;
        case MODE_CHASING:
            data_to_send_size = 5;
            data_to_send[0]   = 0b01100001;
            data_to_send[1]   = 0b00000000;
            for (uint8_t i = 0; i < 51; i++) {
                if (need_break()) return;

                switch (color_index) {
                    case 0:
                        data_to_send[2] = 0b00001000;
                        break;
                    case 1:
                        data_to_send[2] = 0b00000100;
                        break;
                    case 2:
                        data_to_send[2] = 0b00000010;
                        break;
                    case 3:
                        data_to_send[2] = 0b00001100;
                        break;
                    case 4:
                        data_to_send[2] = 0b00000110;
                        break;
                    case 5:
                        data_to_send[2] = 0b00001010;
                        break;
                    case 6:
                        data_to_send[2] = 0b00001110;
                        break;
                    default:
                        break;
                }
                data_to_send[3] = i * 5;
                send_one_frame(data_to_send, data_to_send_size);
            }
            break;
        case MODE_BLOOMFADE:
            data_to_send_size = 6;
            data_to_send[0]   = 0b00110001;
            data_to_send[1]   = 0b00000001;

            for (uint8_t i = 0; i < 2; i++) {
                data_to_send[2] = i;
                set_color(3, COLOR_NONE);
                send_one_frame(data_to_send, data_to_send_size);
            }

            data_to_send[1] = 0b00001111;
            for (uint8_t i = 16; i > 0; i--) {
                data_to_send[2] = 0b00000000;
                set_color(3, colors[color_index]);
                send_one_frame(data_to_send, data_to_send_size);

                for (uint8_t j = 0; j < i - 1; j++) {
                    if (need_break()) return;

                    data_to_send[2] = j;
                    set_color(3, colors[color_index]);
                    send_one_frame(data_to_send, data_to_send_size);

                    set_color(3, COLOR_NONE);
                    send_one_frame(data_to_send, data_to_send_size);
                }

                data_to_send[2] = i - 1;
                set_color(3, colors[color_index]);
                send_one_frame(data_to_send, data_to_send_size);
            }

            for (uint8_t i = 0; i < 16; i++) {
                data_to_send[2] = i;
                set_color(3, COLOR_NONE);
                send_one_frame(data_to_send, data_to_send_size);
            }
            break;
        case MODE_WAVE:

            data_to_send_size = 6;
            data_to_send[0]   = 0b00110001;

            data_to_send[1] = 0b00000001;
            for (uint8_t i = 0; i < 2; i++) {
                data_to_send[2] = i;
                set_color(3, COLOR_NONE);
                send_one_frame(data_to_send, data_to_send_size);
            }

            data_to_send[1] = 0b00011111;

            r = (colors[color_index] & 0xff0000) >> 16;
            g = (colors[color_index] & 0x00ff00) >> 8;
            b = (colors[color_index] & 0x0000ff) >> 0;

            data_to_send[1]     = 0b00011110;
            const int wave_step = 6;

            for (int8_t i = 0; i < 13 + wave_step; i++) {
                if (need_break()) return;

                for (int8_t j = 0; j < wave_step; j++) {
                    if (i - j < 0) break;
                    if (i - j > 12) continue;

                    data_to_send[2] = (i - j) << 1;

                    uint8_t _r      = r * (1.0 + j) / wave_step * .4;
                    uint8_t _g      = g * (1.0 + j) / wave_step * .4;
                    uint8_t _b      = b * (1.0 + j) / wave_step * .4;
                    data_to_send[3] = _r;
                    data_to_send[4] = _g;
                    data_to_send[5] = _b;
                    for (size_t k = 0; k < 2; k++) {
                        send_one_frame(data_to_send, data_to_send_size);
                    }
                }
                // HAL_Delay(500);
            }

            data_to_send[1] = 0b00011110;
            for (int8_t i = 12; i >= 0; i--) {
                if (need_break()) return;

                for (uint8_t j = 0; j < 2; j++) {
                    data_to_send[2] = i << 1;
                    set_color(3, COLOR_NONE);
                    send_one_frame(data_to_send, data_to_send_size);
                }
            }
            HAL_Delay(200);
            break;
        case MODE_BREATHING:

            // turn off
            data_to_send_size = 4;
            data_to_send[0]   = 0b00011001;
            set_color(1, COLOR_NONE);
            send_one_frame(data_to_send, data_to_send_size);

            r = (colors[color_index] & 0xff0000) >> 16;
            g = (colors[color_index] & 0x00ff00) >> 8;
            b = (colors[color_index] & 0x0000ff) >> 0;

            for (uint8_t i = 0; i < 20; i++) {
                uint8_t _r = r * (0.05 + i * 0.05);
                uint8_t _g = g * (0.05 + i * 0.05);
                uint8_t _b = b * (0.05 + i * 0.05);

                data_to_send[1] = _r;
                data_to_send[2] = _g;
                data_to_send[3] = _b;

                for (uint8_t j = 0; j < 32; j++) {
                    send_one_frame(data_to_send, data_to_send_size);
                    if (need_break()) return;
                }
            }

            for (int8_t i = 19; i >= 0; i--) {
                uint8_t _r = r * (0.05 + i * 0.05);
                uint8_t _g = g * (0.05 + i * 0.05);
                uint8_t _b = b * (0.05 + i * 0.05);

                data_to_send[1] = _r;
                data_to_send[2] = _g;
                data_to_send[3] = _b;

                for (uint8_t j = 0; j < 32; j++) {
                    send_one_frame(data_to_send, data_to_send_size);
                    if (need_break()) return;
                }
            }

            data_to_send[0] = 0b00011001;
            set_color(1, COLOR_NONE);
            send_one_frame(data_to_send, data_to_send_size);
            HAL_Delay(100);
            break;
        default:
            break;
    }
}

void control_led(Received_Command cmd)
{
    if (shut_down == 1 && cmd == RECEIVED_KEY) cmd = RECEIVED_ON;

    if (shut_down == 1 && cmd != RECEIVED_ON) return; // Do nothing if shut down.

    switch (cmd) {
        case RECEIVED_ON:
            if (shut_down == 0) return;

            data_to_send_size = 4;
            data_to_send[0]   = 0b00011001;
            set_color(1, COLOR_NONE);
            HAL_GPIO_WritePin(CTRL_PIN_PORT, CTRL_PIN, GPIO_PIN_SET);
            delay_us(&delay_htim, 1000);
            send_one_frame(data_to_send, data_to_send_size);

            initial_sned = 1;
            shut_down    = 0;
            break;
        case RECEIVED_OFF:
            shut_down = 1;
            HAL_GPIO_WritePin(CTRL_PIN_PORT, CTRL_PIN, GPIO_PIN_RESET);
            break;
        case RECEIVED_KEY:
            // switch_mode();
            switch_color();
            // switch_func();
            break;
        default:
            break;
    }
}

void set_timeout()
{
}