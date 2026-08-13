################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_adc.c \
z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_clk.c \
z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_flash.c \
z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_gpio.c \
z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_i2c.c \
z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_lcd.c \
z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_pwr.c \
z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_sys.c \
z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_uart1.c 

C_DEPS += \
./StdPeriphDriver/CH59x_adc.d \
./StdPeriphDriver/CH59x_clk.d \
./StdPeriphDriver/CH59x_flash.d \
./StdPeriphDriver/CH59x_gpio.d \
./StdPeriphDriver/CH59x_i2c.d \
./StdPeriphDriver/CH59x_lcd.d \
./StdPeriphDriver/CH59x_pwr.d \
./StdPeriphDriver/CH59x_sys.d \
./StdPeriphDriver/CH59x_uart1.d 

OBJS += \
./StdPeriphDriver/CH59x_adc.o \
./StdPeriphDriver/CH59x_clk.o \
./StdPeriphDriver/CH59x_flash.o \
./StdPeriphDriver/CH59x_gpio.o \
./StdPeriphDriver/CH59x_i2c.o \
./StdPeriphDriver/CH59x_lcd.o \
./StdPeriphDriver/CH59x_pwr.o \
./StdPeriphDriver/CH59x_sys.o \
./StdPeriphDriver/CH59x_uart1.o 

DIR_OBJS += \
./StdPeriphDriver/*.o \

DIR_DEPS += \
./StdPeriphDriver/*.d \

DIR_EXPANDS += \
./StdPeriphDriver/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
StdPeriphDriver/CH59x_adc.o: z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_adc.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH59x_clk.o: z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_clk.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH59x_flash.o: z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_flash.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH59x_gpio.o: z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_gpio.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH59x_i2c.o: z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_i2c.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH59x_lcd.o: z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_lcd.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH59x_pwr.o: z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_pwr.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH59x_sys.o: z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_sys.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
StdPeriphDriver/CH59x_uart1.o: z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/CH59x_uart1.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

