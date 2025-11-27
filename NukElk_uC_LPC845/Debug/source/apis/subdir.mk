################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/apis/BCD_API.c \
../source/apis/SPI_API.c \
../source/apis/USART_API.c 

C_DEPS += \
./source/apis/BCD_API.d \
./source/apis/SPI_API.d \
./source/apis/USART_API.d 

OBJS += \
./source/apis/BCD_API.o \
./source/apis/SPI_API.o \
./source/apis/USART_API.o 


# Each subdirectory must supply rules for building sources it contributes
source/apis/%.o: ../source/apis/%.c source/apis/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_LPC845M301JBD48 -DCPU_LPC845M301JBD48_cm0plus -DSDK_DEBUGCONSOLE=1 -DPRINTF_FLOAT_ENABLE=1 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\component\uart" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\drivers" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\CMSIS" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\CMSIS\m-profile" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\device" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\utilities\debug_console_lite" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\utilities\str" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\utilities" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\device\periph2" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\board" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\source" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source-2f-apis

clean-source-2f-apis:
	-$(RM) ./source/apis/BCD_API.d ./source/apis/BCD_API.o ./source/apis/SPI_API.d ./source/apis/SPI_API.o ./source/apis/USART_API.d ./source/apis/USART_API.o

.PHONY: clean-source-2f-apis

