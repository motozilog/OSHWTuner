/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH & motozilog
 * Version            : V1.0
 * Date               : 2020/08/06
 * Description        : 蓝牙键盘应用主函数及任务系统初始化
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "CONFIG.h"
#include "HAL.h"
#include "hiddev.h"
#include "hidkbd.h"
// motozilog start
#include "getId.h"
#include "usb_hid.h"
#include "hidkbd_usb.h"
#include "CH59x_common.h"
// motozilog end

/*********************************************************************
 * GLOBAL TYPEDEFS
 */

// motozilog start

// motozilog end

__attribute__ ((aligned (4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if (defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x02};
#endif

// 引用 usb_hid.c 中的变量
extern uint8_t usbConnected;

/*********************************************************************
 * @fn      Main_Circulation
 *
 * @brief   主循环
 *
 * @return  none
 */
__HIGH_CODE
__attribute__ ((noinline)) void Main_Circulation() {
    while (1) {
        TMOS_SystemProcess();
    }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   主函数
 *
 * @return  none
 */
int main (void) {
    uint8_t mode;
    uint8_t transport;

#if (defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg (ENABLE);
#endif
    SetSysClock (CLK_SOURCE_PLL_60MHz);
#if (defined(HAL_SLEEP)) && (HAL_SLEEP == TRUE)
    GPIOA_ModeCfg (GPIO_Pin_All, GPIO_ModeIN_PU);
    GPIOB_ModeCfg (GPIO_Pin_All, GPIO_ModeIN_PU);
#endif
#ifdef DEBUG
    GPIOA_SetBits (bTXD1);
    GPIOA_ModeCfg (bTXD1, GPIO_ModeOut_PP_5mA);
    UART1_DefInit();
#endif
    //  PRINT ("WCH VER_LIB:%s\n", VER_LIB);
    PRINT ("OSHW Tuner By motozilog V2.2 (Built: %s %s)\n", __DATE__, __TIME__);

    // 初始化 DIP 映射表 (从 Flash 加载)
    dip_map_init();

    // 获取UID:START
    char uid_str[17] = {0};
    GetChipUID (uid_str);
    PRINT ("CH592 Unique ID: %s\n", uid_str);
    // 获取UID:END

    // ========== USB 初始化 ==========
    SetSysClock (CLK_SOURCE_PLL_60MHz);
    UsbHid_Init();

    // 等待 USB 枚举完成 (约 5s)
    DelayMs (5000);

    // 检查 USB 是否配置成功
    if (UsbHid_IsConfigured()) {
        PRINT ("USB HID Keyboard Mode\n");

        // USB 模式下配置按键引脚 (只配置 PA5 和 PA15)
        GPIOA_ModeCfg (GPIO_Pin_5 | GPIO_Pin_15, GPIO_ModeIN_PU);
        // 配置 DIP 开关引脚
        GPIOA_ModeCfg (GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeIN_PU);

        HidKbd_MainInit();

        // 简单主循环（不依赖 App_Run）
        while (1) {
            static uint32_t loop_count = 0;
            static uint8_t was_configured = 0;
            HidKbd_MainProcess();

            // ===== 每5秒打印一次 USB 寄存器状态 =====
            loop_count++;
            if (loop_count >= 500) {  // 10ms * 500 = 5000ms = 5秒
                loop_count = 0;
                uint8_t dev_addr = R8_USB_DEV_AD & MASK_USB_ADDR;
                PRINT ("USB: addr=0x%02X, configured=%d, connected=%d\n",
                       dev_addr, UsbHid_IsConfigured(), UsbHid_GetConnected());
            }

            // ===== 检测 USB 断开 =====
            // 记录曾经配置成功过
            if (UsbHid_IsConfigured()) {
                was_configured = 1;
            }

            // 如果曾经配置成功过，但现在已断开（configured=0 且 connected=0）
            if (was_configured && !UsbHid_IsConfigured() && !UsbHid_GetConnected()) {
                PRINT (">>> USB DISCONNECTED! Rebooting to BLE mode...\n");
                DelayMs (100);
                SYS_ResetExecute();
            }


            DelayMs (10);  // 简单的延时
        }
    } else {
        PRINT ("USB not configured, fallback to BLE mode\n");

        // 彻底关闭 USB 控制器，防止 Windows 识别到设备
        // 关闭 USB 上拉，让 Windows 检测不到设备
        R8_USB_CTRL &= ~RB_UC_DEV_PU_EN;                           // 关闭 D+ 上拉
        R16_PIN_ANALOG_IE &= ~(RB_PIN_USB_IE | RB_PIN_USB_DP_PU);  // 关闭 USB 引脚
        R8_UDEV_CTRL &= ~RB_UD_PORT_EN;                            // 关闭 USB 端口

        // 禁用 USB 中断
        PFIC_DisableIRQ (USB_IRQn);


        // 标准蓝牙初始化
        CH59x_BLEInit();
        HAL_Init();
        GAPRole_PeripheralInit();
        HidDev_Init();

        // motozilog start: GPIO初始化（放在HAL_Init之后，确保不被覆盖）
        //  PA5, PA15 为上拉输入（按键）
        GPIOA_ModeCfg (GPIO_Pin_5 | GPIO_Pin_15, GPIO_ModeIN_PU);
        // PA12, PA13, PA14 为上拉输入（8421编码开关）
        GPIOA_ModeCfg (GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeIN_PU);
        // PB7 为推挽输出（低电量指示）
        GPIOB_ModeCfg (GPIO_Pin_7, GPIO_ModeOut_PP_5mA);
        GPIOB_ResetBits (GPIO_Pin_7);  // 初始低电平
        // PB4 为推挽输出（蓝牙指示灯）
        GPIOB_ModeCfg (GPIO_Pin_4, GPIO_ModeOut_PP_5mA);
        GPIOB_ResetBits (GPIO_Pin_4);  // 初始低电平
        // motozilog end

        HidEmu_Init();
        Main_Circulation();
    }
}

/******************************** endfile @ main ******************************/