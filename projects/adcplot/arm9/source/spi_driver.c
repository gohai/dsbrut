/*
 * $Id: spi_driver.c 1 2007-01-24 13:23:49Z jos $
 *
 * libspi
 *
 * Ben Jaques <masscat@btinternet.com>
 *
 * Modifications by Jos Hendriks <jos@circuitdb.com>
 */
#include <stdint.h>

#include <nds.h>

#include "spi.h"
#include "spi_internals.h"

/** \brief A SPI driver descriptor.
 *
 */
struct spi_driver {
  /** The size of the block transfer */
  uint32_t block_size;

  /** The SPI bus control bits */
  uint16_t config;
};


/** \brief The SPI driver instance.
 */
static struct spi_driver driver;


/** \brief Initialise the SPI driver.
 *
 * Initialise the Card SPI driver.
 */
void
init_cardSPI( void) {
  disable_cardSPI();

  /* no block transfer in progress */
  driver.block_size = 0;

  /* no configuration yet */
  driver.config = 0;
}


/** \brief Read from the card SPI.
 *
 * If the SPI bus is busy then the read will fail. Otherwise the last
 * received byte is returned.
 *
 * \param read_byte Pointer to the location to store the received byte.
 *
 * \return CARD_SPI_OK if the read was successful or CARD_SPI_BUSY if a transfer
 * is in progress.
 */
enum card_spi_return_code
read_cardSPI( uint8_t *read_byte) {
  enum card_spi_return_code ret_code = CARD_SPI_BUSY;

  if ( !CARD_SPI_BUSY_TEST) {
    *read_byte = CARD_EEPDATA;
    ret_code = CARD_SPI_OK;
  }

  return ret_code;
}


/** \brief Read from the card SPI blocking if it is busy.
 *
 * Block until any outstanding transfer has completed and then
 * return the byte receive over the SPI bus.
 * The call is always successful, the function parameters and return
 * type is kept in the same format as the non blocking function.
 *
 * \param read_byte Pointer to the location to store the received byte.
 *
 * \return CARD_SPI_OK if the read was successful.
 */
enum card_spi_return_code
readBlocking_cardSPI( uint8_t *read_byte) {
  CARD_SPI_WAIT_IDLE();

  *read_byte = CARD_EEPDATA;

  return CARD_SPI_OK;
}


/** \brief Write a byte out on the SPI bus (initiates a transfer)
 *
 * Writes the supplied byte out on the SPI bus. The write initiates a
 * transfer which once completed will mean the next received byte is
 * available.
 *
 * \param byte The byte value to be sent out.
 *
 * \return CARD_SPI_OK if the write was successful or CARD_SPI_BUSY if
 * the SPI bus is busy.
 */
enum card_spi_return_code
write_cardSPI( uint8_t byte) {
  enum card_spi_return_code ret_code = CARD_SPI_BUSY;

  if ( !CARD_SPI_BUSY_TEST) {
    if ( driver.block_size > 1) {
      /* hold the chip select low after the transfer */
      driver.block_size -= 1;

      CARD_CR1 = driver.config | CARD_SPI_CS_HOLD_BIT;
    }
    else {
      /* the chip select will be set high after the transfer */
      CARD_CR1 = driver.config;
    }

    CARD_EEPDATA = byte;
    ret_code = CARD_SPI_OK;
  }

  return ret_code;
}


/** \brief Write a byte out on the SPI bus (initiates a transfer) blocking
 * until the write can be performed.
 *
 * Block until any outstanding transfer has completed and then
 * send the byte over the SPI bus.
 * The call is always successful, the function parameters and return
 * type is kept in the same format as the non blocking function.
 *
 * \param byte The byte value to be sent
 *
 * \return CARD_SPI_OK if the read was successful.
 */
enum card_spi_return_code
writeBlocking_cardSPI( uint8_t byte) {
  while ( write_cardSPI( byte) != CARD_SPI_OK);

  return CARD_SPI_OK;
}


/** \brief Set up a transfer of a number of consecutive bytes.
 *
 * The chip select will remain low (active) until the supplied
 * number of bytes have been sent out.
 */
void
setupConsecutive_cardSPI( uint32_t num_bytes) {
  driver.block_size = num_bytes;
}


/** \brief Set the configuration of the Card SPI bus.
 *
 */
void
config_cardSPI( enum card_spi_clock_freq freq, int use_interrupts) {
  driver.config = CARD_SPI_ENABLE_BIT;

  if ( use_interrupts) {
    driver.config |= CARD_SPI_IRQ_BIT;
  }

 // if ( freq <= CARD_SPI_524_KHZ_CLOCK) {
    driver.config |= freq;
 // }

  driver.config |= CARD_SPI_SLOT_CS_BIT;

  driver.block_size = 0;
}

/** \brief Disable the Card SPI bus.
 *
 * Disables the Card SPI bus. A write call will enable the bus again.
 */
void
disable_cardSPI( void) {
  CARD_CR1 = 0;
}
