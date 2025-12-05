#include <uart.h>
#include <avr/io.h>
#define FOSC (16000000UL) // Clock Speed
#define BAUD (115200UL)
#define MYUBRR ((FOSC/(16*BAUD))-1)

void UART__init(void){
  UCSR0A=_BV(U2X0);
  UCSR0B=_BV(RXEN0)|_BV(TXEN0); // enable TX and RX
  UCSR0C=_BV(UCSZ00)|_BV(UCSZ01); // 8-bits data

  /*Set baud rate */
  UBRR0H=0; UBRR0L=16;
  //UBRR0H = (unsigned char)(MYUBRR>>8);
  //UBRR0L = (unsigned char)MYUBRR;
}

uint8_t UART__getc(void){
  /* Wait for data to be received */
  while ( !(UCSR0A & _BV(RXC0)) ){}
  /* Get and return received data from buffer */
  return UDR0;
}

void UART__putc(uint8_t data){
  /* Wait for empty transmit buffer */
  while ( !( UCSR0A & _BV(UDRE0)) ){}
  /* Put data into buffer, sends the data */
  UDR0 = data;
}
