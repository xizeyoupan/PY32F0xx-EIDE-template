// Information ---------------------------------------------------------------------------------------------------------

/// @brief          YG350头文件
/// @details
/// @version        v1.0.0
/// @author         HongJunjie
/// @date           2024/11/06

// ---------------------------------------------------------------------------------------------------------------------

#ifndef YG350_H
#define YG350_H

// ---------------------------------------------------------------------------------------------------------------------

#define YG350_Power_Level_01  0X3FC0
#define YG350_Power_Level_02  0X3FB0
#define YG350_Power_Level_03  0X3F30
#define YG350_Power_Level_04  0X7F30
#define YG350_Power_Level_05  0X7E30
#define YG350_Power_Level_06  0X7D30
#define YG350_Power_Level_07  0X7C30
#define YG350_Power_Level_08  0X7B30
#define YG350_Power_Level_09  0X7A30
#define YG350_Power_Level_10  0X7930
#define YG350_Power_Level_11  0x7830

#define YG350_Power_Level_Min YG350_Power_Level_01
#define YG350_Power_Level_Max YG350_Power_Level_11

// ---------------------------------------------------------------------------------------------------------------------

#define YG350_Channel_00  5
#define YG350_Channel_01  10
#define YG350_Channel_02  15
#define YG350_Channel_03  20
#define YG350_Channel_04  25
#define YG350_Channel_05  30
#define YG350_Channel_06  35
#define YG350_Channel_07  40
#define YG350_Channel_08  45
#define YG350_Channel_09  50
#define YG350_Channel_10  55
#define YG350_Channel_11  60
#define YG350_Channel_12  65
#define YG350_Channel_13  70
#define YG350_Channel_14  75
#define YG350_Channel_15  82

#define SEND_CHANNEL_1    YG350_Channel_01
#define SEND_CHANNEL_2    YG350_Channel_07
#define SEND_CHANNEL_3    YG350_Channel_13

#define YG350_Channel_G01 YG350_Channel_01
#define YG350_Channel_G02 YG350_Channel_02
#define YG350_Channel_G03 YG350_Channel_03
#define YG350_Channel_G04 YG350_Channel_04
#define YG350_Channel_G05 YG350_Channel_05
#define YG350_Channel_G06 YG350_Channel_06
#define YG350_Channel_G07 YG350_Channel_07
#define YG350_Channel_G08 YG350_Channel_08
#define YG350_Channel_G09 YG350_Channel_09
#define YG350_Channel_G10 YG350_Channel_10
#define YG350_Channel_G11 YG350_Channel_11
#define YG350_Channel_G12 YG350_Channel_12
#define YG350_Channel_G13 YG350_Channel_13
#define YG350_Channel_G14 YG350_Channel_15

// ---------------------------------------------------------------------------------------------------------------------

extern unsigned char YG350_reg_h; // YG350读寄存器值高8位
extern unsigned char YG350_reg_l; // YG350读寄存器低高8位

// ---------------------------------------------------------------------------------------------------------------------

void YG350_i2c_start(void);
void YG350_i2c_stop(void);
unsigned char YG350_i2c_send_byte(unsigned char send_data);
unsigned char YG350_i2c_recv_byte(unsigned char ack);

void YG350_reg_write(unsigned char addr, unsigned char data_h, unsigned char data_l);
void YG350_reg_read(unsigned char addr);

unsigned char YG350_init(unsigned int power);
void YG350_sleep(void);

void YG350_send(unsigned char channel, unsigned char *buf, unsigned char length);
void YG350_recv(unsigned char channel);
void YG350_read(unsigned char *buf, unsigned char length);

#define YG350_ready() (YG350_reg_read(0x03), (YG350_reg_h & 0x20))

// ---------------------------------------------------------------------------------------------------------------------

#define TRANS_RECV_CHANNEL     3
#define TRANS_RECV_OVERTIME    400
#define TRANS_RECV_OVERTIME_MS 40
#define trans_recv_result()    YG350_ready()

void delay_5us();
void delay_1us();

#endif
