#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>


static uint8_t buf[BUFSIZE];

struct ring_buffer uart__rb;

void UART__init(void){
  cli(); // recommande dans la doc de desactiver les interruptions quand on init
  UCSR0A=_BV(U2X0);
  UCSR0B=_BV(RXEN0)|_BV(TXEN0)|_BV(RXCIE0); // enable interrupts, TX and RX
  UCSR0C=_BV(UCSZ00)|_BV(UCSZ01); // 8-bits data

  /*Set baud rate */
  UBRR0H = 0;
  UBRR0L = 16;

  ring_buffer__init(&uart__rb, buf, BUFSIZE);
  sei();
}

ISR(USART_RX_vect){
  cli();
  ring_buffer__push(&uart__rb, UDR0);
  sei();
}

void UART__putc(uint8_t data){
  while (!(UCSR0A & _BV(UDRE0))) {
    /* attente courte*/
  }
  UDR0 = data;
}
uint8_t uart_data_ready(void){
  /* s'il y a au moins 1 octet dans le ring buffer */
  return (uart__rb.head != uart__rb.tail);
}
uint8_t uart_read_from_buffer(uint8_t *data){
  /* ring_buffer__pop retourne 0 si OK, 1 si vide */
  return ring_buffer__pop(&uart__rb, data);
}
uint8_t usart_read_byte(void){
  uint8_t data;
  /* on boucle tant que le buffer est vide */
  while (ring_buffer__pop(&uart__rb, &data)) {
    
  }
  return data;
}

