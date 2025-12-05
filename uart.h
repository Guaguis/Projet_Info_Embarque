#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "ring_buffer.h"

#define BUFSIZE 41

void UART__init(void);

/* Envoie 1 octet sur l’UART */
void UART__putc(uint8_t data);

/* Indique s’il y a au moins 1 octet dans le buffer RX */
uint8_t uart_data_ready(void);

/* Lit 1 octet du buffer RX (non bloquant)
 *  retourne 0 si OK, 1 si buffer vide
 */
uint8_t uart_read_from_buffer(uint8_t *data);

/* Version bloquante : attend jusqu'à avoir un octet */
uint8_t usart_read_byte(void);

#endif // UART_H
