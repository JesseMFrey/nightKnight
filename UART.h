/*
 * UART.h
 *
 *  Created on: Mar 22, 2020
 *      Author: jesse
 */

#ifndef UART_H_
#define UART_H_

//setup UCA2 in UART mode for Tx and Rx
void initUART(void);

//queue byte to get transmitted
int TxChar(unsigned char c);

//get byte from buffer
int UART_Getc (void);

//get a byte from the buffer but don't wait
int UART_CheckKey (void);


#endif /* UART_H_ */
