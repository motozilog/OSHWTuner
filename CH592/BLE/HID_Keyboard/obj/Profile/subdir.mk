################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Profile/battservice.c \
../Profile/devinfoservice.c \
../Profile/hiddev.c \
../Profile/hidkbdservice.c \
../Profile/scanparamservice.c 

C_DEPS += \
./Profile/battservice.d \
./Profile/devinfoservice.d \
./Profile/hiddev.d \
./Profile/hidkbdservice.d \
./Profile/scanparamservice.d 

OBJS += \
./Profile/battservice.o \
./Profile/devinfoservice.o \
./Profile/hiddev.o \
./Profile/hidkbdservice.o \
./Profile/scanparamservice.o 

DIR_OBJS += \
./Profile/*.o \

DIR_DEPS += \
./Profile/*.d \

DIR_EXPANDS += \
./Profile/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
Profile/%.o: ../Profile/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

