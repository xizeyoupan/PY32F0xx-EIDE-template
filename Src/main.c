#include "main.h"
#include "led_driver.h"
#include "YG350.h"

TIM_HandleTypeDef update_htim, delay_htim;

UART_HandleTypeDef UartHandle;
GPIO_InitTypeDef GPIO_InitStruct;

#define KEY_DOWN_HIHG_CNT (40)
uint16_t f_key_continuous_down_cnt;
uint8_t f_key_down_cnt;
uint8_t f_key_press_down;

uint16_t st_key_continuous_down_cnt;
uint8_t st_key_down_cnt;
uint8_t st_key_press_down;

uint16_t us_cnt;

void check_fkey()
{
    if (HAL_GPIO_ReadPin(KEY_PIN_PORT, KEY_PIN)) {
        if (f_key_down_cnt > 0) {
            f_key_down_cnt--;
        } else {
            f_key_continuous_down_cnt = 0;
            if (f_key_press_down) {
                f_key_press_down = 0;
                control_led(RECEIVED_KEY);
            }
        }
    } else {
        if (f_key_continuous_down_cnt < 3000) {
            f_key_continuous_down_cnt++;
        } else {
            f_key_press_down = 0;
            control_led(RECEIVED_OFF);
            return;
        }

        if (f_key_down_cnt < KEY_DOWN_HIHG_CNT) {
            f_key_down_cnt++;

        } else {
            f_key_press_down = 1;
        }
    }
}

void check_stkey()
{
    if (HAL_GPIO_ReadPin(ST_PIN_PORT, ST_PIN)) {
        if (st_key_down_cnt > 0) {
            st_key_down_cnt--;
        } else {
            st_key_continuous_down_cnt = 0;
            if (st_key_press_down) {
                st_key_press_down = 0;
                // TODO
            }
        }
    } else {
        if (st_key_continuous_down_cnt < 3000) {
            st_key_continuous_down_cnt++;
        } else {
            st_key_press_down = 0;
            return;
        }
        if (st_key_down_cnt < KEY_DOWN_HIHG_CNT) {
            st_key_down_cnt++;

        } else {
            st_key_press_down = 1;
        }
    }
}

void delay_us(TIM_HandleTypeDef *handle, uint16_t us)
{
    __HAL_TIM_SET_COUNTER(handle, 0);
    __HAL_TIM_ENABLE(handle);
    while (__HAL_TIM_GET_COUNTER(handle) < us) {}
    __HAL_TIM_DISABLE(handle);
}

uint8_t data_buf[128];
uint32_t fast_printf(UART_HandleTypeDef *uartHandle, const char *str, ...)
{
    va_list args;
    va_start(args, str);
    uint8_t len = vsprintf((char *)data_buf, str, args);
    HAL_UART_Transmit_IT(uartHandle, data_buf, len);
    va_end(args);
    return len;
}

static void init_uart()
{
    HAL_Delay(5000);
    __HAL_RCC_USART1_CLK_ENABLE();

    /* USART
    PA14：TX,
    PA13：RX
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_14;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    UartHandle.Instance        = USART1;
    UartHandle.Init.BaudRate   = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX_RX;

    if (HAL_UART_Init(&UartHandle) != HAL_OK) {
        APP_ErrorHandler();
    }
}

void APP_SystemClockConfig(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* 振荡器配置 */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;   /* 选择振荡器HSE,HSI,LSI */
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;               /* 开启HSI */
    RCC_OscInitStruct.HSIDiv              = RCC_HSI_DIV1;             /* HSI 1分频 */
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz; /* 配置HSI时钟24MHz */

    /* 配置振荡器 */
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        APP_ErrorHandler();
    }

    /* 时钟源配置 */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; /* 选择配置时钟 HCLK,SYSCLK,PCLK1 */
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;                                            /* 选择HSI作为系统时钟 */
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;                                                 /* AHB时钟 1分频 */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                                   /* APB时钟 1分频 */
    /* 配置时钟源 */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        APP_ErrorHandler();
    }
}

static void APP_Config(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_TIM16_CLK_ENABLE();
    __HAL_RCC_TIM1_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM16_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM16_IRQn);

    GPIO_InitStruct.Pin   = KEY_PIN;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(KEY_PIN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ST_PIN;
    HAL_GPIO_Init(ST_PIN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = CTRL_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(CTRL_PIN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ST_PIN;
    HAL_GPIO_Init(ST_PIN_PORT, &GPIO_InitStruct);

    //  for delay_us(uint16_t us) use, when call delay_us the program will be blocked
    delay_htim.Instance               = TIM1;
    delay_htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    delay_htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    delay_htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    delay_htim.Init.Period            = 65536 - 1;
    delay_htim.Init.Prescaler         = 24 - 1;
    delay_htim.Init.RepetitionCounter = 1 - 1;
    if (HAL_TIM_Base_Init(&delay_htim) != HAL_OK) {
        APP_ErrorHandler();
    }

    //  updates and interrupts every 40 us

    update_htim.Instance               = TIM16;
    update_htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    update_htim.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    update_htim.Init.CounterMode       = TIM_COUNTERMODE_UP;
    update_htim.Init.Period            = 100 - 1;
    update_htim.Init.Prescaler         = 24 - 1;
    update_htim.Init.RepetitionCounter = 1 - 1;
    if (HAL_TIM_Base_Init(&update_htim) != HAL_OK) {
        APP_ErrorHandler();
    }

    if (HAL_TIM_Base_Start_IT(&update_htim) != HAL_OK) /* TIM1初始化 */
    {
        APP_ErrorHandler();
    }
}

extern unsigned int trans_recv_overtime;
extern unsigned char trans_recv_channel;
extern unsigned char trans_recv_start;

int main(void)
{
    /* 初始化所有外设，Flash接口，SysTick */
    HAL_Init();
    APP_SystemClockConfig();

    APP_Config();
    YG350_init();

    // init_uart();
    // fast_printf(&UartHandle, "test\n");

    gen_fade_table();
    control_led(RECEIVED_ON);

    while (1) {
        send_led_data();

        if (trans_recv_result()) {
            trans_recv_overtime = 0;
            trans_recv_start    = 1;
            uint8_t recv_data[4];
            YG350_read(recv_data, 1);
            gen_fade_table();
        }

        if (trans_recv_start) {
            trans_recv_start = 0;
            switch (trans_recv_channel) {
                case 0:
                    YG350_recv(SEND_CHANNEL_1);
                    break;
                case 1:
                    YG350_recv(SEND_CHANNEL_2);
                    break;
                case 2:
                    YG350_recv(SEND_CHANNEL_3);
                    break;
                default:
                    break;
            }
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &update_htim) {
        us_cnt += 100;
        if (us_cnt >= 1000) {
            us_cnt = 0;
            check_fkey();
        }

        trans_recv_overtime++;
        if (trans_recv_overtime >= TRANS_RECV_OVERTIME) {
            trans_recv_overtime = 0;
            trans_recv_channel++;
            if (trans_recv_channel >= TRANS_RECV_CHANNEL) {
                trans_recv_channel = 0;
            }
            trans_recv_start = 1;
        }
    }
}

/**
 * @brief  错误执行函数
 * @param  无
 * @retval 无
 */
void APP_ErrorHandler(void)
{
    /* 无限循环 */
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  输出产生断言错误的源文件名及行号
 * @param  file：源文件名指针
 * @param  line：发生断言错误的行号
 * @retval 无
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* 用户可以根据需要添加自己的打印信息,
       例如: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* 无限循环 */
    while (1) {
    }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
