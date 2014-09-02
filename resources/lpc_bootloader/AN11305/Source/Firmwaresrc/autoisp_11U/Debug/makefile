################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: autoisp.axf

# Tool invocations
autoisp.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\Board Code\11Uxx code\tejas lpcxpresso non rom issue\LPC11Uxx_Driver_Lib\Debug" -L"C:\Board Code\11Uxx code\tejas lpcxpresso non rom issue\CMSISv2p00_LPC11Uxx\Debug" -L"C:\Board Code\11Uxx code\tejas lpcxpresso non rom issue\LPC11Uxx_Driver_Lib" -Xlinker -Map=autoisp.map -Xlinker -gc-sections -mcpu=cortex-m0 -mthumb -T "autoisp_Debug.ld" -o"autoisp.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) autoisp.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size autoisp.axf; arm-none-eabi-objcopy -O binary autoisp.axf autoisp.bin ; checksum autoisp.bin
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
