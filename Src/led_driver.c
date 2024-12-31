#include "led_driver.h"

extern TIM_HandleTypeDef update_htim, delay_htim;

#define FADE_STEP (128)
float fade_table[FADE_STEP];
const uint8_t brightness_0[5]     = {0b00000000, 0b00110001, 0b01100010, 0b10010100, 0b11000101};
const uint8_t brightness_1[5]     = {0b00110001, 0b01100010, 0b10010100, 0b11000101, 0b11110111};
const Color_TypeDef color_of_6[6] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_PURPLE, COLOR_WHITE};
#define COLOR_NUM (11)
const Color_TypeDef colors[COLOR_NUM] = {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_PURPLE,
    COLOR_WHITE,
    COLOR_ORANGE,
    COLOR_AQUAMARINE,
    COLOR_AQUA,
    COLOR_CYAN,
    COLOR_PINK,
};

uint8_t shut_down = 1;
uint8_t data_to_send[6];
uint8_t data_to_send_size;
uint8_t send_continuously;
uint8_t initial_sned;

uint8_t mode_index = MODE_FADE;
uint8_t func_index;
uint8_t color_index;

uint8_t mode_step_num;
volatile uint8_t mode_step_now;
uint32_t mode_step_change_time_ms;
volatile uint32_t mode_trigger_time_ms; //

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
    data_to_send_size = 4;
    data_to_send[0]   = 0b00011001;
    set_color(1, COLOR_NONE);
    send_one_frame(data_to_send, data_to_send_size);
}

void switch_color()
{
    if (++color_index == COLOR_NUM) {
        color_index = 0;
    }
    data_to_send_size = 4;
    data_to_send[0]   = 0b00011001;
    set_color(1, COLOR_NONE);
    send_one_frame(data_to_send, data_to_send_size);
    initial_sned = 1;
}

void switch_func()
{
    if (++func_index == FUNC_NUM) {
        func_index = 0;
    }
    data_to_send_size = 4;
    data_to_send[0]   = 0b00011001;
    set_color(1, COLOR_NONE);
    send_one_frame(data_to_send, data_to_send_size);
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
            if (func_index == 0) {
            } else {
                if (++color_index == COLOR_NUM) {
                    color_index = 0;
                }
            }

            // turn off
            data_to_send_size = 4;
            data_to_send[0]   = 0b00011001;
            set_color(1, COLOR_NONE);
            send_one_frame(data_to_send, data_to_send_size);

            uint8_t r = (colors[color_index] & 0xff0000) >> 16;
            uint8_t g = (colors[color_index] & 0x00ff00) >> 8;
            uint8_t b = (colors[color_index] & 0x0000ff) >> 0;

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
            // switch_color();
            switch_func();
            break;
        default:
            break;
    }
}
