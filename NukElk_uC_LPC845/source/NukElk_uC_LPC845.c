/**
 * Copyright 2016-2025 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file    NukElk_uC_LPC845.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_mrt.h"
#include "fsl_power.h"
#include "math.h"

# define SYSTICK SysTick_Config(SystemCoreClock / 1000U) // Ticks every ~1ms

# include "ic_drivers/SSD1309.h"
# include "apis/BCD_API.h"
# include "apis/USART_API.h"


// USART lenghts
# define USART_IN_LEN_MAX 32
# define USART_OUT 512
# define USART_SPECIAL 50

// Default values
# define MIN_MA_OUT_V 0
# define MAX_MA_OUT_V (float)3.3
# define VREFP_MA_OUT_V (float)3.3
# define MIN_MA_CPS 0
# define MAX_MA_CPS 1024
# define SIREN_CPS 600
# define SIREN_PERCENT 8;

/* SysTick INTR */
# define LOOP_INTERVAL 10U // ms
# define MEASURE_INTERVAL 1000U // 1s
# define USART_INTERVAL 20U // ms
//# define SIREN_INTERVAL 50U // ms
# define MSR_INTERVAL_CYCLES 30000000 // 30MHz => 1s
volatile uint32_t systick_counter = LOOP_INTERVAL;
//volatile uint32_t measure_counter = MEASURE_INTERVAL;
volatile uint32_t usart_counter = USART_INTERVAL;
//volatile uint32_t siren_counter = SIREN_INTERVAL;
// Callback for Systick Interrupt
void SysTick_Handler(void) {
    if (systick_counter != 0U) {
        systick_counter--;
    }
    /*if (measure_counter != 0U) {
    	measure_counter--;
    }*/
    if (usart_counter != 0U) {
    	usart_counter--;
    }
    /*if (siren_counter != 0U) {
    	siren_counter--;
	}*/
}

/* Enable Register
 * Enables functions of the device
 * 0x01: cps reading
 * 0x02: 4-20 mA output
 * 0x04: Oled disp.
 * 0x08: 7seg disp.
 * 0x10: siren
 * 0x20: NIX
 * 0x40: math
 * 0x80: USART login granted
 */
volatile uint8_t en_reg = 0b01011110;

/* Init variable names and list in memory */
typedef enum Vars_name {
	cps = 0, min_mA_cps = 1, max_mA_cps = 2, out_value_reg = 3, out_value_V = 4, out_value_mA = 5, min_offset_V = 6, min_offset_D = 7, siren_cps = 8
} Vars_name;
# define VARS_LEN 10
volatile uint32_t vars[VARS_LEN];

// Store the errors somewhere
volatile uint8_t err = 0;

// Semafor for IRQ
volatile uint8_t signal = 0; //read_cps function always gets it. Meth fn. runs more than 1/s

// Function declarations
void read_cps(volatile uint32_t *cps);
void write_mA(volatile uint32_t *vars);
void meth(volatile uint32_t *vars);
uint8_t check_vars(volatile uint32_t *vars, double *value, Vars_name name);
uint8_t set_vars(volatile uint32_t *vars, double value, Vars_name name);
void display_texts(char* banner, char* cps_string, char* dac_string);

/* MRT0_IRQn interrupt handler, triggers every 1s, stops, then resets the CTimer. */
void MRT0_IRQHANDLER(void) {
	 __DSB();//Memory access boundary for the cpu
	CTIMER_StopTimer(CTIMER0_PERIPHERAL);
	if (en_reg & 0x01) {
		read_cps(vars+cps);
	}
	CTIMER_Reset(CTIMER0_PERIPHERAL);
	GPIO_PortToggle(BOARD_INITLEDSPINS_LED_RED_GPIO, BOARD_INITLEDSPINS_LED_RED_PORT, BOARD_INITLEDSPINS_LED_RED_PIN_MASK);
	// Clear IRQ
	CTIMER_StartTimer(CTIMER0_PERIPHERAL);
	MRT_ClearStatusFlags(MRT0_PERIPHERAL, MRT0_CHANNEL_0, kMRT_TimerInterruptFlag);
	__DSB();//Memory access boundary for the cpu
}

uint8_t set_vars(volatile uint32_t *vars, double dvalue, Vars_name name) {
	uint8_t ret = 1;
	double tmp = ((vars[max_mA_cps]-vars[min_mA_cps])/(1024-vars[min_offset_D]));
	switch(name) {
		case cps:
		case siren_cps:
		case min_mA_cps:
		case max_mA_cps:
			vars[name] = (uint32_t)(dvalue);
			ret = 0;
			break;
		case min_offset_V:
			vars[min_offset_V] = (uint32_t)dvalue;
			vars[min_offset_D] = (dvalue/100/(VREFP_MA_OUT_V/1024));
			ret = 0;
			break;
		case min_offset_D:
			vars[min_offset_D] = (uint32_t)dvalue;
			vars[min_offset_V] = floor(dvalue*(VREFP_MA_OUT_V/1024));
			ret = 0;
			break;
		case out_value_reg: //calc out_value_V and ..._D
			vars[out_value_reg] = floor(dvalue);
			dvalue *= (VREFP_MA_OUT_V/1024);
			vars[out_value_V] = floor(dvalue*100);
			vars[out_value_mA] = ceil(((dvalue-MIN_MA_OUT_V)/(MAX_MA_OUT_V-MIN_MA_OUT_V))*16+4);
			ret = 0;
			break;
		case out_value_V: //calc out_value_reg and ..._mA
			vars[out_value_V] = (uint32_t)dvalue;
			dvalue /= 100;
			vars[out_value_mA] = ceil(((dvalue-MIN_MA_OUT_V)/(MAX_MA_OUT_V-MIN_MA_OUT_V))*16+4);
			dvalue /= (VREFP_MA_OUT_V/1024);
			//double tmp = ((vars[max_mA_cps]-vars[min_mA_cps])/(1024-vars[min_offset_D]));
			vars[out_value_reg] = floor((dvalue-vars[min_offset_D])*tmp);
			ret = 0;
			break;
		case out_value_mA: //calc out_value_reg and ..._V
			vars[out_value_mA] = (uint32_t)dvalue;
			dvalue = (((dvalue-4)/16)*(MAX_MA_OUT_V-MIN_MA_OUT_V)+MIN_MA_OUT_V);
			vars[out_value_V] = floor(dvalue*100);
			dvalue /= (VREFP_MA_OUT_V/1024);
			//double tmp = ((vars[max_mA_cps]-vars[min_mA_cps])/(1024-vars[min_offset_D]));
			vars[out_value_reg] = floor(dvalue);//floor((dvalue-vars[min_offset_D])*tmp);
			ret = 0;
			break;
		default:
			break;
		}
	return ret;
}

uint8_t check_vars(volatile uint32_t *vars, double *dvalue, Vars_name name) {
	uint8_t ret = 1;
	int32_t value = *dvalue;
	switch(name) {
	case cps:
		if (value > -1) {
			ret = 0;
		}
		break;
	case min_mA_cps:
		if (value > -1 && value < vars[max_mA_cps]) {
			ret = 0;
		}
		break;
	case max_mA_cps:
		if (value > -1 && value > vars[min_mA_cps]) {
			ret = 0;
		}
		break;
	case out_value_reg:
		if (value > -1 && value < 1024) {
			ret = 0;
		}
		break;
	case out_value_V:
		if (value > -1 && value < 334) {
			ret = 0;
		}
		break;
	case out_value_mA:
		if (value > 3 && value < 21) {
			ret = 0;
		}
		break;
	case min_offset_V:
		if (value > -1 && value < 333) {
			ret = 0;
		}
		break;
	case min_offset_D:
		if (value > -1 && value < 1024) {
			ret = 0;
		}
		break;
	case siren_cps:
		if (value > -1) {
			ret = 0;
		}
		break;
	default:
		break;
	}
	return ret;
}

void meth(volatile uint32_t *vars) {
	double out_val = 0;
	Semafor_meth: // Read needed value, the rest won't affect it anyway
	if (!signal) {
		signal = 1;
		//out_val = (double)vars[cps];
		out_val = vars[cps];
		signal = 0;
	}
	else {
		goto Semafor_meth;
	}
	if (out_val > vars[siren_cps]) {
		en_reg |= 0x10; //enable siren
	}
	else {
		en_reg &= 0xEF; //disable siren
	}
	// Corrected cps for  between min and max cps with offset.
	// CAN BE OVER 1023, but the DAC's max is 1023.
	double tmp = ((vars[max_mA_cps]-vars[min_mA_cps])/(1024-vars[min_offset_D]));
	out_val = (out_val/tmp)+vars[min_offset_D];
	vars[out_value_reg] = floor(out_val);
	// Value in Volts on the pin XX with one decimal.
	out_val *= (VREFP_MA_OUT_V/1024);
	vars[out_value_V] = floor(out_val*100);
	// Value in expected mA
	vars[out_value_mA] = ceil(((out_val-MIN_MA_OUT_V)/(MAX_MA_OUT_V-MIN_MA_OUT_V))*16+4);
}

void write_mA(volatile uint32_t *vars) {
	if (vars[out_value_reg] < 1024) { //vars[max_mA_cps]
		DAC_SetBufferValue(DAC0_PERIPHERAL,vars[out_value_reg]);
	}
	else {
		DAC_SetBufferValue(DAC0_PERIPHERAL,1023);
	}
}

/**
 * Reads the number of counts counted by the timer on pin XX.
 */
void read_cps(volatile uint32_t *cps) {
	signal = 1;
	*cps = (uint32_t)CTIMER_GetTimerCountValue(CTIMER0_PERIPHERAL);
	signal = 0;
}

void display_texts(char* banner, char* cps_string, char* dac_string) {
    OLED_print_string(7,0, banner);
    OLED_print_string(7,3, cps_string);
    OLED_print_string(7,4, dac_string);
}

/**
 * @brief   Application entry point.
 */
int main(void) {
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif
    if (SYSTICK) {
		while (1) { //  If function failed infinite Error message on USART
		   PRINTF("ERROR: SysTick Init\r\n");
		}
    }

    /* Init 7segment display */
    BCD_init();
    BCD_pint2(0);

    /* Init Oled display */
    char banner[22] = "Nuclear Electronics";
    char cps_string[5] = "CPS:";
    char dac_string[7] = "DAC V:";
    OLED_init();
    display_texts(banner,cps_string,dac_string);

    /* Init USART */
	char str_in[USART_IN_LEN_MAX];
	char special[USART_SPECIAL];
	char str_out[USART_OUT];
	char usart_text[270] = "\r\nParams:\x1b[19GValue:\r\n"
"\tCPS:\r\n"
"\tmin_mA_CPS:\r\n"
"\tmax_mA_CPS:\r\n"
"\tout_val_R:\r\n"
"\tout_val_V:\r\n"
"\tout_val_mA:\r\n"
"\tmin_offset_D:\r\n"
"\tmin_offset_V:\r\n"
"\tsiren_cps:\r\n"
"Enabled:\r\n"
"\tcps_in:\r\n"
"\tmA_out:\r\n"
"\toled:\r\n"
"\t7segment:\r\n"
"\tsiren:\r\n"
"\tNIX:\r\n"
"\tmath:\r\n"
"\tUSART:\r\n";
	char usart_values [250] = "\x1b[ ? 25 l"
"\x1b[s"
"\x1b[18A"
"\x1b[25G"
"%5d"
"\x1b[1B"
"\x1b[25G"
"%5d"
"\x1b[1B"
"\x1b[25G"
"%5d"
"\x1b[1B"
"\x1b[25G"
"%5d"
"\x1b[1B"
"\x1b[25G"
"%5.2f"
"\x1b[1B"
"\x1b[25G"
"%5d"
"\x1b[1B"
"\x1b[25G"
"%5d"
"\x1b[1B"
"\x1b[25G"
"%5.2f"
"\x1b[1B"
"\x1b[25G"
"%5d"

"\x1b[2B"
"\x1b[29G"
"%1hd"
"\x1b[1B"
"\x1b[29G"
"%1hd"
"\x1b[1B"
"\x1b[29G"
"%1hd"
"\x1b[1B"
"\x1b[29G"
"%1hd"
"\x1b[1B"
"\x1b[29G"
"%1hd"
"\x1b[1B"
"\x1b[29G"
"%1hd"
"\x1b[1B"
"\x1b[29G"
"%1hd"
"\x1b[1B"
"\x1b[29G"
"%1hd"

"\x1b[u"
"\x1b[ ? 25 h";
	uint8_t usart_disp_txt = 0;
	INIT_USART();
	GetString_TillEndChar(str_in, CR_CHAR, USART_IN_LEN_MAX, 1); // reset USART

	/* Init default values of variables */
	vars[cps] = 0;
	vars[min_mA_cps] = MIN_MA_CPS;
	vars[max_mA_cps] = MAX_MA_CPS;
	vars[out_value_reg] = 0;
	vars[out_value_V] = 0;
	vars[out_value_mA] = 0;
	// Calculate from the given minimum voltage the offset of the dac's output_value
	vars[min_offset_V] = MIN_MA_OUT_V; //volts_irl*100
	vars[min_offset_D] = floor((MIN_MA_OUT_V/(VREFP_MA_OUT_V/1024))*100);
	// Sounds the siren if exceeded
	vars[siren_cps] = SIREN_CPS;

	/* Start MultiRateTimer, this generates an INTR every 1s, to accurately measure the cps */
	MRT_StartTimer(MRT0_PERIPHERAL, MRT0_CHANNEL_0, MSR_INTERVAL_CYCLES);

    /* Main Loop */
    while(1) {
        if (systick_counter == 0U) { //unifying time elapsed between loop starts
        	systick_counter = LOOP_INTERVAL; //Reset Loop interval

        	/* Perform enabled functions */
        	//if (measure_counter == 0U) {
				//measure_counter = MSR_INTERVAL;
				//if (en_reg & 0x01) {
				//	read_cps(Vars+cps);
				//}
        	//}
        	if (en_reg & 0x40) { // Calculate Other variables based on cps. Disabling only helps debug.
        		meth(vars);
        	}
			if (en_reg & 0x02) { // DAC
				POWER_DisablePD(kPDRUNCFG_PD_DAC0); // Ensure DAC is ON
				write_mA(vars); // Needs other variables too
			}
			else {
				POWER_EnablePD(kPDRUNCFG_PD_DAC0); // Ensure DAC is OFF
			}
			if (en_reg & 0x04) { // OLED
				OLED_print_int(37,3, vars[cps]); //(signed long)
				float tmp = vars[out_value_V]/100;
				OLED_print_float(37,4, tmp,2); //(signed long)
			}
			else {
				OLED_clear_screen();
			}
			if (en_reg & 0x08) { // 7Seg
				BCD_pint2(20 < vars[out_value_mA] ? 20 : vars[out_value_mA]); // "20mA" (1023, or 3.3V) is the max on the DAC, therefore 20mA is the max here
			}
			else {
				BCD_blank();
			}
        	/* Enable Siren */
        	GPIO_PinWrite(SIREN_T_GPIO,SIREN_T_PORT,SIREN_T_PIN,(en_reg & 0x10) >> 4); //Shift, 'cause only accepts 0 and 1.
			/*if (en_reg & 0x10) {
				if (siren_counter == 0U) {
					SCTIMER_UpdatePwmDutycycle(SCT0_PERIPHERAL, kSCTIMER_Out_3, siren_percent, SCT0_pwmEvent[1]);
					siren_percent ^= 0x02;
				}
				else { // Turn siren off
					SCTIMER_UpdatePwmDutycycle(SCT0_PERIPHERAL, kSCTIMER_Out_3, 0, SCT0_pwmEvent[1]);
				}
				siren_counter = LOOP_INTERVAL; //Reset Loop interval
        	}
			*/
			/* Check USART, when SysTick counter usart is zero */
			if (usart_counter == 0U) {
				/* Process USART */
				if (GetString_TillEndChar(str_in, CR_CHAR, USART_IN_LEN_MAX, 0)) {
					GetString_TillEndChar(str_in, CR_CHAR, USART_IN_LEN_MAX, 1);
					double new_value = 0; //future place for a parameter's new value. (Double, 'cause it can parse fractions. A bit overkill for this, but I am somewhat pleased with the "arith" part.)
					uint8_t code = USART_parse(str_in, &new_value, &en_reg); //Parse valid USART commands.
					switch (code) {
						case 0:
						{
							strcpy(special,"\r\n\t***Not logged in!***\r\n\r\n");
						}
						break;
						case 1:
						{
							en_reg |= 0x80;
							usart_disp_txt = 1;
							strcpy(special,"\r\n\t***Logged in!***\r\n\r\n");
						}
						break;
						case 2:
						{
							en_reg &= ~0x80;
							usart_disp_txt = 0;
							strcpy(special,"\r\n\t***Logged out!***\r\n\r\n");
						}
						break;
						// errors
						case 90:
						{
							strcpy(special,"\r\n\tNo such command!\r\n\r\n");
						}
						break;
						case 93:
						{
							strcpy(special,"\r\n\tNo such variable!\r\n\r\n");
						}
						break;
						case 96:
						{
							strcpy(special,"\r\n\tNo such function!\r\n\r\n");
						}
						break;
						case 254:
							strcpy(special,"\r\n\r\n\r\n");
							break;
						default:
							if (code > 89) { // Undefined command error
								strcpy(special,"\r\n\tERROR: Not recognised USART code!\r\n\r\n");
							}
							else if (code > 59) { // Enable or disable a function
								en_reg ^= (0x01 << (code-60)); // Toggles a function
								if (code == 62) { // On oled enable
								    display_texts(banner, cps_string, dac_string);
								}
								strcpy(special,"\r\n\tFunction has been dis/enabled!\r\n\r\n");
							}
							else if (code > 29) { //set a parameter
								code -= 30;
								if (!check_vars(vars, &new_value, code)) { //Checks validity
									set_vars(vars, new_value, code);
									//vars[code] = (uint32_t)new_value; // sets new value
									strcpy(special,"\r\n\tParameter has been set!\r\n\r\n");
								}
								else {
									strcpy(special,"\r\n\tInvalid or no parameter value!\r\n\r\n");
								}
							}
						break;
					}
					usart_disp_txt = 1;
				}
				usart_counter = USART_INTERVAL; // Reset upon finished round
			}
        	//loop continuation
        	/* Display USART */
        	if (en_reg & 0x80) {
				if (usart_disp_txt) {
					// Go to 0x0y and clear screen
					sprintf(str_out,"\x1b[%d;%df\x1b[2J", 0,0);
					PrintUSART0_NB(str_out);
					// Print special message
					//sprintf(str_out,"\r\n%s\r\n",special);
					PrintUSART0_NB(special);//
					special[0] = '\0'; // clear
					// Print parameters
					/*sprintf(str_out,
							"\r\nParams:\x1b[19GValue:\r\n"
							"\tCPS:\r\n"
							//"\x1b[25G"
							//"%5d\r\n"
							"\tmin_mA_CPS:\r\n"
							//"\x1b[25G"
							//"%5d\r\n"
							"\tmax_mA_CPS:\r\n"
							//"\x1b[25G"
							//"%5d\r\n"
							"\tout_val_R:\r\n"
							//"\x1b[25G"
							//"%5d\r\n"
							"\tout_val_V:\r\n"
							//"\x1b[25G"
							//"%5d\r\n"
							"\tout_val_mA:\r\n"
							//"\x1b[25G"
							//"%5d\r\n"
							"\tmin_offset_V:\r\n"
							//"\x1b[25G"
							//"%5d\r\n"
							"\tmin_offset_D:\r\n"
							//"\x1b[25G"
							//"%5d\r\n"
							"\tsiren_cps:\r\n"
							//"\x1b[25G"
							//"%5d\r\n"
							"Enabled:\r\n"
							"\tcps_in:\r\n"
							"\tmA_out:\r\n"
							"\toled:\r\n"
							"\t7segment:\r\n"
							"\tsiren:\r\n"
							"\tUSART:\r\n"
							//,
							//vars[cps],vars[min_mA_cps],vars[max_mA_cps],vars[out_value_reg], vars[out_value_V]/100,vars[out_value_mA],vars[min_offset_V]/100,vars[min_offset_D],vars[siren_cps]
							);*/
					PrintUSART0_NB(usart_text);//str_out
					usart_disp_txt = 0;
				}
				//else {
				float tmp0 = (float)vars[out_value_V]/100;
				float tmp1 = (float)vars[min_offset_V]/100;
				PrintUSART0_NB("\x1b[60"); // are you there?
				// Print special message
				//sprintf(str_out,"\r\n%s\r\n",special);
				//PrintUSART0_NB(str_out);
				//special[0] = '\0'; // clear

				sprintf(str_out,
						usart_values
				/*	"\x1b[ ? 25 l"
					"\x1b[s"
					"\x1b[16A"
						"\x1b[25G"
						"%5d"
					"\x1b[1B"
						"\x1b[25G"
						"%5d"
					"\x1b[1B"
						"\x1b[25G"
						"%5d"
					"\x1b[1B"
						"\x1b[25G"
						"%5d"
					"\x1b[1B"
						"\x1b[25G"
						"%5.2f"
					"\x1b[1B"
						"\x1b[25G"
						"%5d"
					"\x1b[1B"
						"\x1b[25G"
						"%5.2f"
					"\x1b[1B"
						"\x1b[25G"
						"%5d"
					"\x1b[1B"
						"\x1b[25G"
						"%5d"

					"\x1b[2B"
						"\x1b[29G"
						"%1hd"
					"\x1b[1B"
						"\x1b[29G"
						"%1hd"
					"\x1b[1B"
						"\x1b[29G"
						"%1hd"
					"\x1b[1B"
						"\x1b[29G"
						"%1hd"
					"\x1b[1B"
						"\x1b[29G"
						"%1hd"
					"\x1b[1B"
						"\x1b[29G"
						"%1hd"

					"\x1b[u"
					"\x1b[ ? 25 h"
					*/
					,
					vars[cps],
					vars[min_mA_cps],
					vars[max_mA_cps],
					vars[out_value_reg],
					tmp0,
					vars[out_value_mA],
					vars[min_offset_D],
					tmp1,
					vars[siren_cps],
					(en_reg & 0x01),
					((en_reg & 0x02) >> 1),
					((en_reg & 0x04) >> 2),
					((en_reg & 0x08) >> 3),
					((en_reg & 0x10) >> 4),
					((en_reg & 0x20) >> 5),
					((en_reg & 0x40) >> 6),
					((en_reg & 0x80) >> 7)
						 );
				PrintUSART0_NB(str_out);
				//}
        	}
        	//loop cont.
        }
    }
    return 0 ;
}
