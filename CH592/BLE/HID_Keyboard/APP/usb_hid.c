/********************************** (C) COPYRIGHT *******************************
 * File Name          : usb_hid.c
 * Author             : bibilala & trbbadboy & motozilog
 * Version            : V1.0
 * Date               : 2026/08/04
 * Description        : USB HID 设备模式实现 - 支持 Feature Report 传输
 *******************************************************************************/

#include <string.h>

#include "CH59x_common.h"
#include "CH59x_usbdev.h"
#include "CONFIG.h"
#include "usb_hid.h"
#include "getId.h"

// 直接定义 USB RAM 指针
__attribute__ ((weak)) uint8_t *pEP0_RAM_Addr;
__attribute__ ((weak)) uint8_t *pEP1_RAM_Addr;
__attribute__ ((weak)) uint8_t *pEP2_RAM_Addr;
__attribute__ ((weak)) uint8_t *pEP3_RAM_Addr;

#define INT_TO_HEX(i) ((i & 0x0f) < 10 ? '0' + (i & 0x0f) : 'A' + ((i & 0x0f) - 10))

#define USB_HID_EP0_SIZE 64U
#define USB_HID_EP1_SIZE 9U
#define USB_HID_BCD_DEVICE 0x0100U

static __attribute__ ((aligned (4))) uint8_t usbHidEP0Ram[192];
static __attribute__ ((aligned (4))) uint8_t usbHidEP1Ram[128];
static __attribute__ ((aligned (4))) uint8_t usbHidEP2Ram[128];
static __attribute__ ((aligned (4))) uint8_t usbHidEP3Ram[128];
static uint8_t usbHidDeviceDesc[18];
static uint8_t usbHidStringDesc[64];

static uint8_t usbHidConfigured;
static uint8_t usbHidSetupReqCode;
static uint8_t usbHidIdleRate;
static uint8_t usbHidProtocol;
static volatile uint8_t usbHidEp1Busy;
static volatile uint8_t usbHidPendingLen;
static uint16_t usbHidSetupReqLen;
static uint8_t usbHidPendingReport[USB_HID_EP1_SIZE];
static const uint8_t *usbHidDescrPtr;

// 新增：用于跟踪 SET_REPORT 数据等待状态
static uint8_t waitForSetReportData = 0;
static uint8_t setReportReportType = 0;
static uint8_t setReportReportId = 0;

// 在文件开头的静态变量区域添加
uint8_t usbConnected = 0;
static uint32_t usbDisconnectCheckCount = 0;

static const uint8_t usbHidLangDesc[] = {
    0x04, USB_DESCR_TYP_STRING, 0x09, 0x04};

static uint8_t UsbHid_BuildStringDesc (const char *ascii, uint8_t *buf, uint8_t bufSize) {
    uint8_t index = 2;
    if ((ascii == NULL) || (buf == NULL) || (bufSize < 2U)) {
        return 0;
    }
    while ((*ascii != '\0') && (index <= (uint8_t)(bufSize - 2U))) {
        buf[index++] = (uint8_t)(*ascii++);
        buf[index++] = 0x00;
    }
    buf[0] = index;
    buf[1] = USB_DESCR_TYP_STRING;
    return index;
}

static const uint8_t usbHidKeyboardReportDesc[] = {
    // 键盘输入报告 - Report ID = 2
    0x05, 0x01, 0x09, 0x06, 0xA1, 0x01,
    0x85, 0x02,
    0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01,
    0x75, 0x01, 0x95, 0x08, 0x81, 0x02,
    0x95, 0x01, 0x75, 0x08, 0x81, 0x03,
    0x95, 0x06, 0x75, 0x08, 0x15, 0x00, 0x25, 0x65,
    0x05, 0x07, 0x19, 0x00, 0x29, 0x65, 0x81, 0x00,
    0xC0,
    // Feature Report - Report ID = 1 (17字节数据)
    0x06, 0x00, 0xFF, 0x09, 0x01, 0xA1, 0x01,
    0x85, 0x01,
    0x09, 0x02, 0x15, 0x00, 0x26, 0xFF, 0x00,
    0x75, 0x08, 0x95, 0x11,  // 17 字节
    0xB1, 0x02,
    0xC0
};

static const uint8_t usbHidKeyboardConfigDesc[] = {
    0x09, USB_DESCR_TYP_CONFIG, 0x22, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32,
    0x09, USB_DESCR_TYP_INTERF, 0x00, 0x00, 0x01, USB_DEV_CLASS_HID, 0x01, 0x01, 0x00,
    0x09, USB_DESCR_TYP_HID, 0x11, 0x01, 0x00, 0x01, USB_DESCR_TYP_REPORT,
    (uint8_t)(sizeof (usbHidKeyboardReportDesc) & 0xFF),
    (uint8_t)(sizeof (usbHidKeyboardReportDesc) >> 8),
    0x07, USB_DESCR_TYP_ENDP, 0x81, USB_ENDP_TYPE_INTER, USB_HID_EP1_SIZE, 0x00, 0x01};

static void UsbHid_BuildDeviceDesc (uint8_t *buf) {
    buf[0] = 0x12; buf[1] = USB_DESCR_TYP_DEVICE;
    buf[2] = 0x10; buf[3] = 0x01;
    buf[4] = 0x00; buf[5] = 0x00; buf[6] = 0x00;
    buf[7] = USB_HID_EP0_SIZE;
    buf[8] = (uint8_t)(APP_USB_VENDOR_ID & 0xFFU);
    buf[9] = (uint8_t)(APP_USB_VENDOR_ID >> 8);
    buf[10] = (uint8_t)(APP_USB_PRODUCT_ID & 0xFFU);
    buf[11] = (uint8_t)(APP_USB_PRODUCT_ID >> 8);
    buf[12] = (uint8_t)(USB_HID_BCD_DEVICE & 0xFF);
    buf[13] = (uint8_t)(USB_HID_BCD_DEVICE >> 8);
    buf[14] = 0x01; buf[15] = 0x02; buf[16] = 0x03; buf[17] = 0x01;
}

static const uint8_t *UsbHid_GetConfigDesc (void) {
    return usbHidKeyboardConfigDesc;
}

static const uint8_t *UsbHid_GetReportDesc (void) {
    return usbHidKeyboardReportDesc;
}

static uint16_t UsbHid_GetReportDescLen (void) {
    return sizeof (usbHidKeyboardReportDesc);
}

static uint16_t UsbHid_GetConfigDescLen (void) {
    return 34U;
}

static const uint8_t *UsbHid_GetHidDesc (uint8_t interfaceNum) {
    if (interfaceNum != 0U) {
        return NULL;
    }
    return UsbHid_GetConfigDesc() + 18;
}

static void UsbHid_TrySendPending (void) {
    if ((!usbHidConfigured) || usbHidEp1Busy || (usbHidPendingLen == 0U)) {
        return;
    }
    memcpy (pEP1_IN_DataBuf, usbHidPendingReport, usbHidPendingLen);
    DevEP1_IN_Deal (usbHidPendingLen);
    usbHidEp1Busy = TRUE;
    usbHidPendingLen = 0;
}

static void UsbHid_ResetState (void) {
    usbHidConfigured = FALSE;
    usbHidSetupReqCode = 0;
    usbHidIdleRate = 0;
    usbHidProtocol = 1;
    usbHidEp1Busy = FALSE;
    usbHidPendingLen = 0;
    usbHidSetupReqLen = 0;
    usbHidDescrPtr = NULL;
    waitForSetReportData = 0;
    setReportReportType = 0;
    setReportReportId = 0;
    R8_USB_DEV_AD = 0x00;
    R8_UEP0_T_LEN = 0;
    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    R8_UEP1_T_LEN = 0;
    R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
}

void UsbHid_Init (void) {
    pEP0_RAM_Addr = usbHidEP0Ram;
    pEP1_RAM_Addr = usbHidEP1Ram;
    pEP2_RAM_Addr = usbHidEP2Ram;
    pEP3_RAM_Addr = usbHidEP3Ram;
    USB_DeviceInit();
    UsbHid_ResetState();
    PFIC_ClearPendingIRQ (USB_IRQn);
    PFIC_EnableIRQ (USB_IRQn);
}

uint8_t UsbHid_IsConfigured (void) {
    return usbHidConfigured;
}

void UsbHid_SendKeyboardReport (uint8_t modifier, uint8_t reserved,
                                uint8_t key1, uint8_t key2, uint8_t key3,
                                uint8_t key4, uint8_t key5, uint8_t key6) {
    uint8_t report[9];
    report[0] = 0x02;
    report[1] = modifier;
    report[2] = reserved;
    report[3] = key1;
    report[4] = key2;
    report[5] = key3;
    report[6] = key4;
    report[7] = key5;
    report[8] = key6;
    if (!usbHidConfigured) {
        return;
    }
    if (usbHidEp1Busy) {
        memcpy (usbHidPendingReport, report, sizeof (report));
        usbHidPendingLen = sizeof (report);
        return;
    }
    memcpy (pEP1_IN_DataBuf, report, sizeof (report));
    DevEP1_IN_Deal (sizeof (report));
    usbHidEp1Busy = TRUE;
}

void USB_DevTransProcess (void) {
    uint8_t len;
    uint8_t intFlag;
    uint8_t errFlag = 0;
    uint8_t reqType;
    uint8_t i;
    static uint8_t last_setup_req_code = 0;

static uint32_t int_counter = 0;
    static uint8_t last_dev_addr = 0xFF;

    intFlag = R8_USB_INT_FG;
    
    // ===== 每次进入都打印中断标志和设备地址 =====
    int_counter++;
    if (1==1) {  // 每100次打印一次，避免刷屏
        uint8_t dev_addr = R8_USB_DEV_AD & MASK_USB_ADDR;
        PRINT("INT: flag=0x%02X, addr=0x%02X, configured=%d\n", 
              intFlag, dev_addr, usbHidConfigured);
    }


    intFlag = R8_USB_INT_FG;
    if (intFlag & RB_UIF_TRANSFER) {
        if ((R8_USB_INT_ST & MASK_UIS_TOKEN) != MASK_UIS_TOKEN) {
            switch (R8_USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP)) {
            case UIS_TOKEN_IN:
                switch (usbHidSetupReqCode) {
                case USB_GET_DESCRIPTOR:
                    len = (usbHidSetupReqLen >= USB_HID_EP0_SIZE) ? USB_HID_EP0_SIZE : (uint8_t)usbHidSetupReqLen;
                    if ((len != 0U) && (usbHidDescrPtr != NULL)) {
                        memcpy (pEP0_DataBuf, usbHidDescrPtr, len);
                        usbHidDescrPtr += len;
                    }
                    usbHidSetupReqLen -= len;
                    R8_UEP0_T_LEN = len;
                    R8_UEP0_CTRL ^= RB_UEP_T_TOG;
                    break;
                case USB_SET_ADDRESS:
                    R8_USB_DEV_AD = (R8_USB_DEV_AD & RB_UDA_GP_BIT) | (uint8_t)(usbHidSetupReqLen & MASK_USB_ADDR);
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                default:
                    R8_UEP0_T_LEN = 0;
                    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                    break;
                }
                break;

            case UIS_TOKEN_OUT:
                // 打印调试信息
                PRINT(">>> UIS_TOKEN_OUT: setup_code=0x%02X, last_code=0x%02X, RX_LEN=%d\n", 
                      usbHidSetupReqCode, last_setup_req_code, R8_USB_RX_LEN);
                
                // 打印 OUT 数据内容
                if (R8_USB_RX_LEN > 0) {
                    PRINT("OUT Data (%d bytes): ", R8_USB_RX_LEN);
                    for (i = 0; i < R8_USB_RX_LEN && i < 64; i++) {
                        PRINT("%02X ", pEP0_DataBuf[i]);
                    }
                    PRINT("\n");
                }
                
                // ===== 处理 SET_REPORT 的 Data 阶段 =====
                // 使用独立的标志变量，不受 setup_code 变化影响
                if (waitForSetReportData) {
                    PRINT("Processing SET_REPORT data: type=%d, id=%d\n", 
                          setReportReportType, setReportReportId);
                    
                    if (setReportReportType == 0x03 && setReportReportId == 0x01) {
                        uint8_t dataLen = R8_USB_RX_LEN;
                        
                        PRINT("\n========================================\n");
                        PRINT("HID_SET_REPORT DATA: len=%d\n", dataLen);
                        PRINT("Raw data (%d bytes): ", dataLen);
                        for (i = 0; i < dataLen && i < 30; i++) {
                            PRINT("%02X ", pEP0_DataBuf[i]);
                        }
                        PRINT("\n");
                        
                        // 打印解析后的 DIP 映射
                        if (dataLen == 18) {
                            PRINT("Parsed DIP Mapping:\n");
                            PRINT("  Report ID: 0x%02X\n", pEP0_DataBuf[0]);
                            for (i = 0; i < 8; i++) {
                                uint8_t pa5 = pEP0_DataBuf[1 + i*2];
                                uint8_t pa15 = pEP0_DataBuf[2 + i*2];
                                PRINT("  DIP%d: PA5=0x%02X, PA15=0x%02X\n", i, pa5, pa15);
                            }
                            PRINT("  Checksum: 0x%02X\n", pEP0_DataBuf[17]);
                            
                            uint8_t *dip_data = &pEP0_DataBuf[1];
                            uint8_t checksum = pEP0_DataBuf[17];
                            uint8_t calc_sum = 0;
                            for (i = 0; i < 16; i++) {
                                calc_sum += dip_data[i];
                            }
                            PRINT("  Calc checksum: 0x%02X\n", calc_sum);
                            
                            if (checksum == calc_sum) {
                                PRINT(">>> Checksum OK, saving to Flash...\n");
                                if (SaveDipMapToFlash(dip_data) == 0) {
                                    PRINT(">>> Save OK!\n");
                                } else {
                                    PRINT(">>> Save FAILED!\n");
                                }
                            } else {
                                PRINT(">>> Checksum MISMATCH! (stored=0x%02X, calc=0x%02X)\n", checksum, calc_sum);
                            }
                        } else {
                            PRINT("Unexpected data length: %d (expected 18)\n", dataLen);
                        }
                        PRINT("========================================\n");
                    } else {
                        PRINT("Ignoring SET_REPORT: type=%d, id=%d (not Feature Report)\n", 
                              setReportReportType, setReportReportId);
                    }
                    
                    // 清除等待标志
                    waitForSetReportData = 0;
                    setReportReportType = 0;
                    setReportReportId = 0;
                } else {
                    // 如果不是 SET_REPORT 数据，检查是否是 GET_REPORT 的 Status 阶段
                    if (usbHidSetupReqCode == HID_GET_REPORT || last_setup_req_code == HID_GET_REPORT) {
                        PRINT(">>> This is GET_REPORT status stage, ignoring\n");
                    }
                }
                break;

            case UIS_TOKEN_IN | 1:
                R8_UEP1_CTRL ^= RB_UEP_T_TOG;
                R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
                usbHidEp1Busy = FALSE;
                UsbHid_TrySendPending();
                break;

            default:
                break;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;
        }

        if (R8_USB_INT_ST & RB_UIS_SETUP_ACT) {
            R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
            usbHidSetupReqLen = pSetupReqPak->wLength;
            usbHidSetupReqCode = pSetupReqPak->bRequest;
            reqType = pSetupReqPak->bRequestType;
            last_setup_req_code = usbHidSetupReqCode;

            len = 0;
            errFlag = 0;

            // ===== 处理 CLASS 请求 =====
            if ((reqType & USB_REQ_TYP_MASK) == USB_REQ_TYP_CLASS) {
                switch (usbHidSetupReqCode) {

                case HID_GET_REPORT: {
                    uint8_t reportType = (pSetupReqPak->wValue >> 8) & 0xFF;
                    uint8_t reportId = pSetupReqPak->wValue & 0xFF;
                    PRINT("HID_GET_REPORT: type=%d, id=%d\n", reportType, reportId);
                    if (reportType == 0x03 && reportId == 0x01) {
                        uint8_t dip_buf[17];
                        EEPROM_READ(DIP_MAP_FLASH_ADDR, dip_buf, 17);
                        pEP0_DataBuf[0] = 0x01;
                        memcpy(&pEP0_DataBuf[1], dip_buf, 17);
                        len = 18;
                        PRINT("Read Flash (17 bytes): ");
                        for (i = 0; i < 17; i++) PRINT("%02X ", dip_buf[i]);
                        PRINT("\n");
                        PRINT("DIP Mapping from Flash:\n");
                        for (i = 0; i < 8; i++) {
                            PRINT("  DIP%d: PA5=0x%02X, PA15=0x%02X\n", i, dip_buf[i*2], dip_buf[i*2+1]);
                        }
                    }
                } break;

                // ===== HID_SET_REPORT (Setup 阶段) =====
                case HID_SET_REPORT: {
                    uint8_t reportType = (pSetupReqPak->wValue >> 8) & 0xFF;
                    uint8_t reportId = pSetupReqPak->wValue & 0xFF;
                    PRINT("HID_SET_REPORT SETUP: type=%d, id=%d, len=%d\n", 
                          reportType, reportId, pSetupReqPak->wLength);
                    
                    // 设置等待标志
                    waitForSetReportData = 1;
                    setReportReportType = reportType;
                    setReportReportId = reportId;
                    
                    PRINT(">>> Waiting for DATA stage... (flag set)\n");
                    
                    // 保存 setup 请求码
                    last_setup_req_code = usbHidSetupReqCode;
                    
                    // 确保端点0准备好接收数据
                    R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_NAK;
                } break;

                // ===== HID_SET_IDLE =====
                case HID_SET_IDLE:
                    usbHidIdleRate = pEP0_DataBuf[3];
                    PRINT("SET_IDLE: rate=0x%02X\n", usbHidIdleRate);
                    break;

                // ===== HID_SET_PROTOCOL =====
                case HID_SET_PROTOCOL:
                    usbHidProtocol = pEP0_DataBuf[2];
                    PRINT("SET_PROTOCOL: mode=%d\n", usbHidProtocol);
                    break;

                // ===== HID_GET_IDLE =====
                case HID_GET_IDLE:
                    pEP0_DataBuf[0] = usbHidIdleRate;
                    len = 1;
                    break;

                // ===== HID_GET_PROTOCOL =====
                case HID_GET_PROTOCOL:
                    pEP0_DataBuf[0] = usbHidProtocol;
                    len = 1;
                    break;

                default:
                    errFlag = 0xFF;
                    break;
                }
            } else {
                // ===== 处理 STANDARD 请求 =====
                switch (usbHidSetupReqCode) {
                case USB_GET_DESCRIPTOR:
                    switch ((uint8_t)(pSetupReqPak->wValue >> 8)) {
                    case USB_DESCR_TYP_DEVICE:
                        UsbHid_BuildDeviceDesc (usbHidDeviceDesc);
                        usbHidDescrPtr = usbHidDeviceDesc;
                        len = sizeof (usbHidDeviceDesc);
                        break;
                    case USB_DESCR_TYP_CONFIG:
                        usbHidDescrPtr = UsbHid_GetConfigDesc();
                        len = (uint8_t)UsbHid_GetConfigDescLen();
                        break;
                    case USB_DESCR_TYP_STRING:
                        switch ((uint8_t)(pSetupReqPak->wValue & 0x00FFU)) {
                        case 0:
                            usbHidDescrPtr = usbHidLangDesc;
                            len = sizeof (usbHidLangDesc);
                            break;
                        case 1:
                            len = UsbHid_BuildStringDesc (APP_COMPANY_NAME, usbHidStringDesc, sizeof (usbHidStringDesc));
                            usbHidDescrPtr = usbHidStringDesc;
                            break;
                        case 2:
                            len = UsbHid_BuildStringDesc (APP_PRODUCT_NAME_KEYBOARD, usbHidStringDesc, sizeof (usbHidStringDesc));
                            usbHidDescrPtr = usbHidStringDesc;
                            break;
                        case 3: {
                            char deviceId[13];
                            uint8_t macAddr[6];
                            uint8_t idx;
                            GetMACAddress (macAddr);
                            for (idx = 0; idx < 6U; ++idx) {
                                deviceId[idx * 2U] = INT_TO_HEX ((macAddr[idx] >> 4) & 0x0FU);
                                deviceId[idx * 2U + 1U] = INT_TO_HEX (macAddr[idx] & 0x0FU);
                            }
                            deviceId[12] = '\0';
                            len = UsbHid_BuildStringDesc (deviceId, usbHidStringDesc, sizeof (usbHidStringDesc));
                            usbHidDescrPtr = usbHidStringDesc;
                        } break;
                        default:
                            errFlag = 0xFF;
                            break;
                        }
                        break;
                    case USB_DESCR_TYP_HID:
                        usbHidDescrPtr = UsbHid_GetHidDesc ((uint8_t)(pSetupReqPak->wIndex & 0x00FFU));
                        if (usbHidDescrPtr != NULL) {
                            len = 9;
                        } else {
                            errFlag = 0xFF;
                        }
                        break;
                    case USB_DESCR_TYP_REPORT:
                        if ((pSetupReqPak->wIndex & 0x00FFU) == 0U) {
                            usbHidDescrPtr = UsbHid_GetReportDesc();
                            len = (uint8_t)UsbHid_GetReportDescLen();
                        } else {
                            errFlag = 0xFF;
                        }
                        break;
                    default:
                        errFlag = 0xFF;
                        break;
                    }
                    if (usbHidSetupReqLen > len) {
                        usbHidSetupReqLen = len;
                    }
                    len = (usbHidSetupReqLen >= USB_HID_EP0_SIZE) ? USB_HID_EP0_SIZE : (uint8_t)usbHidSetupReqLen;
                    if ((len != 0U) && (usbHidDescrPtr != NULL)) {
                        memcpy (pEP0_DataBuf, usbHidDescrPtr, len);
                        usbHidDescrPtr += len;
                    }
                    break;
                case USB_SET_ADDRESS:
                    usbHidSetupReqLen = pSetupReqPak->wValue & MASK_USB_ADDR;
                    break;
                case USB_GET_CONFIGURATION:
                    pEP0_DataBuf[0] = usbHidConfigured;
                    if (usbHidSetupReqLen > 1U) {
                        usbHidSetupReqLen = 1U;
                    }
                    break;
                case USB_SET_CONFIGURATION:
                    usbHidConfigured = (uint8_t)(pSetupReqPak->wValue & 0x00FFU);
                    PRINT("SET_CONFIG: %d\n", usbHidConfigured);
                    // 标记 USB 已连接
                    usbConnected = 1;
                    usbDisconnectCheckCount = 0;
                    break;
                case USB_CLEAR_FEATURE:
                    if ((pSetupReqPak->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) {
                        switch ((uint8_t)(pSetupReqPak->wIndex & 0x00FFU)) {
                        case 0x81:
                            R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                            break;
                        case 0x01:
                            R8_UEP1_CTRL = (R8_UEP1_CTRL & ~(RB_UEP_R_TOG | MASK_UEP_R_RES)) | UEP_R_RES_ACK;
                            break;
                        default:
                            errFlag = 0xFF;
                            break;
                        }
                    } else {
                        errFlag = 0xFF;
                    }
                    break;
                case USB_GET_INTERFACE:
                    pEP0_DataBuf[0] = 0x00;
                    if (usbHidSetupReqLen > 1U) {
                        usbHidSetupReqLen = 1U;
                    }
                    break;
                case USB_SET_INTERFACE:
                    break;
                case USB_GET_STATUS:
                    pEP0_DataBuf[0] = 0x00;
                    pEP0_DataBuf[1] = 0x00;
                    if (usbHidSetupReqLen > 2U) {
                        usbHidSetupReqLen = 2U;
                    }
                    break;
                default:
                    errFlag = 0xFF;
                    break;
                }
            }

            if (errFlag == 0xFFU) {
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;
            } else {
                if (reqType & USB_REQ_TYP_IN) {
                    len = (usbHidSetupReqLen > USB_HID_EP0_SIZE) ? USB_HID_EP0_SIZE : (uint8_t)usbHidSetupReqLen;
                    usbHidSetupReqLen -= len;
                } else {
                    len = 0;
                }
                R8_UEP0_T_LEN = len;
                R8_UEP0_CTRL = RB_UEP_R_TOG | RB_UEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
            }
            R8_USB_INT_FG = RB_UIF_TRANSFER;
        }
    } else if (intFlag & RB_UIF_BUS_RST) {
        UsbHid_ResetState();
            usbConnected = 0;
    PRINT("USB Bus Reset\n");
        R8_USB_INT_FG = RB_UIF_BUS_RST;
    } else if (intFlag & RB_UIF_SUSPEND) {
            // USB 挂起中断 - 拔掉 USB 时会触发
    PRINT(">>> USB SUSPEND! USB disconnected detected!\n");
    usbConnected = 0;
    usbHidConfigured = 0;
    R8_USB_INT_FG = RB_UIF_SUSPEND;
    } else {
        R8_USB_INT_FG = intFlag;
    }

    // ===== USB 断开检测 =====
    if (usbConnected && usbHidConfigured) {
        uint8_t dev_addr = R8_USB_DEV_AD & MASK_USB_ADDR;
        if (dev_addr == 0) {
            usbDisconnectCheckCount++;
            if (usbDisconnectCheckCount >= 10) {
                PRINT(">>> USB DISCONNECTED! (addr=0, count=%d)\n", usbDisconnectCheckCount);
                usbHidConfigured = 0;
                usbConnected = 0;
                usbDisconnectCheckCount = 0;
            }
        } else {
            usbDisconnectCheckCount = 0;
        }
    }
}

__INTERRUPT __HIGH_CODE
void USB_IRQHandler (void) {
    USB_DevTransProcess();
}

void DevEP1_IN_Deal (uint8_t l) {
    R8_UEP1_T_LEN = l;
    R8_UEP1_CTRL = (R8_UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_ACK;
}

void USB_DeviceInit (void) {
    R8_USB_CTRL = 0x00;
    R8_UEP4_1_MOD = RB_UEP4_RX_EN | RB_UEP4_TX_EN | RB_UEP1_RX_EN | RB_UEP1_TX_EN;
    R8_UEP2_3_MOD = RB_UEP2_RX_EN | RB_UEP2_TX_EN | RB_UEP3_RX_EN | RB_UEP3_TX_EN;
    R16_UEP0_DMA = (uint16_t)(uint32_t)pEP0_RAM_Addr;
    R16_UEP1_DMA = (uint16_t)(uint32_t)pEP1_RAM_Addr;
    R16_UEP2_DMA = (uint16_t)(uint32_t)pEP2_RAM_Addr;
    R16_UEP3_DMA = (uint16_t)(uint32_t)pEP3_RAM_Addr;
    R8_UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    R8_UEP1_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
    R8_UEP2_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
    R8_UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK | RB_UEP_AUTO_TOG;
    R8_UEP4_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    R8_USB_DEV_AD = 0x00;
    R8_USB_CTRL = RB_UC_DEV_PU_EN | RB_UC_INT_BUSY | RB_UC_DMA_EN;
    R16_PIN_ANALOG_IE |= RB_PIN_USB_IE | RB_PIN_USB_DP_PU;
    R8_USB_INT_FG = 0xFF;
    R8_UDEV_CTRL = RB_UD_PD_DIS | RB_UD_PORT_EN;
    R8_USB_INT_EN = RB_UIE_SUSPEND | RB_UIE_BUS_RST | RB_UIE_TRANSFER;
}


uint8_t UsbHid_GetConnected(void) {
    return usbConnected;
}