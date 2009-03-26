#ifndef _CARD_SPI_DRIVER_H_
#define _CARD_SPI_DRIVER_H_ 1
/*
 * $Id: spi.h 1 2007-01-24 13:23:49Z jos $
 *
 * libspi
 *
 * Ben Jaques <masscat@btinternet.com>
 *
 * Modifications by Jos Hendriks <jos@circuitdb.com>
 */

/** \brief The driver function return codes.
 */
enum card_spi_return_code {
  /** All was well */
  CARD_SPI_OK = 0,

  /** The SPI bus is busy */
  CARD_SPI_BUSY = 1
};

/** \brief The SPI bus clock frequencies.
 */
enum card_spi_clock_freq {
  /** 4 MHz */
  CARD_SPI_4_MHZ_CLOCK = 0,
  /** 2 MHz */
  CARD_SPI_2_MHZ_CLOCK = 1,
  /** 1 MHz */
  CARD_SPI_1_MHZ_CLOCK = 2,
  /** 524 KHz */
  CARD_SPI_524_KHZ_CLOCK = 3
};


/*
 * The function interface
 */
#ifdef __cplusplus
extern "C" {
#endif

/** \brief Initialise the SPI driver.
 */
void
init_cardSPI( void);


/** \brief Disable the Card SPI bus.
 *
 * Disables the Card SPI bus. A write call will enable the bus again.
 */
void
disable_cardSPI( void);


/** \brief Set the configuration of the Card SPI bus.
 *
 */
void
config_cardSPI( enum card_spi_clock_freq freq, int use_interrupts);


/** \brief Set up a transfer of a number of consecutive bytes.
 *
 * The chip select will remain low (active) until the supplied
 * number of bytes have been sent out.
 */
void
setupConsecutive_cardSPI( uint32_t num_bytes);


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
read_cardSPI( uint8_t *read_byte);

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
readBlocking_cardSPI( uint8_t *read_byte);

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
write_cardSPI( uint8_t byte);

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
writeBlocking_cardSPI( uint8_t byte);

#ifdef __cplusplus
};
#endif

#endif /* End of _CARD_SPI_DRIVER_H_ */
