################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/blinky.c \
../src/cr_startup_lpc11u.c 

OBJS += \
./src/blinky.o \
./src/cr_startup_lpc11u.o 

C_DEPS += \
./src/blinky.d \
./src/cr_startup_lpc11u.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_CMSIS=CMSISv2p00_LPC11Uxx -I"C:\Board Code\11Uxx code\for_blinky_AN_crp\LPC11Uxx_Driver_Lib" -I"C:\Board Code\11Uxx code\for_blinky_AN_crp\CMSISv2p00_LPC11Uxx\inc" -I"C:\Board Code\11Uxx code\for_blinky_AN_crp\LPC11Uxx_Driver_Lib\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/cr_startup_lpc11u.o: ../src/cr_startup_lpc11u.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -D__USE_CMSIS=CMSISv2p00_LPC11Uxx -I"C:\Board Code\11Uxx code\for_blinky_AN_crp\LPC11Uxx_Driver_Lib" -I"C:\Board Code\11Uxx code\for_blinky_AN_crp\CMSISv2p00_LPC11Uxx\inc" -I"C:\Board Code\11Uxx code\for_blinky_AN_crp\LPC11Uxx_Driver_Lib\inc" -Os -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/cr_startup_lpc11u.d" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


