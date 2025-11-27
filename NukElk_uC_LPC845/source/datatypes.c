/**
 * datatypes.c
 *
 *  Created on: 7 May 2025
 *      Author: talos
 */

#include "datatypes.h"

/**
 * The CPU is in active wait for cyc cycles. (Plus the stack and in function operations time.)
 * @param cycles unsigned int, cycles to wait
 */
void nop(unsigned int cyc) {
	for(unsigned int i=0; i < cyc; i++) {
	__asm volatile ("nop");
	}
}
