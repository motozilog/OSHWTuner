/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_hid.h
 * Author             : Codex
 * Version            : V1.0
 * Date               : 2026/04/14
 * Description        : USB HID 设备模式接口
 *******************************************************************************/

#ifndef USB_HID_H
#define USB_HID_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "app_board.h"
#include "CONFIG.h"

void UsbHid_Init(void);
uint8_t UsbHid_IsConfigured(void);
void UsbHid_SendKeyboardReport(uint8_t modifier, uint8_t reserved, 
                                uint8_t key1, uint8_t key2, uint8_t key3,
                                uint8_t key4, uint8_t key5, uint8_t key6);

uint8_t UsbHid_GetConnected(void);

#ifdef __cplusplus
}
#endif

#endif