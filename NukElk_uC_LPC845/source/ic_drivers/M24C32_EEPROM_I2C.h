/**
 * M24C32_EEPROM_I2C.h
 *
 *  Created on: 4 May 2025
 *      Author: talos
 *  M24C32 I2C EEPROM function declarations.
 *  32Kbit; 4Kbytes; one page is 32 bytes.
 *  read & write time is:
 *  	 10ms, when Vcc < 1.7V
 *  	 5ms, when Vcc >= 1.7V
 *
 * M24C32 EEPROM default ADDRESSES
 * Last 3 bits are the E1, E2, E3 pins
 * Memory array: 0x50; 0b1010000
 * Identification page (...-D variant only): 0x58; 0b1011000
 */

#ifndef IC_DRIVERS_M24C32_EEPROM_I2C_H_
#define IC_DRIVERS_M24C32_EEPROM_I2C_H_

#include "stdint.h"

/**
 * Write a single byte
 * @param slave_addr
 * @param byte_msB_addr
 * @param byte_lsB_addr
 * @param data
 * @return
 */
uint8_t M24C32_byte_write(const uint8_t slave_addr, const uint8_t byte_msB_addr, const uint8_t byte_lsB_addr, const uint8_t *data);

/**
 * Write 1-32 bytes to a page (b16-b5 are the same)
 * @param slave_addr
 * @param byte_msB_addr
 * @param byte_lsB_addr
 * @param data
 * @param data_len
 * @return
 */
uint8_t M24C32_page_write(const uint8_t slave_addr, const uint8_t byte_msB_addr, const uint8_t byte_lsB_addr, const uint8_t *data, const uint8_t data_len);

/**
 * Read a byte from the current (internal) position
 * @param slave_addr
 * @param data
 * @return
 */
uint8_t M24C32_curr_addr_read(const uint8_t slave_addr, uint8_t *data);

/**
 * Read a byte from a given address
 * @param slave_addr
 * @param byte_msB_addr
 * @param byte_lsB_addr
 * @param data
 * @return
 */
uint8_t M24C32_rand_addr_read(const uint8_t slave_addr, const uint8_t byte_msB_addr, const uint8_t byte_lsB_addr, uint8_t *data);

/**
 * Sequential read. Start from current (internal) position
 * @param slave_addr
 * @param data
 * @param data_len
 * @return
 */
uint8_t M24C32_seq_curr_addr_read(const uint8_t slave_addr, uint8_t *data, const uint8_t data_len);

/**
 * Sequential read. Starts from a given address
 * @param slave_addr
 * @param byte_msB_addr
 * @param byte_lsB_addr
 * @param data
 * @param data_len
 * @return
 */
uint8_t M24C32_seq_rand_addr_read(const uint8_t slave_addr, const uint8_t byte_msB_addr, const uint8_t byte_lsB_addr, uint8_t *data, const uint8_t data_len);

#endif /* IC_DRIVERS_M24C32_EEPROM_I2C_H_ */
