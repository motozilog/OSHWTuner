################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP/getId.c \
../APP/hidkbd.c \
../APP/hidkbd_main.c \
../APP/hidkbd_usb.c \
../APP/usb_hid.c 

C_DEPS += \
./APP/getId.d \
./APP/hidkbd.d \
./APP/hidkbd_main.d \
./APP/hidkbd_usb.d \
./APP/usb_hid.d 

OBJS += \
./APP/getId.o \
./APP/hidkbd.o \
./APP/hidkbd_main.o \
./APP/hidkbd_usb.o \
./APP/usb_hid.o 

DIR_OBJS += \
./APP/*.o \

DIR_DEPS += \
./APP/*.d \

DIR_EXPANDS += \
./APP/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
APP/%.o: ../APP/%.c
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -DDEBUG=1 -DDEBUG_SDI=1 -I"z:/_Arcade2/CH592EVT/CH592/SRC/Startup" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/APP/include" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HID_Keyboard/Profile/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/StdPeriphDriver/inc" -I"z:/_Arcade2/CH592EVT/CH592/BLE/HAL/include" -I"z:/_Arcade2/CH592EVT/CH592/SRC/Ld" -I"z:/_Arcade2/CH592EVT/CH592/BLE/LIB" -I"z:/_Arcade2/CH592EVT/CH592/SRC/RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

