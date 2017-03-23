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
 * @brief Implementation of hal_clk
 */

#include "nrf24le1.h"
#include <stdbool.h>
#include "hal_clk.h"

void hal_clk_regret_xosc16m_on(bool on)
{
  if(on)
  {
    CLKCTRL = (CLKCTRL | 0x80U) & (uint8_t)~0x08U;   // & ~0x08 to prevent writing 1 to this bit
  }
  else
  {
    CLKCTRL = CLKCTRL & (uint8_t)~0x88U;            // & ~0x08 to prevent writing 1 to this bit
  }
}

void hal_clk_set_input(hal_clk_input_t input)
{
  CLKCTRL = (CLKCTRL & (