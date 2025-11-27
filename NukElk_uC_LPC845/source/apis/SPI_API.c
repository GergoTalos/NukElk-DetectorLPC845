/**
 * SPI.c
 *
 *  Created on: May 2, 2023
 *      Author: benes
 */
#include "SPI_API.h"

#define BUFFER_SIZE (1)
static uint8_t txBuffer[BUFFER_SIZE];
static uint8_t rxBuffer[BUFFER_SIZE];

void SPI_MasterStartTransfer(SPI_Type *base, uint8_t data)
{
    spi_transfer_t xfer = {0};

    /* Init Buffer*/

        txBuffer[0] = data;
        rxBuffer[0] = 0U;

    /*Start Transfer*/
    xfer.txData      = txBuffer;
    xfer.rxData      = rxBuffer;
    xfer.dataSize    = sizeof(txBuffer);
    xfer.configFlags = kSPI_EndOfTransfer | kSPI_EndOfFrame;
    /* Transfer data in polling mode. */
    SPI_MasterTransferBlocking(base, &xfer);
}
