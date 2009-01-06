#ifndef _CARD_SPI_INTERNALS_H_
#define _CARD_SPI_INTERNALS_H_ 1
/*
 * $Id: spi_internals.h 1 2007-01-24 13:23:49Z jos $
 *
 * libspi
 *
 * Ben Jaques <masscat@btinternet.com>
 *
 * Modifications by Jos Hendriks <jos@circuitdb.com>
 */

/** Enable the SPI controller */
#define CARD_SPI_ENABLE_BIT (1 << 15)
/** Generate an interrupt request when the transfer is complete */
#define CARD_SPI_IRQ_BIT (1 << 14)
/** The Chip Select signal for the DS Slot */
#define CARD_SPI_SLOT_CS_BIT (1 << 13)
/** The busy bit in the control register (0 idle, 1 busy) */
#define CARD_SPI_BUSY_BIT (1 << 7)
/** The Chip Select Hold bit */
#define CARD_SPI_CS_HOLD_BIT (1 << 6)


/** Test if the SPI bus is busy (0 is idle, non-zero busy)*/
#define CARD_SPI_BUSY_TEST (CARD_CR1 & CARD_SPI_BUSY_BIT)

/** Wait for the SPI bus to become idle */
#define CARD_SPI_WAIT_IDLE() while ( CARD_SPI_BUSY_TEST);

#endif /* End of _CARD_SPI_INTERNALS_H_ */
