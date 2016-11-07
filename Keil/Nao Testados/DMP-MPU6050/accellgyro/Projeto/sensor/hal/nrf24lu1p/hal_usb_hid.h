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
 * @brief Implementation of  hal_spi
 */

#include "nrf24le1.h"
#include "hal_spi.h"
#include "nordic_common.h"

void hal_spi_master_init(hal_spi_clkdivider_t ck, hal_spi_mode_t mode, hal_spi_byte_order_t bo)
{
  SPIMCON0 = 0;                           // Default register settings
  switch (ck)                             // Set desired clock divider
  {
    case SPI_CLK_DIV2:
      SPIMCON0 |= (0x00 << 4);
      break;
    case SPI_CLK_DIV4:
      SPIMCON0 |= (0x01 << 4);
      break;
    case SPI_CLK_DIV8:
      SPIMCON0 |= (0x02 << 4);
      break;
    case SPI_CLK_DIV16:
      SPIMCON0 |= (0x03 << 4);
      break;
    case SPI_CLK_DIV32:
      SPIMCON0 |= (0x04 << 4);
      break;
    case SPI_CLK_DIV64:                   // We u