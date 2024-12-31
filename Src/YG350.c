// Include -------------------------------------------------------------------------------------------------------------

#include "YG350.h"
#include "main.h"
// ---------------------------------------------------------------------------------------------------------------------

unsigned int trans_recv_overtime;
unsigned char trans_recv_channel;
unsigned char trans_recv_start;

void delay_1us()
{
    for (uint8_t i = 0; i < 20; i++) {
        __NOP();
    }
}

void delay_5us()
{
    delay_1us();
    delay_1us();
    delay_1us();
    delay_1us();
    delay_1us();
}

void delay_ms(uint16_t t)
{
    HAL_Delay(t);
}

void YG350_set_sda_out()
{
    HAL_GPIO_DeInit(SDA_PIN_PORT, SDA_PIN);
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin   = SDA_PIN;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SDA_PIN_PORT, &GPIO_InitStruct);
}

void YG350_set_sda_in()
{
    HAL_GPIO_DeInit(SDA_PIN_PORT, SDA_PIN);
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin   = SDA_PIN;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SDA_PIN_PORT, &GPIO_InitStruct);
}

#define YG350_I2C_SET_SCL_H                                 \
    HAL_GPIO_WritePin(SCL_PIN_PORT, SCL_PIN, GPIO_PIN_SET); \
    delay_1us()

#define YG350_I2C_SET_SCL_L                                   \
    HAL_GPIO_WritePin(SCL_PIN_PORT, SCL_PIN, GPIO_PIN_RESET); \
    delay_1us()

#define YG350_I2C_SET_SDA_OUT YG350_set_sda_out()
#define YG350_I2C_SET_SDA_IN  YG350_set_sda_in()

#define YG350_I2C_SET_SDA_H                                   \
    HAL_GPIO_WritePin(SDA_PIN_PORT, SDA_PIN, GPIO_PIN_RESET); \
    delay_1us()
#define YG350_I2C_SET_SDA_L                                   \
    HAL_GPIO_WritePin(SDA_PIN_PORT, SDA_PIN, GPIO_PIN_RESET); \
    delay_1us()

#define YG350_I2C_SET_SDA_READ HAL_GPIO_ReadPin(SDA_PIN_PORT, SDA_PIN)

#define YG350_SET_ADDR         0x12, 0x34, 0x56, 0x78
#define YG350_SET_SPEED_62K5

/*
 */
#ifdef YG350_I2C_SET_SDA_IN
#define YG350_I2C_SDA_IN() YG350_I2C_SET_SDA_IN
#else
#error Undefine "YG350_I2C_SET_SDA_IN"
#endif

#ifdef YG350_I2C_SET_SDA_OUT
#define YG350_I2C_SDA_OUT() YG350_I2C_SET_SDA_OUT
#else
#error Undefine "YG350_I2C_SET_SDA_OUT"
#endif

#ifdef YG350_I2C_SET_SCL_H
#define YG350_I2C_SCL_H() YG350_I2C_SET_SCL_H
#else
#error Undefine "YG350_I2C_SET_SCL_H"
#endif

#ifdef YG350_I2C_SET_SCL_L
#define YG350_I2C_SCL_L() YG350_I2C_SET_SCL_L
#else
#error Undefine "YG350_I2C_SET_SCL_L"
#endif

#ifdef YG350_I2C_SET_SDA_H
#define YG350_I2C_SDA_H() YG350_I2C_SET_SDA_H
#else
#error Undefine "YG350_I2C_SET_SDA_H"
#endif

#ifdef YG350_I2C_SET_SDA_L
#define YG350_I2C_SDA_L() YG350_I2C_SET_SDA_L
#else
#error Undefine "YG350_I2C_SET_SDA_L"
#endif

#ifdef YG350_I2C_SET_SDA_READ
#define YG350_I2C_SDA_READ() YG350_I2C_SET_SDA_READ
#else
#error Undefine "YG350_I2C_SET_SDA_READ"
#endif

#ifdef YG350_SET_ADDR
#define YG350_ADDR YG350_SET_ADDR
#else
#error Undefine "YG350_SET_ADDR"
#endif

#define YG350_set_addr(addr) __YG350_SET_ADDR(addr)
#define __YG350_SET_ADDR(addr1, addr2, addr3, addr4) \
    YG350_reg_write(0x24, addr1, addr2);             \
    YG350_reg_write(0x27, addr3, addr4);

// ---------------------------------------------------------------------------------------------------------------------

#define MODE_24G   0x0F, 0x64, 0x4C // 2.4G模式
#define MODE_BLE   0x0F, 0xEC, 0x4C // BLE模式
#define FIFO_RESET 0x34, 0x80, 0x80 // 复位FIFO指针
#define IDLE       0x07, 0x00, 0x00 // 进入IDLE状态
#define SLEEP      0x23, 0x43, 0x00 // 休眠
#define WAKEUP     0x38, 0xBF, 0xFE // 唤醒
#define RESET      0x38, 0xBF, 0xFD // 复位

// ---------------------------------------------------------------------------------------------------------------------

unsigned char YG350_reg_h;
unsigned char YG350_reg_l;

// ---------------------------------------------------------------------------------------------------------------------

void YG350_i2c_start(void)
{
    YG350_I2C_SDA_OUT();
    YG350_I2C_SCL_L();
    YG350_I2C_SDA_H();
    delay_5us();
    YG350_I2C_SCL_H();
    YG350_I2C_SDA_L();
    delay_5us();
    YG350_I2C_SCL_L();
    delay_5us();
}

void YG350_i2c_stop(void)
{
    YG350_I2C_SDA_OUT();
    YG350_I2C_SCL_L();
    YG350_I2C_SDA_L();
    delay_5us();
    YG350_I2C_SCL_H();
    YG350_I2C_SDA_H();
    delay_5us();
}

unsigned char YG350_i2c_send_byte(unsigned char send_data)
{
    unsigned char i;
    YG350_I2C_SDA_OUT();
    for (i = 0; i < 8; i++) {
        if (send_data & 0x80) {
            YG350_I2C_SDA_H();
        } else {
            YG350_I2C_SDA_L();
        }
        send_data <<= 1;
        YG350_I2C_SCL_H();
        delay_5us();
        YG350_I2C_SCL_L();
        delay_5us();
    }
    YG350_I2C_SDA_IN();
    YG350_I2C_SCL_H();
    delay_5us();
    for (i = 0; i < 100; i++) {
        if (!YG350_I2C_SDA_READ()) {
            YG350_I2C_SCL_L();
            delay_5us();
            return 1;
        }
    }
    YG350_I2C_SCL_L();
    delay_5us();
    return 0;
}

unsigned char YG350_i2c_recv_byte(unsigned char ack)
{
    unsigned char i;
    unsigned char recv;
    YG350_I2C_SDA_IN();
    recv = 0;
    for (i = 0; i < 8; i++) {
        recv <<= 1;
        YG350_I2C_SCL_H();
        delay_5us();
        if (YG350_I2C_SDA_READ()) {
            recv++;
        }
        YG350_I2C_SCL_L();
        delay_5us();
    }
    YG350_I2C_SDA_OUT();
    if (ack) {
        YG350_I2C_SDA_L();
    } else {
        YG350_I2C_SDA_H();
    }
    YG350_I2C_SCL_H();
    delay_5us();
    YG350_I2C_SCL_L();
    delay_5us();
    return recv;
}

void YG350_reg_write(unsigned char addr, unsigned char data_h, unsigned char data_l)
{
    YG350_i2c_start();
    YG350_i2c_send_byte(addr & 0x7F);
    YG350_i2c_send_byte(data_h);
    YG350_i2c_send_byte(data_l);
    YG350_i2c_stop();
}

void YG350_reg_read(unsigned char addr)
{
    YG350_i2c_start();
    YG350_i2c_send_byte(addr | 0X80);
    YG350_reg_h = YG350_i2c_recv_byte(1);
    YG350_reg_l = YG350_i2c_recv_byte(0);
    YG350_i2c_stop();
}

unsigned char YG350_init(unsigned int power)
{
    delay_ms(120); // 上电等待电源稳定
    YG350_reg_write(WAKEUP);
    delay_ms(10);
    YG350_reg_write(RESET);
    delay_ms(10);
    YG350_reg_read(0x00);
    delay_ms(50);
    if (YG350_reg_h != 0x6F || YG350_reg_l != 0XE0) { return 0; }
    YG350_reg_write(0x01, 0x57, 0x81);        // 固定初始化
    YG350_reg_write(0x1A, 0x3A, 0x00);        // 调制幅度设置
    YG350_reg_write(0x1C, 0x18, 0x00);        // 频偏设置
    YG350_reg_write(0x23, 0x03, 0x00);        // 自动重传设置
    YG350_reg_write(0x2A, 0xFD, 0xB0);        // AUTO_ACK时间设置
    YG350_reg_write(0x29, 0xB0, 0x00);        // 收发模式设置
    YG350_reg_write(0x28, 0x44, 0x01);        // 地址容错设置
    YG350_reg_write(0x20, 0x4A, 0x00);        // 数据格式配置
    YG350_reg_write(0x09, power >> 8, power); // 功率设置
#if defined(YG350_SET_SPEED_1M)               // 速度设置
    YG350_reg_write(0x2C, 0x01, 0x01);
    YG350_reg_write(0x2D, 0x00, 0x80);
    YG350_reg_write(0x08, 0x6C, 0x90);
#elif defined(YG350_SET_SPEED_250K)
    YG350_reg_write(0x2C, 0x04, 0x01);
    YG350_reg_write(0x2D, 0x05, 0x52);
    YG350_reg_write(0x08, 0x6C, 0x90);
#elif defined(YG350_SET_SPEED_125K)
    YG350_reg_write(0x2C, 0x08, 0x01);
    YG350_reg_write(0x2D, 0x05, 0x52);
    YG350_reg_write(0x08, 0x6C, 0x90);
#else
    YG350_reg_write(0x2C, 0x10, 0x01);
    YG350_reg_write(0x2D, 0x05, 0x52);
    YG350_reg_write(0x08, 0x6C, 0x50);
#endif

    YG350_i2c_start(); // 通过发送来初始化PKT标志位
    YG350_i2c_send_byte(0x32 & 0x7F);
    YG350_i2c_send_byte(0x00);
    YG350_i2c_stop();
    YG350_reg_write(MODE_24G);
    YG350_reg_write(0x24, 0xFF, 0xFF);
    YG350_reg_write(0x27, 0xFF, 0xFF);
    YG350_reg_write(0x07, 0x01, 0x00);

    YG350_reg_write(FIFO_RESET);
    return 1;
}

void YG350_sleep(void)
{
    YG350_reg_write(IDLE);
    YG350_reg_write(SLEEP);
    return;
}

void YG350_send(unsigned char channel, unsigned char *buf, unsigned char length)
{
    unsigned char i;
    YG350_reg_write(IDLE);
    YG350_reg_write(MODE_24G);
    YG350_set_addr(YG350_ADDR);
    YG350_reg_write(FIFO_RESET);
    YG350_i2c_start();
    YG350_i2c_send_byte(0x32 & 0x7F);
    YG350_i2c_send_byte(length);
    for (i = 0; i < length; i++) {
        YG350_i2c_send_byte(buf[i]);
    }
    YG350_i2c_stop();
    YG350_reg_write(0x07, 0x01, channel & 0x7F);
    return;
}

void YG350_recv(unsigned char channel)
{
    YG350_reg_write(IDLE);
    YG350_reg_write(MODE_24G);
    YG350_set_addr(YG350_ADDR);
    YG350_reg_write(0x07, 0x00, channel | 0X80);
    return;
}

void YG350_read(unsigned char *buf, unsigned char length)
{
    unsigned char i;
    YG350_reg_write(IDLE);
    YG350_reg_write(FIFO_RESET);
    YG350_i2c_start();
    YG350_i2c_send_byte(0x32 | 0x80);
    YG350_i2c_recv_byte(1);
    for (i = 0; i < length; i++) {
        buf[i] = YG350_i2c_recv_byte(1);
    }
    YG350_i2c_stop();
    return;
}

// ---------------------------------------------------------------------------------------------------------------------
