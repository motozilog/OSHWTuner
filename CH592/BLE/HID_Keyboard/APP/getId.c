/********************************** (C) COPYRIGHT *******************************
 * File Name          : getId.c
 * Author             : motozilog
 * Version            : V1.0
 * Date               : 2026/07/23
 * Description        : 获取CPU ID、读写FLASH等工具类
 *******************************************************************************/

#include "getId.h"
#include "CONFIG.h"
#include "HAL.h"

#include "CH59x_flash.h"


// DIP开关按键映射表 - 默认配置
typedef struct {
    uint8_t key_pa5;   // PA5按键对应的HID键码
    uint8_t key_pa15;  // PA15按键对应的HID键码
} dip_key_map_t;

// 默认 DIP 映射表 (出厂配置)
static const dip_key_map_t dip_key_map_default[] = {
    {0x50, 0x4F},  // DIP=0: PA5=左箭头(0x50), PA15=右箭头(0x4F)
    {0x52, 0x51},  // DIP=1: PA5=上箭头(0x52), PA15=下箭头(0x51)
    {0x4B, 0x4E},  // DIP=2: PA5=PageUp(0x4B), PA15=PageDown(0x4E)
    {0x2C, 0x28},  // DIP=3: PA5=空格(0x2C), PA15=回车(0x28)
    {0x4F, 0x50},  // DIP=4: PA5=右箭头(0x4F), PA15=左箭头(0x50)
    {0x51, 0x52},  // DIP=5: PA5=下箭头(0x51), PA15=上箭头(0x52)
    {0x4E, 0x4B},  // DIP=6: PA5=PageDown(0x4E), PA15=PageUp(0x4B)
    {0x28, 0x2C}   // DIP=7: PA5=回车(0x28), PA15=空格(0x2C)
};

// 当前使用的 DIP 映射表 (运行时缓存)
static dip_key_map_t dip_key_map[DIP_MAP_DATA_SIZE / 2];

void GetChipUID(char *out_str)
{
    uint8_t uid_buf[8] = {0};
    uint8_t i, high, low;
    GET_UNIQUE_ID(uid_buf);

    for(i = 0; i < 8; i++)
    {
        high = (uid_buf[i] >> 4) & 0x0F;
        low  = uid_buf[i] & 0x0F;
        out_str[i*2]  = (high < 10) ? ('0' + high) : ('A' + high - 10);
        out_str[i*2+1]= (low  < 10) ? ('0' + low)  : ('A' + low  - 10);
    }
    out_str[16] = '\0';
}

/**
 * @brief   读取8421编码开关的值 (PA12=1, PA13=2, PA14=4)
 * @return  DIP值 (0-7)
 */
uint8_t getDIPValue(void)
{
    uint8_t dip_value = 0;
    if(!GPIOA_ReadPortPin(GPIO_Pin_12)) dip_value |= 0x01;
    if(!GPIOA_ReadPortPin(GPIO_Pin_13)) dip_value |= 0x02;
    if(!GPIOA_ReadPortPin(GPIO_Pin_14)) dip_value |= 0x04;
    return dip_value;
}

/**
 * @brief   计算 DIP 映射表的校验和 (字节累加)
 * @param   map     映射表指针 (16字节)
 * @return  校验和
 */
static uint8_t dip_map_checksum(const uint8_t *map)
{
    uint8_t i;
    uint8_t sum = 0;
    for (i = 0; i < DIP_MAP_DATA_SIZE; i++) {
        sum += map[i];
    }
    return sum;
}

/**
 * @brief   从 Flash 读取 DIP 映射表 (17字节: 16字节数据 + 1字节校验和)
 * @param   buf     输出缓冲区 (17字节)
 * @return  0-成功, 1-失败
 */
static uint8_t dip_map_read_from_flash(uint8_t *buf)
{
    if (buf == NULL) return 1;
    EEPROM_READ(DIP_MAP_FLASH_ADDR, buf, DIP_MAP_STORE_SIZE);
    return 0;
}

/**
 * @brief   验证 DIP 映射表数据的有效性 (校验和检查)
 * @param   buf     数据缓冲区 (17字节)
 * @return  1-有效, 0-无效
 */
static uint8_t dip_map_validate(const uint8_t *buf)
{
    if (buf == NULL) return 0;
    
    // 计算前16字节的校验和
    uint8_t calc_sum = dip_map_checksum(buf);
    
    // 与第17字节比较
    if (calc_sum != buf[16]) {
        return 0;
    }
    
    // 检查第17字节是否为0xFF (未写入状态)
    if (buf[16] == 0xFF) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief   加载 DIP 映射表到运行时缓存
 * @param   buf     数据缓冲区 (16字节数据)
 */
static void dip_map_load_to_cache(const uint8_t *buf)
{
    if (buf == NULL) return;
    memcpy(dip_key_map, buf, DIP_MAP_DATA_SIZE);
}

/**
 * @brief   初始化 DIP 映射表 (从 Flash 加载，失败则使用默认配置)
 */
void dip_map_init(void)
{
    uint8_t i;
    uint8_t read_buf[DIP_MAP_STORE_SIZE];  // 17字节
    
    // 从 Flash 读取 17 字节
    dip_map_read_from_flash(read_buf);
    
    // 打印读取到的数据
    PRINT("Flash DIP Map (17 bytes): ");
    for (i = 0; i < DIP_MAP_STORE_SIZE; i++) {
        PRINT("%02X ", read_buf[i]);
    }
    PRINT("\n");
    
    // 验证数据有效性
    if (dip_map_validate(read_buf)) {
        PRINT("DIP map loaded from Flash (checksum OK)\n");
        dip_map_load_to_cache(read_buf);
    } else {
        PRINT("DIP map invalid, loading default...\n");
        // 加载默认配置到缓存
        dip_map_load_to_cache((uint8_t*)dip_key_map_default);
        // 将默认配置写入 Flash (包含校验和)
        SaveDipMapToFlash((uint8_t*)dip_key_map);
    }
}

/**
 * @brief   根据DIP值获取对应的按键映射 (从运行时缓存读取)
 * @param   dip_value   DIP值 (0-7)
 * @param   key_pa5     输出: PA5按键对应的HID键码
 * @param   key_pa15    输出: PA15按键对应的HID键码
 */
void getDIPKeyMap(uint8_t dip_value, uint8_t *key_pa5, uint8_t *key_pa15)
{
    if (dip_value > 7) {
        dip_value = 0;
    }
    if (key_pa5) {
        *key_pa5 = dip_key_map[dip_value].key_pa5;
    }
    if (key_pa15) {
        *key_pa15 = dip_key_map[dip_value].key_pa15;
    }
}

/**
 * @brief   从 Flash 读取 DIP 映射表 (16字节数据)
 * @param   buf     输出缓冲区 (16字节)
 * @return  0-成功, 1-失败
 */
uint8_t ReadDipMapFromFlash(uint8_t *buf)
{
    uint8_t read_buf[DIP_MAP_STORE_SIZE];
    
    if (buf == NULL) return 1;
    
    dip_map_read_from_flash(read_buf);
    
    // 验证校验和
    if (!dip_map_validate(read_buf)) {
        return 1;  // 校验和错误
    }
    
    memcpy(buf, read_buf, DIP_MAP_DATA_SIZE);
    return 0;
}

/**
 * @brief   保存 DIP 映射表到 Flash (16字节数据 + 1字节校验和)
 *          写入成功后立即校验，校验失败则加载默认值并重新写入
 * @param   buf     数据缓冲区 (16字节)
 * @return  0-成功, 1-失败
 */
uint8_t SaveDipMapToFlash(uint8_t *buf)
{
    uint8_t store_buf[DIP_MAP_STORE_SIZE];
    uint8_t verify_buf[DIP_MAP_STORE_SIZE];
    uint8_t ret;
    uint8_t i;
    
    if (buf == NULL) return 1;
    
    // 计算校验和
    uint8_t sum = dip_map_checksum(buf);
    
    // 组装 17 字节数据
    memcpy(store_buf, buf, DIP_MAP_DATA_SIZE);
    store_buf[DIP_MAP_DATA_SIZE] = sum;  // 第17字节 = 校验和
    
    PRINT("Saving to Flash: ");
    for (i = 0; i < DIP_MAP_STORE_SIZE; i++) {
        PRINT("%02X ", store_buf[i]);
    }
    PRINT("\n");
    
    // 擦除 4KB 块
    ret = EEPROM_ERASE(DIP_MAP_FLASH_ADDR, EEPROM_BLOCK_SIZE);
    if (ret != 0) {
        PRINT(">>> Flash ERASE failed! (ret=%d)\n", ret);
        goto load_default;
    }
    
    // 写入 17 字节
    ret = EEPROM_WRITE(DIP_MAP_FLASH_ADDR, store_buf, DIP_MAP_STORE_SIZE);
    if (ret != 0) {
        PRINT(">>> Flash WRITE failed! (ret=%d)\n", ret);
        goto load_default;
    }
    
    // ========== 写入成功后立即校验 ==========
    PRINT("Verifying Flash write...\n");
    
    // 从 Flash 读取刚写入的数据
    dip_map_read_from_flash(verify_buf);
    
    // 打印读取到的数据
    PRINT("Read back from Flash: ");
    for (i = 0; i < DIP_MAP_STORE_SIZE; i++) {
        PRINT("%02X ", verify_buf[i]);
    }
    PRINT("\n");
    
    // 逐字节对比
    for (i = 0; i < DIP_MAP_STORE_SIZE; i++) {
        if (store_buf[i] != verify_buf[i]) {
            PRINT(">>> Flash VERIFY FAILED at byte %d: wrote 0x%02X, read 0x%02X\n", 
                  i, store_buf[i], verify_buf[i]);
            goto load_default;
        }
    }
    
    // 验证校验和
    if (!dip_map_validate(verify_buf)) {
        PRINT(">>> Flash VERIFY FAILED: checksum mismatch!\n");
        goto load_default;
    }
    
    // ========== 校验成功：加载到运行时缓存 ==========
    PRINT(">>> Flash write VERIFIED OK!\n");
    dip_map_load_to_cache(verify_buf);
    PRINT("DIP map loaded to cache from Flash\n");
    return 0;

load_default:
    // ========== 校验失败：加载默认值并写入 Flash ==========
    PRINT(">>> Flash verify FAILED! Loading default DIP map...\n");
    
    // 加载默认配置到缓存
    dip_map_load_to_cache((uint8_t*)dip_key_map_default);
    PRINT("Default DIP map loaded to cache\n");
    
    // 重新计算默认配置的校验和
    uint8_t default_sum = dip_map_checksum((uint8_t*)dip_key_map_default);
    uint8_t default_store[DIP_MAP_STORE_SIZE];
    memcpy(default_store, dip_key_map_default, DIP_MAP_DATA_SIZE);
    default_store[DIP_MAP_DATA_SIZE] = default_sum;
    
    PRINT("Writing default DIP map to Flash: ");
    for (i = 0; i < DIP_MAP_STORE_SIZE; i++) {
        PRINT("%02X ", default_store[i]);
    }
    PRINT("\n");
    
    // 擦除 4KB 块
    ret = EEPROM_ERASE(DIP_MAP_FLASH_ADDR, EEPROM_BLOCK_SIZE);
    if (ret != 0) {
        PRINT(">>> Failed to write default map (ERASE failed)!\n");
        return 1;
    }
    
    // 写入默认配置
    ret = EEPROM_WRITE(DIP_MAP_FLASH_ADDR, default_store, DIP_MAP_STORE_SIZE);
    if (ret != 0) {
        PRINT(">>> Failed to write default map (WRITE failed)!\n");
        return 1;
    }
    
    PRINT(">>> Default DIP map written to Flash\n");
    return 1;  // 返回失败，但已恢复默认配置
}