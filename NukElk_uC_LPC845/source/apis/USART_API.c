/**
 * USART.c
 *
 *  Created on: Apr 14, 2023
 *      Author: benes, talos: USART_parse, artith, backtracking and loopback in GetString_TillEndChar
 * Defines USART API
 */

#include "USART_API.h"
#include "peripherals.h"
#include "fsl_usart.h"

#include "string.h"
#include "stdio.h"

 /*
  The above struct and functions used by USART:
struct _usart_handle
{
    const uint8_t *volatile txData; /!< Address of remaining data to send. /
    volatile size_t txDataSize;     /!< Size of the remaining data to send./
    size_t txDataSizeAll;           /!< Size of the data to send out.  /
    uint8_t *volatile rxData;       /!< Address of remaining data to receive. /
    volatile size_t rxDataSize;     /!< Size of the remaining data to receive. /
    size_t rxDataSizeAll;           /!< Size of the data to receive. /

    uint8_t *rxRingBuffer;              /!< Start address of the receiver ring buffer. /
    size_t rxRingBufferSize;            /!< Size of the ring buffer. /
    volatile uint16_t rxRingBufferHead; /!< Index for the driver to store received data into ring buffer. /
    volatile uint16_t rxRingBufferTail; /!< Index for the user to get data from the ring buffer. /
    usart_transfer_callback_t callback; /!< Callback function. /
    void *userData;                     /!< USART callback function parameter./

    volatile uint8_t txState; /!< TX transfer state. /
    volatile uint8_t rxState; /!< RX transfer state /
}


typedef struct _usart_transfer
{
     // Use separate TX and RX data pointer, because TX data is const data.
     // The member data is kept for backward compatibility.
     //
    union
    {
        uint8_t *data;         /!< The buffer of data to be transfer./
        uint8_t *rxData;       /!< The buffer to receive data. /
        const uint8_t *txData; /!< The buffer of data to be sent. /
    };
    size_t dataSize; /!< The byte count to be transfer. /
} usart_transfer_t;
*/

//status_t USART_TransferSendNonBlocking(USART_Type *base, usart_handle_t *handle, usart_transfer_t *xfer);
/*
void USART_TransferStartRingBuffer(USART_Type *base,
                                   usart_handle_t *handle,
                                   uint8_t *ringBuffer,
                                   size_t ringBufferSize);
*/



void INIT_USART(void)
{
	USART_TransferStartRingBuffer(USART0, &USART0_handle, &USART0_rxBuffer[0], USART0_RX_BUFFER_SIZE);
}

void PrintUSART0_NB(char * str)
{
	uint32_t count;
	status_t status;
	do{
		status = USART_TransferGetSendCount(USART0, &USART0_handle, &count);
	}while(!(status == kStatus_NoTransferInProgress)&(status == kStatus_Success));
    strcpy(USART0_txTransfer.txData,str);
    USART0_txTransfer.dataSize = strlen(str);
    USART_TransferSendNonBlocking(USART0, &USART0_handle, &USART0_txTransfer);
}

uint8_t GetUSART0(uint8_t *data)
{
    USART_DisableInterrupts(USART0, (uint32_t)kUSART_RxReadyInterruptEnable |
                                      (uint32_t)kUSART_HardwareOverRunInterruptEnable);
	uint8_t retval = 0;
	if(USART_TransferGetRxRingBufferLength(&USART0_handle) > 0)
	{
		*data = USART0_handle.rxRingBuffer[USART0_handle.rxRingBufferTail];
		USART0_handle.rxRingBufferTail = (USART0_handle.rxRingBufferTail + 1) % USART0_handle.rxRingBufferSize;
		if(USART0_handle.rxDataSize > 0) USART0_handle.rxDataSize = USART0_handle.rxDataSize - 1;
		retval = 1;
	}
    USART_EnableInterrupts(USART0,
                           (uint32_t)kUSART_RxReadyInterruptEnable | (uint32_t)kUSART_HardwareOverRunInterruptEnable);

	return(retval);
}

/**
 * Gets chars from usart0 into a string until the end character.
 * Uses states, needs to be reset at startup and after and end character is reached.
 * I only modified it, so it can backtrack and echo, since hardware echo is dodgey.
 * @param str char*, the result string
 * @param End_Char uint8_t, the ecd char
 * @param LenMax uint8_t, the length of the result buffer
 * @param reset uint8_t, to reset the state-machine
 * @return uint8_t if an error occurred
 */
uint8_t GetString_TillEndChar(char *str, uint8_t End_Char, uint8_t LenMax, uint8_t reset)
{
#define RESET_state		1
#define COLLECT_state	2
#define CR_State		3
#define END_State		4

	static uint8_t GstrState, i, data, prev_data, ret_val = 0;
	if(reset)
	{
		GstrState = RESET_state;
	}

	switch(GstrState)
	{
	case RESET_state:
		i = 0;
		ret_val = 0;
		GstrState = COLLECT_state;
		prev_data = 0;
		break;
	case COLLECT_state:
		prev_data = data;
		if(GetUSART0(&data))
		{
			if(data == ESC)
			{
				GstrState = RESET_state;
			}
			else
				if(data == End_Char)
				{
					*(str + i) = '\0'; //0
					GstrState = END_State;
					ret_val = 1;
				}
				else if((data == BS || data == DEL || (prev_data == '[' && data == 'D')) && i > 0) {
					i--;
					prev_data = 0;
					PrintUSART0_NB("\x1b[1D"); //backtrack
				}
				else
				{
					if (data != '[' && data != BS && data != DEL) {
						char lp[2];
						sprintf(lp,"%c",data);
						PrintUSART0_NB(lp); //Loopback
						*(str + i) = data;

						if(i < LenMax-1)
						{
							i++;
						}
						else
						{
							*(str + i) = 0;
							GstrState = END_State;
							ret_val = 2;  //nem fér el a stringben!
						}
					}
				}
		}
		break;
	case END_State:
		break;
	}
	return(ret_val);
}

/**
 * Parses a number from a string.
 * @param buffer char*, the buffer/string
 * @param i int8_t, the offset of parsing
 * @return double, the parsed number
 */
double arith(const char *buffer, int8_t i) {
    double res = 0;
    uint8_t fract = 0;
    int8_t sig = 1;
    if (buffer[i] == '-') { //negative
		sig = -1;
		i++;
	}
    while (buffer[i] != '\0') {
    	if (buffer[i] == '.') { //fractals
    		fract = 1;
    	}
        if (buffer[i] >= '0' && buffer[i] <= '9') {
			if (fract == 0) {
				res = res * 10 + (buffer[i] - '0');
			}
			else {
				uint8_t powfract = fract;
				double buffi = (buffer[i] - '0');
				while (powfract > 0) {
					buffi /= 10;
					powfract -= 1;
				}
				res = res + buffi;
				fract += 1;
			}
        }
        i++;
    }
    return res * sig;
}


/**
 * USART command parser
 * @param buff char*, the input buffer
 * @param set_value double*, the future value, that needs to be set to a parameter
 * @param logd_in uint8_t*, for authorization
 * @return uint8_t, the result of the parsing:
 *      + 0 Not loged in
 *      + 1 login
 *      + 2 logout
 *      + 30-38 set parameters
 *      + 254 ENTER, empty
 *      + >90, error
 */
uint8_t USART_parse(const char* buff, double* set_value, const uint8_t* const logd_in) {
	uint8_t ret = 0;
	// extract command
	char command[COMMAND_LEN+1];
	uint8_t i = 0;
	while (buff[i] != '\0' && buff[i] != ' ' && i < COMMAND_LEN) {
		command[i] = buff[i];
		i++;
	}
	command[i] = '\0';

	// what to do
	if (strcmp(command, USRT_NIX) == 0) {
			ret = 254;
		}
	else if (strcmp(command, USRT_LOGIN) == 0) { // PASSWD?
		ret = 1;
	}
	else if (strcmp(command, USRT_LOGOUT) == 0) {
		ret = 2;
	}
	else {
		if (*logd_in & 0x80) {
			if (strcmp(command, USRT_MODIFY) == 0) {
				// parse the parameter
				char para_c[PARAM_LEN+1];
				i += 1;
				for (uint8_t j = 0; buff[i] != '\0' && buff[i] != ' ' && j < PARAM_LEN; j++) {
					para_c[j] = buff[i];
					i++;
					para_c[j+1] = '\0';
				}
				*set_value = arith(buff, i+1);
				// what to do
				if (strcmp(para_c, USRT_MODIFY_CPS) == 0) {
					ret = 30;
				}
				else if (strcmp(para_c, USRT_MODIFY_MIN_CPS) == 0) {
					ret = 31;
				}
				else if (strcmp(para_c, USRT_MODIFY_MAX_CPS) == 0) {
					ret = 32;
				}
				else if (strcmp(para_c, USRT_MODIFY_OUT_V_REG) == 0) {
					ret = 33;
				}
				else if (strcmp(para_c, USRT_MODIFY_OUT_V_V) == 0) {
					ret = 34;
				}
				else if (strcmp(para_c, USRT_MODIFY_OUT_V_MA) == 0) {
					ret = 35;
				}
				else if (strcmp(para_c, USRT_MODIFY_MA_OFFS_V) == 0) {
					ret = 36;
				}
				else if (strcmp(para_c, USRT_MODIFY_MA_OFFS_D) == 0) {
					ret = 37;
				}
				else if (strcmp(para_c, USRT_MODIFY_SIREN_CPS) == 0) {
					ret = 38;
				}
				else {
					//No such parameter
					ret = 93; // error code
				}
			}
			else if (strcmp(command, USRT_EN) == 0) {
				// parse the parameter
				char para_c[PARAM_LEN+1];
				i += 1;
				for (uint8_t j = 0; buff[i] != '\0' && buff[i] != ' ' && j < PARAM_LEN; j++) {
					para_c[j] = buff[i];
					i++;
					para_c[j+1] = '\0';
				}
				// what to do
				if (strcmp(para_c, USRT_EN_CPS) == 0) {
					ret = 60;
				}
				else if (strcmp(para_c, USRT_EN_MA) == 0) {
					ret = 61;
				}
				else if (strcmp(para_c, USRT_EN_DISP) == 0) {
					ret = 62;
				}
				else if (strcmp(para_c, USRT_EN_SEG) == 0) {
					ret = 63;
				}
				else if (strcmp(para_c, USRT_EN_SIREN) == 0) {
					ret = 64;
				}/*
				else if (strcmp(para_c, USRT_EN_???) == 0) {
					ret = 65;
				}*/
				else if (strcmp(para_c, USRT_EN_MATH) == 0) {
					ret = 66;
				}
				else if (strcmp(para_c, USRT_EN_USART) == 0) {
					ret = 67;
				}
				else {
					ret = 96; // error code
				}
			}
			/*
			else if (strcmp(command, USRT_MSRWINDOW) == 0) {
				// parse the parameter
				char para_c[PARAM_LEN+1];
				i += 1;
				for (uint8_t j = 0; buff[i] != '\0' && buff[i] != ' ' && j < PARAM_LEN; j++) {
					para_c[j] = buff[i];
					i++;
					para_c[j+1] = '\0';
				}
                *set_value = arith(buff, i+1);
				// what to do
				if (strcmp(para_c, USRT_MSRWINDOW_0) == 0) {
					ret = 70;
				}
				else if (strcmp(para_c, USRT_MSRWINDOW_1) == 0) {
					ret = 71;
				}
				else if (strcmp(para_c, USRT_MSRWINDOW_2) == 0) {
					ret = 72;
				}
				else if (strcmp(para_c, USRT_MSRWINDOW_3) == 0) {
					ret = 73;
				}
				else if (strcmp(para_c, USRT_MSRWINDOW_4) == 0) {
					ret = 74;
				}
				else {
					ret = 97; // error code
				}
			}
			*/
			else {
				ret = 90; // error code
			}
		}
	}
	return ret;
}

