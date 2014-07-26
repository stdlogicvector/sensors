################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include sensors/src/subdir.mk
-include sensors/inc/subdir.mk
-include modules/source/subdir.mk
-include LCP1768_StdPeriph_Driver/source/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: sensors.axf

# Tool invocations
sensors.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"E:\Code\LPC\Projects_1768\CMSISv2p00_LPC17xx\Debug" -Xlinker -Map="sensors.map" -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T "sensors_Debug.ld" -o "sensors.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) sensors.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "sensors.axf"; # arm-none-eabi-objcopy -O binary "sensors.axf" "sensors.bin" ; checksum -p LPC1768 -d "sensors.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
