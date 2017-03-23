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
 * $LastChangedRevision: 2513 $
 */

/** @file
 * @brief Interface for clock management.
 * @defgroup hal_nrf24le1_hal_clk Clock control (hal_clk)
 * @{
 * @ingroup hal_nrf24le1
 *
 * The clock to the MCU is sourced from either an on-chip RC oscillator
 * or an external crystal oscillator. This module contains functions for selecting
 * clock source and clock frequency.
 */

#ifndef HAL_CLK_H__
#define HAL_CLK_H__

#include <stdint.h>
#include <stdbool.h>

/** An enum describing the possible system clock inputs.
 *
 */
typedef enum
{
  HAL_CLK_XOSC16_OR_RCOSC16 = 0,
  HAL_CLK_PAD_XC1 = 1
} hal_clk_input_t;

/** An enum used for