/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic 
 * Semiconductor ASA.Terms and conditions of usage are described in detail 
 * in NORDIC SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT. 
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRENTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *              
 * $LastChangedRevision: 133 $
 */

/** @file
* @brief Implementation of the UART HAL module for nRF24LU1+ with data buffering
*/
 
#include "nrf24lu1p.h"
#include <stdint.h>
#include "hal_uart.h"

#ifndef UART_NBUF
#define UART_NBUF   8
#endif

#define BAUD_57K6   1015  // = Round(1024 - (2*16e6)/(64*57600))
#define BAUD_38K4   1011  // = Round(1024 - (2*16e6)/(64*38400))
#define BAUD_19K2    998  // = Round(1024 - (2*16e6)/(64*19200))
#define BAUD_9K6     972  // = Round(1024 - (2*16e6)/(64*9600))

static uint8_t uart_tx_wp, uart_tx_rp, uart_tx_cnt;
static uint8_t idata uart_tx[UART_NBUF];

static uint8_t uart_rx_wp, uart_rx_rp, uart_rx_cnt;
static uint8_t idata uart_rx[UART_NBUF];

UART0_ISR()
{
  if (RI0 == 1)
  {
    RI0 = 0;
    if (uart_rx_cnt < UART_NBUF)
    {
      uart_rx[uart_rx_wp] = S0BUF;
      uart_rx_wp = (uart_rx_wp + 1) % UART_NBUF;
      uart_rx_cnt++;
    }
  }
  if (TI0 == 1)
  {
    TI0 = 0;
    if (uart_tx_cnt > 1)
    {
      S0BUF = uart_tx[uart_tx_rp];
      uart_tx_rp = (uart_tx_rp + 1) % UART_NBUF;
    }
    uart_tx_cnt--;
  }
}

void hal_uart_init(hal_uart_baudrate_t baud)
{
  uint16_t temp;

  ES0 = 0;                      // Disable UART0 interrupt while initializing
  uart_tx_wp = uart_tx_rp = 0;
  uart_tx_cnt = 0;
  uart_rx_wp = uart_rx_rp = 0;
  uart_rx_cnt = 0;
  REN0 = 1;                     // Enable receiver
  SM0 = 0;                      // Mode 1..
  SM1 = 1;                      // ..8 bit variable baud rate
  PCON |= 0x80;                 // SMOD = 1
  WDCON |= 0x80;                // Select internal baud rate generator
  switch(baud)
  {
    case UART_BAUD_57K6:
      temp = BAUD_57K6;
      break;
    case UART_BAUD_38K4:
      temp = BAUD_38K4;
      break;
    case UART_BAUD_9K6:
      temp = BAUD_9K6;
      break;
    case UART_BAUD_19K2:
    default:
      temp = BAUD_19K2;
      break;
  }
  S0RELL = (uint8_t)temp;
  S0RELH = (uint8_t)(temp >> 8);
  P0ALT |= 0x06;                // Select alternate functions on P01 and P02
  P0EXP &= 0xf0;                // Select RxD on P01 and TxD on P02
  P0DIR |= 0x02;                // P01 