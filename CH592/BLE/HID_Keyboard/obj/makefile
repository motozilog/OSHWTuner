################################################################################
# MRS Version: 2.4.0
# Automatically-generated file. Do not edit!
################################################################################
-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include StdPeriphDriver/subdir.mk
-include Startup/subdir.mk
-include RVMSIS/subdir.mk
-include Profile/subdir.mk
-include LIB/subdir.mk
-include HAL/subdir.mk
-include APP/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
HID_Keyboard.hex \

SECONDARY_LIST += \
HID_Keyboard.lst \

SECONDARY_SIZE += \
HID_Keyboard.siz \


# All Target
all: 
	$(MAKE) --no-print-directory main-build 

main-build: HID_Keyboard.elf secondary-outputs

# Tool invocations
HID_Keyboard.elf: $(OBJS) $(USER_OBJS_ESCAPE)
	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -g -T "z:\_Arcade2\CH592EVT\CH592\SRC\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -L"z:\_Arcade2\CH592EVT\CH592\BLE\HID_Keyboard" -L"z:\_Arcade2\CH592EVT\CH592\BLE\LIB" -L"z:\_Arcade2\CH592EVT\CH592\SRC\StdPeriphDriver" -Xlinker --print-memory-usage -Wl,-Map,"HID_Keyboard.map" --specs=nano.specs --specs=nosys.specs -o "HID_Keyboard.elf" $(OBJS) $(LIBS)
HID_Keyboard.hex: HID_Keyboard.elf
	@	riscv-none-embed-objcopy -O ihex "HID_Keyboard.elf" "HID_Keyboard.hex"
HID_Keyboard.lst: HID_Keyboard.elf
	@	riscv-none-embed-objdump --source --all-headers --demangle --line-numbers --wide "HID_Keyboard.elf" > "HID_Keyboard.lst"
HID_Keyboard.siz: HID_Keyboard.elf
	riscv-none-embed-size --format=berkeley "HID_Keyboard.elf"

# Other Targets
clean:
	-$(RM) $(DIR_OBJS) $(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE) HID_Keyboard.elf
	-$(RM) $(DIR_EXPANDS) $(CALLGRAPH_DOT)
	-$(RM) $(DIR_DEPS)
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets