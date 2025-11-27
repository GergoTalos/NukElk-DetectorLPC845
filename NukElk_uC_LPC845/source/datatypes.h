/**
 * datatypes.h
 *
 *  Created on: 4 Apr 2025
 *      Author: talos
 *  Define enumerations for usage in multiple components.
 */

#ifndef DATATYPES_H_
#define DATATYPES_H_


#define EEPROM_WAIT 6000 //cycles

/**
 * The CPU is in active wait for cyc cycles. (Plus the stack and in function operations time.)
 * @param cyc unsigned int, cycles to wait
 */
void nop(unsigned int cyc);

#endif /* DATATYPES_H_ */
