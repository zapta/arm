################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/clkconfig.c \
../src/gpio.c \
../src/nmi.c \
../src/system_LPC11Uxx.c \
../src/timer16.c \
../src/timer32.c \
../src/uart.c 

OBJS += \
./src/clkconfig.o \
./src/gpio.o \
./src/nmi.o \
./src/system_LPC11Uxx.o \
./src/timer16.o \
./src/timer32.o \
./src/uart.o 

C_DEPS += \
./src/clkconfig.d \
./src/gpio.d \
./src/nmi.d \
./src/system_LPC11Uxx.d \
./src/timer16.d \
./src/timer32.d \
./src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_CMSIS=CMSISv2p00_LPC11Uxx -I"C:\Board Code\11Uxx code\tejas lpcxpresso non rom issue\LPC11Uxx_Driver_Lib\inc" -I"C:\Board Code\11Uxx code\tejas lpcxpresso non rom issue\CMSISv2p00_LPC11Uxx\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


