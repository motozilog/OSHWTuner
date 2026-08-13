################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
z:/_Arcade2/CH592EVT/CH592/BLE/LIB/ble_task_scheduler.S 

S_UPPER_DEPS += \
./LIB/ble_task_scheduler.d 

OBJS += \
./LIB/ble_task_scheduler.o 

DIR_OBJS += \
./LIB/*.o \

DIR_DEPS += \
./LIB/*.d \

DIR_EXPANDS += \
./LIB/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
LIB/ble_task_scheduler.o: z:/_Arcade2/CH592EVT/CH592/BLE/LIB/ble_task_scheduler.S
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

