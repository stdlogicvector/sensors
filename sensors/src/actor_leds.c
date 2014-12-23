/*
 * actor_leds.c
 *
 *  Created on: 21.04.2014
 *      Author: Konstantin
 */

#include "actor_leds.h"
#include "lpc17xx_gpio.h"

// ----------------------------------------------------------------------------

static void init_(void);
static uint8_t set_on_(void);
static uint8_t set_off_(void);
static uint8_t set_action_(uint8_t num);

static value_t value_[1];

// ----------------------------------------------------------------------------

// Sensor Definition ----------------------------------------------------------

#define NO_OF_ACTIONS		2

const actor_t actor_leds =
{
		.name = "LEDs",
		.part = "Red LED",

		.no_of_actions = 2,

		.actions =
		{
			{
				.name  = "LED 1",
				.range = { .min.u_int = 0, .max.u_int = 1, .digits = 0 },
				.size  = 1,
				.unit   = {
						.name = " ",
						.symbol = " ",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = NONE, .exponent = +0 },
								{ .baseunit = NONE, .exponent = +0 },
								{ .baseunit = NONE, .exponent = +0 },
								{ .baseunit = NONE, .exponent = +0 }
							}
					}
			},
			{
				.name  = "LED 2",
				.range = { .min.u_int = 0, .max.u_int = 1, .digits = 0 },
				.size  = 1,
				.unit   = {
						.name = " ",
						.symbol = " ",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = NONE, .exponent = +0 },
								{ .baseunit = NONE, .exponent = +0 },
								{ .baseunit = NONE, .exponent = +0 },
								{ .baseunit = NONE, .exponent = +0 }
							}
					}
			}
		}
};
