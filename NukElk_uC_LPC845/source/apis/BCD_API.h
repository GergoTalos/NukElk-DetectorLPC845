/*
 * BCD_API.h
 *
 *  Created on: 20 Nov 2025
 *      Author: talos
 */

#ifndef APIS_BCD_API_H_
#define APIS_BCD_API_H_

#include "stdint.h"

/**
 * Lower 4 bits are used.
 */
uint8_t BCD_digit(uint8_t digit);

/**
 * int to bcd for two digit, positive only displays
 */
uint8_t BCD_pint2(int8_t num);

/**
 *
 */
void BCD_init(void);

/**
 * Blanks all the digits
 */
void BCD_blank(void);

/**
 * Rising edge
 */
void BCD_cnt_up(void);

/**
 * Rising edge
 */
void BCD_cnt_dw(void);

/**
 * Rising edge
 */
void BCD_clr(void);

#endif /* APIS_BCD_API_H_ */
