#ifndef _UART_H_
#define _UART_H_


uint8 uart_init();

uint16 uart_write(uint8 *buf, uint16 size);

void uart_send(char *s);

void uart_sendc(char c);

void uart_flush();

uint16 uart_available();

uint16 uart_read(uint8 *dest, uint16 size);

uint16 uart_readstr(char *dest, uint16 size);

uint16 uart_readln(char *dest, uint16 size, char nl);

bool uart_requeue(uint8 *src, uint16 size);

void uart_wait();

bool uart_set_bps(uint32 bps);

void uart_set_spi_bps(uint32 bps);

void uart_set_watermarks(uint16 high, uint16 low);

float uart_get_spi_bps();

void uart_write_prio(uint8 *buf, uint16 size, uint8 *dest, uint16 irq_bytes);

bool uart_wait_prio(uint8 timeout);

uint8 uart_firmware_ver();

void uart_close();


#endif	// _UART_H_
