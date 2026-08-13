/********************************** (C) COPYRIGHT *******************************
 * File Name          : hidkbd.c
 * Author             : WCH & motozilog
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : 蓝牙键盘应用程序，初始化广播连接参数，然后广播，直至连接主机后，定时上传键值
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "devinfoservice.h"
#include "battservice.h"
#include "hidkbdservice.h"
#include "hiddev.h"
#include "hidkbd.h"

// motozilog start
#include "getId.h"
#include "CH59x_common.h"  // 添加低功耗函数头文件
#include "getId.h"
// motozilog end

/*********************************************************************
 * MACROS
 */
// HID keyboard input report length
#define HID_KEYBOARD_IN_RPT_LEN 8

// HID LED output report length
#define HID_LED_OUT_RPT_LEN 1

/*********************************************************************
 * CONSTANTS
 */
// Param update delay
#define START_PARAM_UPDATE_EVT_DELAY 12800

// Param update delay
#define START_PHY_UPDATE_DELAY 1600

// HID idle timeout in msec; set to zero to disable timeout
#define DEFAULT_HID_IDLE_TIMEOUT 60000

// Minimum connection interval (units of 1.25ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL 8

// Maximum connection interval (units of 1.25ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL 8

// Slave latency to use if parameter update request
#define DEFAULT_DESIRED_SLAVE_LATENCY 0

// Supervision timeout value (units of 10ms)
#define DEFAULT_DESIRED_CONN_TIMEOUT 500

// Default passcode
#define DEFAULT_PASSCODE 0

// Default GAP pairing mode
#define DEFAULT_PAIRING_MODE GAPBOND_PAIRING_MODE_WAIT_FOR_REQ

// Default MITM mode (TRUE to require passcode or OOB when pairing)
#define DEFAULT_MITM_MODE FALSE

// Default bonding mode, TRUE to bond
#define DEFAULT_BONDING_MODE TRUE

// Default GAP bonding I/O capabilities
#define DEFAULT_IO_CAPABILITIES GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT

// Battery level is critical when it is less than this %
#define DEFAULT_BATT_CRITICAL_LEVEL 6

// motozilog start
//  蓝牙连接指示灯闪烁间隔 (ms)
#define LED_BLINK_INTERVAL 1000

// 按键扫描间隔 (ms)
#define KEY_SCAN_INTERVAL 50
// 休眠超时时间 (ms) - 300秒(5分钟，调试时为20秒)
#define SLEEP_TIMEOUT 300000
//#define SLEEP_TIMEOUT 20000

// 连接状态下休眠超时时间 (ms) - 1800秒(30分钟，调试时为30秒)
#define SLEEP_TIMEOUT_RUNNING 1800000
//#define SLEEP_TIMEOUT_RUNNING 30000


// ADC采样间隔 (ms) - 60秒 (1分钟，调试时为1秒)
#define ADC_SAMPLE_INTERVAL 60000
// #define ADC_SAMPLE_INTERVAL 1000

// 低电量阈值 (mV)
#define LOW_BATTERY_THRESHOLD 3500

// ALWAYS_ON阈值 (mV)
#define ALWAYS_ON_THRESHOLD 1000


// 休眠计时器模式
#define SLEEP_MODE_IDLE 0     // 空闲模式（未连接）
#define SLEEP_MODE_RUNNING 1  // 运行模式（已连接）

// motozilog end
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Task ID
static uint8_t hidEmuTaskId = INVALID_TASK_ID;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// GAP Profile - Name attribute for SCAN RSP data
static uint8_t scanRspData[] = {
    0x0F,                            // length of this data
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,  // AD Type = Complete local name
    'O',
    'S',
    'H',
    'W',
    'T',
    'u',
    'n',
    'e',
    'r',
    '_',
    '1',
    '2',
    '3',
    '4',                                            // connection interval range
    0x05,                                           // length of this data
    GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
    LO_UINT16 (DEFAULT_DESIRED_MIN_CONN_INTERVAL),  // 100ms
    HI_UINT16 (DEFAULT_DESIRED_MIN_CONN_INTERVAL),
    LO_UINT16 (DEFAULT_DESIRED_MAX_CONN_INTERVAL),  // 1s
    HI_UINT16 (DEFAULT_DESIRED_MAX_CONN_INTERVAL),

    // service UUIDs
    0x05,  // length of this data
    GAP_ADTYPE_16BIT_MORE,
    LO_UINT16 (HID_SERV_UUID),
    HI_UINT16 (HID_SERV_UUID),
    LO_UINT16 (BATT_SERV_UUID),
    HI_UINT16 (BATT_SERV_UUID),

    // Tx power level
    0x02,  // length of this data
    GAP_ADTYPE_POWER_LEVEL,
    0      // 0dBm
};

// Advertising data
static uint8_t advertData[] = {
    // flags
    0x02,  // length of this data
    GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    // appearance
    0x03,  // length of this data
    GAP_ADTYPE_APPEARANCE,
    LO_UINT16 (GAP_APPEARE_HID_KEYBOARD),
    HI_UINT16 (GAP_APPEARE_HID_KEYBOARD)};

// Device name attribute value
static uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "OSHWTuner_1234";

// HID Dev configuration
static hidDevCfg_t hidEmuCfg = {
    DEFAULT_HID_IDLE_TIMEOUT,  // Idle timeout
    HID_FEATURE_FLAGS          // HID feature flags
};

static uint16_t hidEmuConnHandle = GAP_CONNHANDLE_INIT;

// motozilog start
static uint8_t ledBlinkEnable = FALSE;          // LED闪烁使能标志

static volatile uint8_t sleep_enabled = FALSE;  // 休眠使能标志
static uint32_t sleep_timer_count = 0;          // 休眠计时器计数
static uint8_t sleep_mode = SLEEP_MODE_IDLE;    // 休眠模式
static uint32_t last_key_time = 0;              // 上次按键时间

static uint16_t adc_raw_value = 0;              // ADC原始值
static int16_t adc_calibrated_value = 0;        // 校准后的ADC值
static int16_t rough_calib_value = 0;           // ADC粗调偏差值

int32_t battery_mv = 0;                         // 电池电压 (mV)


static uint16_t adc_ain9_raw = 0;              // AIN9 (PB6) ADC原始值
static int32_t vin_ain9_mv = 0;                // AIN9输入电压 (mV)
static int32_t battery_ain9_mv = 0;            // AIN9电池电压 (mV)
static volatile uint8_t is_always_on = FALSE;  // 是否使用长用功能
// motozilog end

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void hidEmu_ProcessTMOSMsg (tmos_event_hdr_t *pMsg);
static void hidEmuSendKbdReport (uint8_t keycode);
static uint8_t hidEmuRcvReport (uint8_t len, uint8_t *pData);
static uint8_t hidEmuRptCB (uint8_t id, uint8_t type, uint16_t uuid,
                            uint8_t oper, uint16_t *pLen, uint8_t *pData);
static void hidEmuEvtCB (uint8_t evt);
static void hidEmuStateCB (gapRole_States_t newState, gapRoleEvent_t *pEvent);

static void ADC_Sample (void);
static void ADC_Sample_Always_On (void);

/*********************************************************************
 * PROFILE CALLBACKS
 */

static hidDevCB_t hidEmuHidCBs = {
    hidEmuRptCB,
    hidEmuEvtCB,
    NULL,
    hidEmuStateCB};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************

/*********************************************************************
 * @fn      ADC_Init
 *
 * @brief   初始化ADC (PA4/AIN0，单端输入，PGA增益-6db/1/2倍)
 *
 * @return  none
 */
static void ADC_Init (void) {
    // 采样AIN9 (PB6) 电压，仅在初始化时调用一次
    ADC_Sample_Always_On();


    // 配置PA4为模拟输入（浮空）
    GPIOA_ModeCfg (GPIO_Pin_4, GPIO_ModeIN_Floating);

    // ADC单通道初始化：采样频率3.2MHz，PGA增益0dB
    // 注意：先初始化为0dB，然后再通过ADC_PGACfg设置增益
    ADC_ExtSingleChSampInit (SampleFreq_3_2, ADC_PGA_1_2);

    // ★★★ 关键：选择内部1.05V参考源 ★★★
    // 根据CH59x手册，需要设置R8_ADC_CFG的RB_ADC_VREF_SEL位
    // 0: 外部VREF (VDD)   1: 内部VREF (1.05V)
    R8_ADC_CFG |= RB_ADC_POWER_ON;  // 选择内部1.05V参考


    // 配置PGA增益为-6dB (1/2倍)
    // ADC_PGA_1_2 对应 -6dB, 1/2倍
    ADC_PGACfg (ADC_PGA_1_2);

    // 选择ADC通道0 (PA4/AIN0)
    ADC_ChannelCfg (CH_EXTIN_0);

    // 获取ADC粗调偏差值（用于校准）
    rough_calib_value = ADC_DataCalib_Rough();

    PRINT ("ADC initialized (AIN0/PA4, PGA -6dB, Calib: %d)\n", rough_calib_value);

    // 初始化时做一次采样
    ADC_Sample();
}

/*********************************************************************
 * @fn      ADC_Sample_Always_On
 *
 * @brief   采样AIN9 (PB6) 电压，91K+220K分压，取10次采样后滤波平均
 *          (丢弃最高2次和最低2次，取中间6次平均)
 *
 * @return  none
 */
static void ADC_Sample_Always_On (void) {
    uint16_t adc_value;
    int32_t vin_mv;
    int32_t battery_mv;
    uint8_t saved_channel;
    uint8_t saved_cfg;
    uint8_t i, j;
    int32_t vin_samples[10];
    int32_t battery_samples[10];
    int32_t temp;
    int32_t sum_vin = 0;
    int32_t sum_battery = 0;

    PRINT ("Sampling AIN9 (PB6) - 10 times with filtering...\n");

    // 保存当前ADC配置
    saved_channel = R8_ADC_CHANNEL;
    saved_cfg = R8_ADC_CFG;

    // 配置PB6为模拟输入（浮空）
    GPIOB_ModeCfg (GPIO_Pin_6, GPIO_ModeIN_Floating);

    // ADC单通道初始化：采样频率3.2MHz，PGA增益-6dB (1/2倍)
    ADC_ExtSingleChSampInit (SampleFreq_3_2, ADC_PGA_1_2);
    ADC_PGACfg (ADC_PGA_1_2);
    ADC_ChannelCfg (CH_EXTIN_9);

    // 连续采样10次，每次间隔100ms
    for (i = 0; i < 10; i++) {
        // 执行单次ADC转换
        adc_value = ADC_ExcutSingleConver();
        adc_ain9_raw = adc_value;

        // 应用粗调校准
        int16_t calib_val = (int16_t)adc_value + rough_calib_value;
        if (calib_val < 0)
            calib_val = 0;
        if (calib_val > 4095)
            calib_val = 4095;

// 计算电压
#define VREF_MV 1050
#define ADC_MAX 4095
#define PGA_INV 2  // 1/A, 当A=0.5时

        int32_t calc = (int32_t)PGA_INV * 2 * calib_val * VREF_MV / ADC_MAX;
        calc = calc - (int32_t)PGA_INV * VREF_MV + VREF_MV;
        vin_mv = (int32_t)calc;

        // 补偿分压电阻 (91K + 220K)
        battery_mv = vin_mv * (91 + 220) / 220;

        // 存入数组
        vin_samples[i] = vin_mv;
        battery_samples[i] = battery_mv;

        // 打印每次采样的值
        PRINT ("  Sample %d: ADC=%4d, VIN=%d.%03dV, BAT=%d.%03dV\n",
               i + 1, adc_value,
               vin_mv / 1000, vin_mv % 1000,
               battery_mv / 1000, battery_mv % 1000);

        // 间隔100ms (除了最后一次)
        if (i < 9) {
            DelayMs (100);
        }
    }

    // ----- 对vin_samples进行排序 (冒泡排序) -----
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 9 - i; j++) {
            if (vin_samples[j] > vin_samples[j + 1]) {
                temp = vin_samples[j];
                vin_samples[j] = vin_samples[j + 1];
                vin_samples[j + 1] = temp;
            }
        }
    }

    // ----- 对battery_samples进行排序 (冒泡排序) -----
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 9 - i; j++) {
            if (battery_samples[j] > battery_samples[j + 1]) {
                temp = battery_samples[j];
                battery_samples[j] = battery_samples[j + 1];
                battery_samples[j + 1] = temp;
            }
        }
    }

    // 丢弃最高2次和最低2次，取中间6次求和
    for (i = 2; i < 8; i++) {
        sum_vin += vin_samples[i];
        sum_battery += battery_samples[i];
    }

    // 计算平均值
    vin_ain9_mv = sum_vin / 6;
    battery_ain9_mv = sum_battery / 6;

    if (battery_ain9_mv > ALWAYS_ON_THRESHOLD) {
        is_always_on = TRUE;
    }

    PRINT ("AIN9 (PB6) Filtered - VIN: %d.%03d V, Battery: %d.%03d V Always On: %d\n",
           vin_ain9_mv / 1000, vin_ain9_mv % 1000,
           battery_ain9_mv / 1000, battery_ain9_mv % 1000, is_always_on);

    // 恢复ADC配置到AIN0 (PA4)
    R8_ADC_CHANNEL = saved_channel;
    R8_ADC_CFG = saved_cfg;
    ADC_PGACfg (ADC_PGA_1_2);

    // 恢复PB6为输入上拉（释放引脚）
    GPIOB_ModeCfg (GPIO_Pin_6, GPIO_ModeIN_PU);
}

/*********************************************************************
 * @fn      ADC_Sample
 *
 * @brief   采样ADC值并转换为电压
 *
 * @return  none
 */
static void ADC_Sample (void) {
    uint16_t adc_value;
    int32_t vin_mv;

    adc_value = ADC_ExcutSingleConver();
    adc_calibrated_value = (int16_t)adc_value + rough_calib_value;

    if (adc_calibrated_value < 0)
        adc_calibrated_value = 0;
    if (adc_calibrated_value > 4095)
        adc_calibrated_value = 4095;

    adc_raw_value = (uint16_t)adc_calibrated_value;

    // 使用简化公式: VIN = 2*Vref*(ADC/4096 - 0.5) / A + Vref
    // 合并系数后: VIN = Vref * (2*ADC/4096 - 1) / A + Vref
    // 对于 A=0.5: VIN = Vref * (4*ADC/4096 - 2) + Vref
    // 简化: VIN = Vref * (4*ADC/4096 - 1)

#define VREF_MV 1050
#define ADC_MAX 4095
#define PGA_INV 2  // 1/A, 当A=0.5时

    // 计算VIN (mV)
    // VIN = Vref * (PGA_INV * 2 * ADC / ADC_MAX - PGA_INV + 1)
    int32_t calc = (int32_t)PGA_INV * 2 * adc_raw_value * VREF_MV / ADC_MAX;
    calc = calc - (int32_t)PGA_INV * VREF_MV + VREF_MV;
    vin_mv = (int32_t)calc;

    // 补偿分压电阻
    battery_mv = vin_mv * (91 + 220) / 220;

    // 如果电池电压低于3.5V，PB7输出高电平，否则低电平
    if (battery_mv < LOW_BATTERY_THRESHOLD) {
        GPIOB_SetBits (GPIO_Pin_7);
    } else {
        GPIOB_ResetBits (GPIO_Pin_7);
    }

    PRINT ("ADC Raw: %4d, Calib: %4d, VIN: %d.%03d V, Battery: %d.%03d V\n",
           adc_value, adc_raw_value,
           vin_mv / 1000, vin_mv % 1000,
           battery_mv / 1000, battery_mv % 1000);
}

/*********************************************************************
 * @fn      configWakeupPin
 *
 * @brief   配置PA5和PA15为唤醒源
 *
 * @return  none
 */
static void configWakeupPin (void) {
    // 配置PA5和PA15为唤醒源（下降沿唤醒）
    GPIOA_ITModeCfg (GPIO_Pin_5 | GPIO_Pin_15, GPIO_ITMode_FallEdge);
    PFIC_EnableIRQ (GPIO_A_IRQn);
    GPIOA_ClearITFlagBit (GPIO_Pin_5 | GPIO_Pin_15);
}

/*********************************************************************
 * @fn      enterSleepMode
 *
 * @brief   进入睡眠模式（使用官方LowPower_Sleep）
 *
 * @return  none
 */
static void enterSleepMode (void) {
    PRINT ("Entering Sleep Mode...\n");

    // 停止所有任务
    ledBlinkEnable = FALSE;
    tmos_stop_task (hidEmuTaskId, LED_BLINK_EVT);
    tmos_stop_task (hidEmuTaskId, KEY_SCAN_EVT);
    tmos_stop_task (hidEmuTaskId, SLEEP_TIMER_EVT);
    tmos_stop_task (hidEmuTaskId, ADC_SAMPLE_EVT);

    // 配置唤醒引脚 - PA5和PA15作为唤醒源（下降沿唤醒）
    GPIOA_ModeCfg (GPIO_Pin_5 | GPIO_Pin_15, GPIO_ModeIN_PU);
    GPIOA_ITModeCfg (GPIO_Pin_5 | GPIO_Pin_15, GPIO_ITMode_FallEdge);
    PFIC_EnableIRQ (GPIO_A_IRQn);
    GPIOA_ClearITFlagBit (GPIO_Pin_5 | GPIO_Pin_15);

    // 关灯
    GPIOB_ResetBits (GPIO_Pin_4);
    GPIOB_ResetBits (GPIO_Pin_7);

    // 配置唤醒源 - 使用PWR_PeriphWakeUpCfg
    // 参数: ENABLE, 唤醒源, 延迟模式
    PWR_PeriphWakeUpCfg (ENABLE, RB_SLP_GPIO_WAKE, Short_Delay);

    // 在进入休眠前打印配置
    PRINT ("GPIOA INT EN: 0x%04X\n", R16_PA_INT_EN);
    PRINT ("SLP_WAKE_CTRL: 0x%02X\n", R8_SLP_WAKE_CTRL);
    PRINT ("POWER_PLAN: 0x%04X\n", R16_POWER_PLAN);

    // 进入Sleep模式，保留24K+2K SRAM供电
    LowPower_Sleep (RB_PWR_RAM24K | RB_PWR_RAM2K);

    // 唤醒后执行
    PRINT ("Wake up from Sleep Mode!\n");

    // 清除中断标志
    GPIOA_ClearITFlagBit (GPIO_Pin_5 | GPIO_Pin_15);
    PFIC_DisableIRQ (GPIO_A_IRQn);

    // 恢复GPIO配置
    GPIOB_ModeCfg (GPIO_Pin_4, GPIO_ModeOut_PP_5mA);
    GPIOB_ModeCfg (GPIO_Pin_7, GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg (GPIO_Pin_5 | GPIO_Pin_15, GPIO_ModeIN_PU);
    GPIOA_ModeCfg (GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14, GPIO_ModeIN_PU);

    // 恢复LED闪烁
    ledBlinkEnable = TRUE;
    GPIOB_ResetBits (GPIO_Pin_4);
    tmos_start_task (hidEmuTaskId, LED_BLINK_EVT, LED_BLINK_INTERVAL);
    tmos_start_task (hidEmuTaskId, KEY_SCAN_EVT, KEY_SCAN_INTERVAL);
    tmos_start_task (hidEmuTaskId, ADC_SAMPLE_EVT, ADC_SAMPLE_INTERVAL);

    // 重置休眠计时器
    sleep_timer_count = 0;
    last_key_time = 0;
    sleep_enabled = FALSE;
    tmos_start_task (hidEmuTaskId, SLEEP_TIMER_EVT, 1000);
}

/* @fn      HidEmu_Init
 *
 * @brief   Initialization function for the HidEmuKbd App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by TMOS.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void HidEmu_Init() {
    hidEmuTaskId = TMOS_ProcessEventRegister (HidEmu_ProcessEvent);

    // motozilog start: 修改广播名称中的1234为UID后4位
    {
        char uid_str[17] = {0};
        GetChipUID (uid_str);
        scanRspData[12] = uid_str[12];
        scanRspData[13] = uid_str[13];
        scanRspData[14] = uid_str[14];
        scanRspData[15] = uid_str[15];

        attDeviceName[10] = uid_str[12];
        attDeviceName[11] = uid_str[13];
        attDeviceName[12] = uid_str[14];
        attDeviceName[13] = uid_str[15];
    }
    // motozilog end


    // Setup the GAP Peripheral Role Profile
    {
        uint8_t initial_advertising_enable = TRUE;

        // Set the GAP Role Parameters
        GAPRole_SetParameter (GAPROLE_ADVERT_ENABLED, sizeof (uint8_t), &initial_advertising_enable);

        GAPRole_SetParameter (GAPROLE_ADVERT_DATA, sizeof (advertData), advertData);
        GAPRole_SetParameter (GAPROLE_SCAN_RSP_DATA, sizeof (scanRspData), scanRspData);
    }

    // Set the GAP Characteristics
    GGS_SetParameter (GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, (void *)attDeviceName);

    // Setup the GAP Bond Manager
    {
        uint32_t passkey = DEFAULT_PASSCODE;
        uint8_t pairMode = DEFAULT_PAIRING_MODE;
        uint8_t mitm = DEFAULT_MITM_MODE;
        uint8_t ioCap = DEFAULT_IO_CAPABILITIES;
        uint8_t bonding = DEFAULT_BONDING_MODE;
        GAPBondMgr_SetParameter (GAPBOND_PERI_DEFAULT_PASSCODE, sizeof (uint32_t), &passkey);
        GAPBondMgr_SetParameter (GAPBOND_PERI_PAIRING_MODE, sizeof (uint8_t), &pairMode);
        GAPBondMgr_SetParameter (GAPBOND_PERI_MITM_PROTECTION, sizeof (uint8_t), &mitm);
        GAPBondMgr_SetParameter (GAPBOND_PERI_IO_CAPABILITIES, sizeof (uint8_t), &ioCap);
        GAPBondMgr_SetParameter (GAPBOND_PERI_BONDING_ENABLED, sizeof (uint8_t), &bonding);
    }
    {
        gapPeriConnectParams_t ConnectParams;
        ConnectParams.intervalMin = 6;
        ConnectParams.intervalMax = 9;
        ConnectParams.latency = 20;
        ConnectParams.timeout = 0x012C;
        GGS_SetParameter (GGS_PERI_CONN_PARAM_ATT, sizeof (gapPeriConnectParams_t), &ConnectParams);
    }
    // Setup Battery Characteristic Values
    {
        uint8_t critical = DEFAULT_BATT_CRITICAL_LEVEL;
        Batt_SetParameter (BATT_PARAM_CRITICAL_LEVEL, sizeof (uint8_t), &critical);
    }

    // Set up HID keyboard service
    Hid_AddService();

    // Register for HID Dev callback
    HidDev_Register (&hidEmuCfg, &hidEmuHidCBs);

    // Setup a delayed profile startup
    tmos_set_event (hidEmuTaskId, START_DEVICE_EVT);

    // motozilog start: 启动LED闪烁
    ledBlinkEnable = TRUE;
    GPIOB_ResetBits (GPIO_Pin_4);  // 初始设为低电平
    tmos_start_task (hidEmuTaskId, LED_BLINK_EVT, LED_BLINK_INTERVAL);

    // 启动按键扫描
    tmos_start_task (hidEmuTaskId, KEY_SCAN_EVT, KEY_SCAN_INTERVAL);

    // 启动休眠定时器（每秒检查一次）
    sleep_timer_count = 0;
    tmos_start_task (hidEmuTaskId, SLEEP_TIMER_EVT, 1000);

    // 初始化ADC并启动采样任务
    ADC_Init();
    tmos_start_task (hidEmuTaskId, ADC_SAMPLE_EVT, ADC_SAMPLE_INTERVAL);
    // motozilog end
}

/*********************************************************************
 * @fn      hidEmuSendKey
 *
 * @brief   发送单个HID键值（包含按下和释放）
 *
 * @param   keycode - HID键码，0x00表示释放
 *
 * @return  none
 */
static void hidEmuSendKey (uint8_t keycode) {
    uint8_t buf[HID_KEYBOARD_IN_RPT_LEN] = {0};

    if (keycode != 0x00) {
        // 按下：发送键值
        buf[2] = keycode;
        HidDev_Report (HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                       HID_KEYBOARD_IN_RPT_LEN, buf);
    } else {
        // 释放：发送空键
        HidDev_Report (HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                       HID_KEYBOARD_IN_RPT_LEN, buf);
    }
}

/*********************************************************************
 * @fn      HidEmu_ProcessEvent
 *
 * @brief   HidEmuKbd Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16_t HidEmu_ProcessEvent (uint8_t task_id, uint16_t events) {
    static uint8_t send_char = 4;

    if (events & SYS_EVENT_MSG) {
        uint8_t *pMsg;

        if ((pMsg = tmos_msg_receive (hidEmuTaskId)) != NULL) {
            hidEmu_ProcessTMOSMsg ((tmos_event_hdr_t *)pMsg);

            // Release the TMOS message
            tmos_msg_deallocate (pMsg);
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if (events & START_DEVICE_EVT) {
        return (events ^ START_DEVICE_EVT);
    }

    if (events & START_PARAM_UPDATE_EVT) {
        // Send connect param update request
        GAPRole_PeripheralConnParamUpdateReq (hidEmuConnHandle,
                                              DEFAULT_DESIRED_MIN_CONN_INTERVAL,
                                              DEFAULT_DESIRED_MAX_CONN_INTERVAL,
                                              DEFAULT_DESIRED_SLAVE_LATENCY,
                                              DEFAULT_DESIRED_CONN_TIMEOUT,
                                              hidEmuTaskId);

        return (events ^ START_PARAM_UPDATE_EVT);
    }

    if (events & START_PHY_UPDATE_EVT) {
        // start phy update
        PRINT ("Send Phy Update %x...\n", GAPRole_UpdatePHY (hidEmuConnHandle, 0,
                                                             GAP_PHY_BIT_LE_2M, GAP_PHY_BIT_LE_2M, 0));

        return (events ^ START_PHY_UPDATE_EVT);
    }

    // if(events & START_REPORT_EVT)
    // {
    //     hidEmuSendKbdReport(send_char);
    //     send_char++;
    //     if(send_char >= 29)
    //         send_char = 4;
    //     hidEmuSendKbdReport(0x00);
    //     tmos_start_task(hidEmuTaskId, START_REPORT_EVT, 2000);
    //     return (events ^ START_REPORT_EVT);
    // }

    //LED闪烁事件处理
    if (events & LED_BLINK_EVT) {
        if (ledBlinkEnable) {
            // 读取当前状态并打印
            uint8_t pin_state = GPIOB_ReadPortPin (GPIO_Pin_4);

            // 翻转
            if (pin_state) {
                GPIOB_ResetBits (GPIO_Pin_4);
            } else {
                GPIOB_SetBits (GPIO_Pin_4);
            }
            // 重新启动定时器
            tmos_start_task (hidEmuTaskId, LED_BLINK_EVT, LED_BLINK_INTERVAL);
        }
        return (events ^ LED_BLINK_EVT);
    }

    //按键扫描事件处理
    if (events & KEY_SCAN_EVT) {
        static uint8_t last_pa5_state = 0;
        static uint8_t last_pa15_state = 0;
        static uint8_t last_dip_value = 0xFF;

        uint8_t pa5_pressed = !(GPIOA_ReadPortPin (GPIO_Pin_5));
        uint8_t pa15_pressed = !(GPIOA_ReadPortPin (GPIO_Pin_15));

        uint8_t dip_value = getDIPValue();  // 使用 getId.c 中的函数

        if (dip_value != last_dip_value) {
            PRINT ("DIP Switch changed: %d\n", dip_value);
            last_dip_value = dip_value;
        }

        // 获取当前DIP值对应的按键映射
        uint8_t key_pa5, key_pa15;
        getDIPKeyMap (dip_value, &key_pa5, &key_pa15);
        // 检测PA5按键按下 (上升沿检测)
        if (pa5_pressed && !last_pa5_state) {
            hidEmuSendKey (key_pa5);
            PRINT ("PA5: DIP=%d, Key=0x%02X\n", dip_value, key_pa5);
            last_key_time = 0;  // 重置按键计时
        }

        // 检测PA15按键按下 (上升沿检测)
        if (pa15_pressed && !last_pa15_state) {
            hidEmuSendKey (key_pa15);
            PRINT ("PA15: DIP=%d, Key=0x%02X\n", dip_value, key_pa15);
            last_key_time = 0;  // 重置按键计时
        }

        // 检测按键释放 (下降沿)
        if (!pa5_pressed && last_pa5_state) {
            hidEmuSendKey (0x00);  // 发送空键释放
        }
        if (!pa15_pressed && last_pa15_state) {
            hidEmuSendKey (0x00);  // 发送空键释放
        }

        // 保存当前状态
        last_pa5_state = pa5_pressed;
        last_pa15_state = pa15_pressed;

        // 重新启动按键扫描定时器
        tmos_start_task (hidEmuTaskId, KEY_SCAN_EVT, KEY_SCAN_INTERVAL);
        return (events ^ KEY_SCAN_EVT);
    }

    // 休眠定时器事件处理
    if (events & SLEEP_TIMER_EVT) {
        sleep_timer_count++;
        last_key_time++;  // 按键空闲时间累加

        // 如果is_always_on为TRUE，不进行休眠
        if (is_always_on) {
            // 常开模式，不做任何休眠操作
            tmos_start_task (hidEmuTaskId, SLEEP_TIMER_EVT, 1000);
            return (events ^ SLEEP_TIMER_EVT);
        }

        // 根据连接状态选择不同的超时时间
        uint32_t timeout = SLEEP_TIMEOUT / 1000;  // 默认空闲超时

        if (hidEmuConnHandle != GAP_CONNHANDLE_INIT) {
            // 已连接状态，使用运行超时
            timeout = SLEEP_TIMEOUT_RUNNING / 1000;
            sleep_mode = SLEEP_MODE_RUNNING;
        } else {
            sleep_mode = SLEEP_MODE_IDLE;
        }

        // 检查是否需要进入休眠
        // 条件1: 超时未连接 且 未休眠
        // 条件2: 已连接 且 超时未按键 且 未休眠
        uint8_t should_sleep = FALSE;

        if (hidEmuConnHandle == GAP_CONNHANDLE_INIT) {
            // 未连接：超时进入休眠
            if (sleep_timer_count >= timeout && !sleep_enabled) {
                should_sleep = TRUE;
            }
        } else {
            // 已连接：超时且无按键操作进入休眠
            if (last_key_time >= timeout && !sleep_enabled) {
                should_sleep = TRUE;
                PRINT ("No key pressed for %d seconds, entering sleep...\n", timeout);
            }
        }

        if (should_sleep) {
            sleep_enabled = TRUE;
            enterSleepMode();
            sleep_enabled = FALSE;
            sleep_timer_count = 0;
            last_key_time = 0;  // 唤醒后重置按键计时
        }

        // 重新启动定时器
        tmos_start_task (hidEmuTaskId, SLEEP_TIMER_EVT, 1000);
        return (events ^ SLEEP_TIMER_EVT);
    }

    // ADC采样事件处理
    if (events & ADC_SAMPLE_EVT) {
        ADC_Sample();
        tmos_start_task (hidEmuTaskId, ADC_SAMPLE_EVT, ADC_SAMPLE_INTERVAL);
        return (events ^ ADC_SAMPLE_EVT);
    }


    return 0;
}

/*********************************************************************
 * @fn      hidEmu_ProcessTMOSMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void hidEmu_ProcessTMOSMsg (tmos_event_hdr_t *pMsg) {
    switch (pMsg->event) {
    default:
        break;
    }
}

/*********************************************************************
 * @fn      hidEmuSendKbdReport
 *
 * @brief   Build and send a HID keyboard report.
 *
 * @param   keycode - HID keycode.
 *
 * @return  none
 */
static void hidEmuSendKbdReport (uint8_t keycode) {
    uint8_t buf[HID_KEYBOARD_IN_RPT_LEN];

    buf[0] = 0;        // Modifier keys
    buf[1] = 0;        // Reserved
    buf[2] = keycode;  // Keycode 1
    buf[3] = 0;        // Keycode 2
    buf[4] = 0;        // Keycode 3
    buf[5] = 0;        // Keycode 4
    buf[6] = 0;        // Keycode 5
    buf[7] = 0;        // Keycode 6

    HidDev_Report (HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT,
                   HID_KEYBOARD_IN_RPT_LEN, buf);
}

/*********************************************************************
 * @fn      hidEmuStateCB
 *
 * @brief   GAP state change callback.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void hidEmuStateCB (gapRole_States_t newState, gapRoleEvent_t *pEvent) {
    switch (newState & GAPROLE_STATE_ADV_MASK) {
    case GAPROLE_STARTED: {
        uint8_t ownAddr[6];
        GAPRole_GetParameter (GAPROLE_BD_ADDR, ownAddr);
        GAP_ConfigDeviceAddr (ADDRTYPE_STATIC, ownAddr);
        PRINT ("Initialized..\n");
    } break;

    case GAPROLE_ADVERTISING:
        if (pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT) {
            PRINT ("Advertising..\n");
        }
        break;

    case GAPROLE_CONNECTED:
        if (pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT) {
            gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;

            // get connection handle
            hidEmuConnHandle = event->connectionHandle;
            tmos_start_task (hidEmuTaskId, START_PARAM_UPDATE_EVT, START_PARAM_UPDATE_EVT_DELAY);
            PRINT ("Connected..\n");

            // motozilog start: 连接成功后停止闪烁，PB4设为高电平
            ledBlinkEnable = FALSE;
            GPIOB_SetBits (GPIO_Pin_4);

            // 重置休眠计时器和按键计时
            sleep_timer_count = 0;
            last_key_time = 0;
            // 切换到运行模式
            sleep_mode = SLEEP_MODE_RUNNING;
            // motozilog end
        }
        break;

    case GAPROLE_CONNECTED_ADV:
        if (pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT) {
            PRINT ("Connected Advertising..\n");
        }
        break;

    case GAPROLE_WAITING:
        if (pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT) {
            PRINT ("Waiting for advertising..\n");
        } else if (pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT) {
            PRINT ("Disconnected.. Reason:%x\n", pEvent->linkTerminate.reason);

            // motozilog start: 断开连接后恢复闪烁
            ledBlinkEnable = TRUE;
            GPIOB_ResetBits (GPIO_Pin_4);  // 先设为低电平
            tmos_start_task (hidEmuTaskId, LED_BLINK_EVT, LED_BLINK_INTERVAL);

            // 重置休眠计时器
            hidEmuConnHandle = GAP_CONNHANDLE_INIT;
            sleep_timer_count = 0;
            last_key_time = 0;
            sleep_enabled = FALSE;
            sleep_mode = SLEEP_MODE_IDLE;
            tmos_start_task (hidEmuTaskId, SLEEP_TIMER_EVT, 1000);
            // motozilog end
        } else if (pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT) {
            PRINT ("Advertising timeout..\n");
        }
        // Enable advertising
        {
            uint8_t initial_advertising_enable = TRUE;
            // Set the GAP Role Parameters
            GAPRole_SetParameter (GAPROLE_ADVERT_ENABLED, sizeof (uint8_t), &initial_advertising_enable);
        }
        break;

    case GAPROLE_ERROR:
        PRINT ("Error %x ..\n", pEvent->gap.opcode);
        break;

    default:
        break;
    }
}

/*********************************************************************
 * @fn      hidEmuRcvReport
 *
 * @brief   Process an incoming HID keyboard report.
 *
 * @param   len - Length of report.
 * @param   pData - Report data.
 *
 * @return  status
 */
static uint8_t hidEmuRcvReport (uint8_t len, uint8_t *pData) {
    // verify data length
    if (len == HID_LED_OUT_RPT_LEN) {
        // set LEDs
        return SUCCESS;
    } else {
        return ATT_ERR_INVALID_VALUE_SIZE;
    }
}

/*********************************************************************
 * @fn      hidEmuRptCB
 *
 * @brief   HID Dev report callback.
 *
 * @param   id - HID report ID.
 * @param   type - HID report type.
 * @param   uuid - attribute uuid.
 * @param   oper - operation:  read, write, etc.
 * @param   len - Length of report.
 * @param   pData - Report data.
 *
 * @return  GATT status code.
 */
static uint8_t hidEmuRptCB (uint8_t id, uint8_t type, uint16_t uuid,
                            uint8_t oper, uint16_t *pLen, uint8_t *pData) {
    uint8_t status = SUCCESS;

    // write
    if (oper == HID_DEV_OPER_WRITE) {
        if (uuid == REPORT_UUID) {
            // process write to LED output report; ignore others
            if (type == HID_REPORT_TYPE_OUTPUT) {
                status = hidEmuRcvReport (*pLen, pData);
            }
        }

        if (status == SUCCESS) {
            status = Hid_SetParameter (id, type, uuid, *pLen, pData);
        }
    }
    // read
    else if (oper == HID_DEV_OPER_READ) {
        status = Hid_GetParameter (id, type, uuid, pLen, pData);
    }
    // notifications enabled
    else if (oper == HID_DEV_OPER_ENABLE) {
        tmos_start_task (hidEmuTaskId, START_REPORT_EVT, 500);
    }
    return status;
}

/*********************************************************************
 * @fn      hidEmuEvtCB
 *
 * @brief   HID Dev event callback.
 *
 * @param   evt - event ID.
 *
 * @return  HID response code.
 */
static void hidEmuEvtCB (uint8_t evt) {
    // process enter/exit suspend or enter/exit boot mode
    return;
}

/*********************************************************************
 * @fn      GPIOA_IRQHandler
 *
 * @brief   GPIOA中断处理函数（用于唤醒清除标志）
 *
 * @return  none
 */
__INTERRUPT

__HIGH_CODE
void GPIOA_IRQHandler (void) {
    GPIOA_ClearITFlagBit (GPIO_Pin_5 | GPIO_Pin_15);
}

/*********************************************************************
*********************************************************************/
