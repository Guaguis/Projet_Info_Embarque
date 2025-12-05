#ifndef UART_H
#define UART_H

#include <stdint.h>

void UART__init(void);
uint8_t UART__getc(void);
void UART__putc(uint8_t);

#endif // UART_H
