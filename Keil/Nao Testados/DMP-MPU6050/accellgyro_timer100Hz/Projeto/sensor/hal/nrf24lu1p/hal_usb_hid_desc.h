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
 * $LastChangedRevision: 167 $
 */

/** @file
 * @brief Interface functions for the Serial Peripheral Interface (SPI).
 *
 * @defgroup hal_nrf24le1_hal_spi Serial Peripheral Interface (hal_spi)
 * @{
 * @ingroup hal_nrf24le1
 * 
 * The Serial Peripheral Interface (SPI) is double buffered and can be configured to 
 * work in all four SPI modes. The default is mode 0.
 *
 * The SPI connects to the following pins of the device: MMISO, MMOSI, MSCK.
 * The S