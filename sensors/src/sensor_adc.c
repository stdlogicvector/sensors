/*
 * sensor_adc.c
 *
 *  Created on: 21.03.2014
 *      Author: Konstantin
 *
 *    			Interface for the on-chip ADC
 */

#include "sensor_adc.h"

#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"

// ----------------------------------------------------------------------------

static void init_(void);
static uint8_t set_on_(void);
static uint8_t set_off_(void);
static uint8_t set_range_(uint8_t measurement, uint8_t range);
static uint8_t get_measurement_(uint8_t num);

static value_t value_[1];

// ----------------------------------------------------------------------------

#define SUPPLY_VOLTAGE	3.3f
#define ADC_STEPS		4096

#define STEPS2VOLTAGE(x)	((float) x * (SUPPLY_VOLTAGE / ADC_STEPS))

// Sensor Definition ----------------------------------------------------------

#define NO_OF_MEASUREMENTS		6

static uint8_t range_[NO_OF_MEASUREMENTS] = {0};

const sensor_t sensor_adc =
{
		.type = ADC,
		.name = "AD Converter",
		.part = "LPC1768",

		.no_of_measurements = NO_OF_MEASUREMENTS,

		.measurements =
		{
			{
				.name   = "Channel 1",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = 0, .max.flt = SUPPLY_VOLTAGE, .digits = 4 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 }
					},
				.duration = 1,
				.size	= 1,
				.unit   = {
						.name = "Volt",
						.symbol = "V",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = KILOGRAM,.exponent = +1 },
								{ .baseunit = METER,   .exponent = +2 },
								{ .baseunit = SECOND,  .exponent = -3 },
								{ .baseunit = AMPERE,  .exponent = -1 }
							}
					}
			},
			{
				.name   = "Channel 2",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = 0, .max.flt = SUPPLY_VOLTAGE, .digits = 4 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 }
					},
				.duration = 1,
				.size	= 1,
				.unit   = {
						.name = "Volt",
						.symbol = "V",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = KILOGRAM,.exponent = +1 },
								{ .baseunit = METER,   .exponent = +2 },
								{ .baseunit = SECOND,  .exponent = -3 },
								{ .baseunit = AMPERE,  .exponent = -1 }
							}
					}
			},
			{
				.name   = "Channel 3",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = 0, .max.flt = SUPPLY_VOLTAGE, .digits = 4 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 }
					},
				.duration = 1,
				.size	= 1,
				.unit   = {
						.name = "Volt",
						.symbol = "V",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = KILOGRAM,.exponent = +1 },
								{ .baseunit = METER,   .exponent = +2 },
								{ .baseunit = SECOND,  .exponent = -3 },
								{ .baseunit = AMPERE,  .exponent = -1 }
							}
					}
			},
			{
				.name   = "Channel 4",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = 0, .max.flt = SUPPLY_VOLTAGE, .digits = 4 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 }
					},
				.duration = 1,
				.size	= 1,
				.unit   = {
						.name = "Volt",
						.symbol = "V",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = KILOGRAM,.exponent = +1 },
								{ .baseunit = METER,   .exponent = +2 },
								{ .baseunit = SECOND,  .exponent = -3 },
								{ .baseunit = AMPERE,  .exponent = -1 }
							}
					}
			},
			{
				.name   = "Channel 5",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = 0, .max.flt = SUPPLY_VOLTAGE, .digits = 4 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 }
					},
				.duration = 1,
				.size	= 1,
				.unit   = {
						.name = "Volt",
						.symbol = "V",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = KILOGRAM,.exponent = +1 },
								{ .baseunit = METER,   .exponent = +2 },
								{ .baseunit = SECOND,  .exponent = -3 },
								{ .baseunit = AMPERE,  .exponent = -1 }
							}
					}
			},
			{
				.name   = "Channel 6",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = 0, .max.flt = SUPPLY_VOLTAGE, .digits = 4 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 }
					},
				.duration = 1,
				.size	= 1,
				.unit   = {
						.name = "Volt",
						.symbol = "V",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = KILOGRAM,.exponent = +1 },
								{ .baseunit = METER,   .exponent = +2 },
								{ .baseunit = SECOND,  .exponent = -3 },
								{ .baseunit = AMPERE,  .exponent = -1 }
							}
					}
			}/*,
			{
				.name   = "Channel 7",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = 0, .max.flt = SUPPLY_VOLTAGE, .digits = 4 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 }
					},
				.duration = 1,
				.size	= 1,
				.unit   = {
						.name = "Volt",
						.symbol = "V",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = KILOGRAM,.exponent = +1 },
								{ .baseunit = METER,   .exponent = +2 },
								{ .baseunit = SECOND,  .exponent = -3 },
								{ .baseunit = AMPERE,  .exponent = -1 }
							}
					}
			},
			{
				.name   = "Channel 8",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = 0, .max.flt = SUPPLY_VOLTAGE, .digits = 4 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 },
						{ .min.flt = 0, .max.flt = 0, .digits = 0 }
					},
				.duration = 1,
				.size	= 1,
				.unit   = {
						.name = "Volt",
						.symbol = "V",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = KILOGRAM,.exponent = +1 },
								{ .baseunit = METER,   .exponent = +2 },
								{ .baseunit = SECOND,  .exponent = -3 },
								{ .baseunit = AMPERE,  .exponent = -1 }
							}
					}
			}
			*/
		},

		.init = init_,

		.set_on  = set_on_,
		.set_off = set_off_,

		.set_range = set_range_,
		.range = range_,

		.get_measurement = get_measurement_,

		.value = value_
};

// Sensor Interface Functions -------------------------------------------------

static void init_(void)
{
	// AD0.0 PIO0.23	F1
	// AD0.1 PIO0.24	F1
	// AD0.2 PIO0.25	F1
	// AD0.3 PIO0.26	F1

	// AD0.4 PIO1.30	F3
	// AD0.5 PIO1.31	F3

	//// AD0.6 PIO0.3	F2
	//// AD0.7 PIO0.2 	F2
	PINSEL_CFG_Type pin_config;

	pin_config.Portnum = 0;
	pin_config.Funcnum = 1;
	pin_config.Pinmode = 0; //PINSEL_PINMODE_TRISTATE;
	pin_config.OpenDrain = PINSEL_PINMODE_NORMAL;

	pin_config.Pinnum = 23;
	PINSEL_ConfigPin(&pin_config);
#pragma message "ADC CH0 on Pin : P0_23"

	pin_config.Pinnum = 24;
	PINSEL_ConfigPin(&pin_config);
#pragma message "ADC CH1 on Pin : P0_24"

	pin_config.Pinnum = 25;
	PINSEL_ConfigPin(&pin_config);
#pragma message "ADC CH2 on Pin : P0_25"

	pin_config.Pinnum = 26;
	PINSEL_ConfigPin(&pin_config);
#pragma message "ADC CH3 on Pin : P0_26"

	pin_config.Portnum = 1;
	pin_config.Funcnum = 3;

	pin_config.Pinnum = 30;
	PINSEL_ConfigPin(&pin_config);
#pragma message "ADC CH4 on Pin : P1_30"

	pin_config.Pinnum = 31;
	PINSEL_ConfigPin(&pin_config);
#pragma message "ADC CH5 on Pin : P1_31"

/*
	pin_config.Portnum = 0;
	pin_config.Funcnum = 2;

	pin_config.Pinnum = 3;
	PINSEL_ConfigPin(&pin_config);
#pragma message "ADC CH6 on Pin : P0_3"

	pin_config.Pinnum = 2;
	PINSEL_ConfigPin(&pin_config);
#pragma message "ADC CH7 on Pin : P0_2"
*/

	ADC_Init(LPC_ADC, 200000);

//	set_on_();
}

static uint8_t set_on_(void)
{
	ADC_PowerdownCmd(LPC_ADC, 1);
	return 0;
}

static uint8_t set_off_(void)
{
	ADC_PowerdownCmd(LPC_ADC, 0);
	return 0;
}

static uint8_t set_range_(uint8_t measurement, uint8_t range)
{
	if ((measurement < NO_OF_MEASUREMENTS) && (range < sensor_adc.measurements[measurement].ranges))
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
		ADC_ChannelCmd(LPC_ADC, num, ENABLE);
		ADC_StartCmd(LPC_ADC, ADC_START_NOW);

		while (!(ADC_ChannelGetStatus(LPC_ADC, num, ADC_DATA_DONE)));

		ADC_ChannelCmd(LPC_ADC, num, DISABLE);

		value_[0].flt = (float)STEPS2VOLTAGE(ADC_ChannelGetData(LPC_ADC, num));

		return 0;
	} else
		return -1;
}
