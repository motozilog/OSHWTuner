#ifndef __GETID_H
#define __GETID_H

#include <stdint.h>

#define DIP_MAP_FLASH_ADDR  0x0000  // DataFlash 起始地址
#define DIP_MAP_DATA_SIZE   16
#define DIP_MAP_STORE_SIZE  17

/**
 * @brief 读取芯片8字节唯一ID，输出16位十六进制字符串(长度17含结束符)
 * @param out_str 输出缓冲区，char[17]
 * @retval 无
 */
void GetChipUID(char *out_str);

/**
 * @brief 读取8421编码开关的值 (PA12=1, PA13=2, PA14=4)
 * @return DIP值 (0-7)
 */
uint8_t getDIPValue(void);

/**
 * @brief 根据DIP值获取对应的按键映射
 * @param dip_value DIP值 (0-7)
 * @param key_pa5 输出: PA5按键对应的HID键码
 * @param key_pa15 输出: PA15按键对应的HID键码
 */
void getDIPKeyMap(uint8_t dip_value, uint8_t *key_pa5, uint8_t *key_pa15);

/**
 * @brief 初始化 DIP 映射表 (从 Flash 加载)
 */
void dip_map_init(void);

/**
 * @brief 从 Flash 读取 DIP 映射表 (16字节数据 + 1字节校验和)
 * @param buf 输出缓冲区 (16字节)
 * @return 0-成功, 1-失败
 */
uint8_t ReadDipMapFromFlash(uint8_t *buf);

/**
 * @brief 保存 DIP 映射表到 Flash (16字节数据 + 1字节校验和)
 * @param buf 数据缓冲区 (16字节)
 * @return 0-成功, 1-失败
 */
uint8_t SaveDipMapToFlash(uint8_t *buf);
#endif