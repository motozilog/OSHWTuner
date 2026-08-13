/********************************** (C) COPYRIGHT *******************************
 * File Name          : hidkbd_usb.c
 * Author             : bibilala & trbbadboy & motozilog
 * Version            : V1.0
 * Date               : 2026/07/23
 * Description        : USB 键盘模式处理
 *******************************************************************************/

#include <string.h>
#include "CONFIG.h"
#include "usb_hid.h"
#include "getId.h"

// 由于没有 app_common.h，使用 PRINT 宏替代 BPRINT
#ifdef DEBUG
#define BPRINT(...) PRINT(__VA_ARGS__)
#else
#define BPRINT(...)
#endif


// APP_KEY_* 宏定义在 app_board.h 中，但 CONFIG.h 没有包含
// 需要从 CH582 的 app_board.h 中复制过来
#ifndef APP_KEY_A
#define APP_KEY_A        0x0010
#define APP_KEY_B        0x0020
#define APP_KEY_SELECT   0x0040
#define APP_KEY_START    0x0080
#define APP_KEY_UP       0x0002
#define APP_KEY_DOWN     0x0008
#define APP_KEY_LEFT     0x0001
#define APP_KEY_RIGHT    0x0004
#endif

/* ============================================================
   键盘报告结构定义
   ============================================================ */
typedef struct {
    uint8_t modifier;   // 修饰键
    uint8_t reserved;   // 保留
    uint8_t key[6];     // 6个键码
} kbd_report_t;

/* ============================================================
   状态变量
   ============================================================ */
static kbd_report_t current_kbd_report = {0, 0, {0, 0, 0, 0, 0, 0}};
static kbd_report_t last_kbd_report = {0, 0, {0, 0, 0, 0, 0, 0}};

/* ============================================================
   键码映射表 (USB HID Usage ID)
   ============================================================ */
#define KEY_A           0x04
#define KEY_B           0x05
#define KEY_ENTER       0x28
#define KEY_ESC         0x29
#define KEY_UP          0x52
#define KEY_DOWN        0x51
#define KEY_LEFT        0x50
#define KEY_RIGHT       0x4F

/* ============================================================
   私有函数
   ============================================================ */
static void hidKbd_AddKeyToReport(kbd_report_t *report, uint8_t usage);
static void hidKbd_SendReport(void);

/* ============================================================
   添加键码到报告 (去重)
   ============================================================ */
static void hidKbd_AddKeyToReport(kbd_report_t *report, uint8_t usage)
{
    uint8_t i;
    
    if (usage == 0) return;
    
    for (i = 0; i < 6; i++) {
        if (report->key[i] == usage)
            return;
    }
    
    for (i = 0; i < 6; i++) {
        if (report->key[i] == 0) {
            report->key[i] = usage;
            return;
        }
    }
}
/**
 * @brief   读取按键状态 (只读取 PA5 和 PA15)
 * @return  按键位掩码
 */
static uint16_t App_KeyRead(void)
{
    uint32_t gpio_keys;
    uint16_t keys = 0;

    // 只读取 PA5 和 PA15 (上拉输入，按下为低电平)
    gpio_keys = (~GPIOA_ReadPortPin(GPIO_Pin_5 | GPIO_Pin_15)) & 
                (GPIO_Pin_5 | GPIO_Pin_15);
    
    if (gpio_keys & GPIO_Pin_5)
        keys |= APP_KEY_A;
    if (gpio_keys & GPIO_Pin_15)
        keys |= APP_KEY_B;

    return keys;
}

/* ============================================================
   发送键盘报告
   ============================================================ */
static void hidKbd_SendReport(void)
{
    if (memcmp(&current_kbd_report, &last_kbd_report, sizeof(kbd_report_t)) == 0) {
        return;
    }
    
    memcpy(&last_kbd_report, &current_kbd_report, sizeof(kbd_report_t));
    
    UsbHid_SendKeyboardReport(
        current_kbd_report.modifier,
        current_kbd_report.reserved,
        current_kbd_report.key[0],
        current_kbd_report.key[1],
        current_kbd_report.key[2],
        current_kbd_report.key[3],
        current_kbd_report.key[4],
        current_kbd_report.key[5]
    );
    
    BPRINT("KBD: Mod=0x%02X Keys=%02X %02X %02X %02X %02X %02X\n",
           current_kbd_report.modifier,
           current_kbd_report.key[0], current_kbd_report.key[1],
           current_kbd_report.key[2], current_kbd_report.key[3],
           current_kbd_report.key[4], current_kbd_report.key[5]);
}

/* ============================================================
   初始化函数
   ============================================================ */
void HidKbd_MainInit(void)
{
    memset(&current_kbd_report, 0, sizeof(kbd_report_t));
    memset(&last_kbd_report, 0, sizeof(kbd_report_t));
}

/* ============================================================
   主循环处理函数
   ============================================================ */
void HidKbd_MainProcess(void)
{
    // 读取 DIP 开关值
    uint8_t dip_value = getDIPValue();
    uint8_t key_pa5, key_pa15;
    getDIPKeyMap(dip_value, &key_pa5, &key_pa15);
    
    // 读取按键状态
    uint16_t keys = App_KeyRead();
    
    // 清空当前报告
    memset(&current_kbd_report, 0, sizeof(kbd_report_t));
    
    // PA5 按键 -> 根据 DIP 映射
    if (keys & APP_KEY_A) {
        hidKbd_AddKeyToReport(&current_kbd_report, key_pa5);
    }
    
    // PA15 按键 -> 根据 DIP 映射
    if (keys & APP_KEY_B) {
        hidKbd_AddKeyToReport(&current_kbd_report, key_pa15);
    }
    
    // 发送报告
    hidKbd_SendReport();
}