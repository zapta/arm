################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/autoisp_main.c \
../src/cr_startup_lpc11u.c \
../src/wdt.c 

OBJS += \
./src/autoisp_main.o \
./src/cr_startup_lpc11u.o \
./src/wdt.o 

C_DEPS += \
./src/autoisp_main.d \
./src/cr_startup_lpc11u.d \
./src/wdt.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__USE_CMSIS=CMSISv1p30_LPC13xx -DDEBUG -D__CODE_RED -D__REDLIB__ -I"C:\Board Code\11Uxx code\tejas lpcxpresso non rom issue\LPC11Uxx_Driver_Lib" -I"C:\Board Code\11Uxx code\tejas lpcxpresso non rom issue\CMSISv2p00_LPC11Uxx\inc" -I"C:\Board Code\11Uxx code\tejas lpcxpresso non rom issue\LPC11Uxx_Driver_Lib\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


