/*
 * sensor_template.c
 *
 *  Created on: 21.03.2014
 *      Author: Konstantin
 *
 *    			Template for Sensor Interface
 */

#include "sensor_template.h"

// ----------------------------------------------------------------------------

static void init_(void);
static uint8_t set_on_(void);
static uint8_t set_off_(void);
static uint8_t set_range_(uint8_t measurement, uint8_t range);
static uint8_t get_measurement_(uint8_t num);

static value_t value_[1];

// ----------------------------------------------------------------------------

	// Sensor Specific Defines & Function Prototypes

// Sensor Definition ----------------------------------------------------------

#define NO_OF_MEASUREMENTS		1

static uint8_t range_[NO_OF_MEASUREMENTS] = {0};

const sensor_t sensor_template =
{
		.name = "sensor name",
		.part = "sensor part",

		.no_of_measurements = NO_OF_MEASUREMENTS,

		.measurements =
		{
			{
				.name   = "Measurement",
				.ranges = 1,
				.range  =
					{
						{ .min.s_int = -1000, .max.s_int = 1000, .digits = 0 },
						{ .min.s_int =     0, .max.s_int =    0, .digits = 0 },
						{ .min.s_int =     0, .max.s_int =    0, .digits = 0 },
						{ .min.s_int =     0, .max.s_int =    0, .digits = 0 },
					},
				.duration = 100,
				.size	= 1,
				.type   = TYPE_INT16,
				.unit   = {
						.name = "Unit",
						.symbol = "un",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .dimension = METER,  .exponent = +1 },
								{ .dimension = SECOND, .exponent = -2 },
								{ .dimension = NONE,   .exponent = +0 },
								{ .dimension = NONE,   .exponent = +0 }
							}
					}
			}
		},

		.init = init_,

		.set_on  = set_on_,
		.set_off = set_off_,

		.set_range = set_range_,
		.range = range_,

		.get_measurement = get_measurement_,

		.value = value_
};

static void init_(void)
{

}


static uint8_t set_on_(void)
{

	return 0;
}

static uint8_t set_off_(void)
{

	return 0;
}

static uint8_t set_range_(uint8_t measurement, uint8_t range)
{
	if ((measurement < NO_OF_MEASUREMENTS) && (range < sensor_template.measurements[measurement].ranges))
	{
		range_[measurement] = range;

		return 0;
	}

	return 1;
}

static uint8_t get_measurement_(uint8_t num)
{
	if (num < NO_OF_MEASUREMENTS)
	{

		return 0;
	} else
		return 1;
}
