/**
 * SPI.h
 *
 *  Created on: May 2, 2023
 *      Author: benes
 */

#ifndef SPI_API_H
#define SPI_API_H
#include "fsl_spi.h"

void SPI_MasterStartTransfer(SPI_Type *base, uint8_t data);

#endif /* SPI_API_H */
