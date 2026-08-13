################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS/core_riscv.c 

C_DEPS += \
./RVMSIS/core_riscv.d 

OBJS += \
./RVMSIS/core_riscv.o 

DIR_OBJS += \
./RVMSIS/*.o \

DIR_DEPS += \
./RVMSIS/*.d \

DIR_EXPANDS += \
./RVMSIS/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
RVMSIS/core_riscv.o: z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS/core_riscv.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

