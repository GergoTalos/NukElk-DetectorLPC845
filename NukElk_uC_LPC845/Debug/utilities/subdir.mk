################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utilities/fsl_assert.c 

C_DEPS += \
./utilities/fsl_assert.d 

OBJS += \
./utilities/fsl_assert.o 


# Each subdirectory must supply rules for building sources it contributes
utilities/%.o: ../utilities/%.c utilities/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_LPC845M301JBD48 -DCPU_LPC845M301JBD48_cm0plus -DSDK_DEBUGCONSOLE=1 -DPRINTF_FLOAT_ENABLE=1 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\component\uart" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\drivers" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\CMSIS" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\CMSIS\m-profile" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\device" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\utilities\debug_console_lite" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\utilities\str" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\utilities" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\device\periph2" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\board" -I"F:\School\BME-VIK_inf\Semester_VII\NukElk\MCU\NukElk_uC_LPC845\source" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-utilities

clean-utilities:
	-$(RM) ./utilities/fsl_assert.d ./utilities/fsl_assert.o

.PHONY: clean-utilities

