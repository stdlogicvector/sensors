/*
 * sensor_ms5611.c
 *
 *  Created on: 08.04.2014
 *      Author: Konstantin
 *
 *      		Interface vor MS5611 Barometric Pressure Sensor
 */

#include "sensor_ms5611.h"

#include "lpc17xx_i2c.h"
#include "delay_module.h"

// ----------------------------------------------------------------------------

static void init_(void);
static uint8_t set_on_(void);
static uint8_t set_off_(void);
static uint8_t set_range_(uint8_t measurement, uint8_t range);
static uint8_t get_measurement_(uint8_t num);

static value_t value_[1];

// ----------------------------------------------------------------------------

static I2C_M_SETUP_Type i2c;

static int32_t sendCmd(uint8_t cmd);
static int32_t readData(uint32_t *data, uint8_t size);

// ----------------------------------------------------------------------------

static int32_t readCalibration(void);
static int32_t calcPressure(uint32_t d1, uint32_t d2);
static int32_t calcTemperature(uint32_t d2);

#define I2C_DEV	LPC_I2C0
#define SLAVE_ADDR	0b1110111

#define CMD_RESET		0x1E
#define CMD_D1_256		0x40
#define CMD_D1_512		0x42
#define CMD_D1_1024		0x44
#define CMD_D1_2048		0x46
#define CMD_D1_4096		0x48
#define CMD_D2_256		0x50
#define CMD_D2_512		0x52
#define CMD_D2_1024		0x54
#define CMD_D2_2048		0x56
#define CMD_D2_4096		0x58
#define CMD_ADC_READ	0x00
#define CMD_PROM_READ	0xA0

static uint16_t calibration[6] = {0};

// Sensor Definition ----------------------------------------------------------

#define NO_OF_MEASUREMENTS		2

static uint8_t range_[NO_OF_MEASUREMENTS] = {0};

const sensor_t sensor_ms5611 =
{
		.type = PRESSURE,
		.name = "Barometer",
		.part = "MS5611",

		.no_of_measurements = NO_OF_MEASUREMENTS,

		.measurements =
		{
			{
				.name   = "Pressure",
				.ranges = 1,
				.range  =
					{
						{ .min.u_int = 1000, .max.u_int = 120000, .digits = 0 },
						{ .min.u_int =    0, .max.u_int =      0, .digits = 0 },
						{ .min.u_int =    0, .max.u_int =      0, .digits = 0 },
						{ .min.u_int =    0, .max.u_int =      0, .digits = 0 }
					},
				.duration = 10,
				.size	= 1,
				.unit   = {
						.name = "Pascal",
						.symbol = "pa",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = KILOGRAM,.exponent = +1 },
								{ .baseunit = METER,   .exponent = -1 },
								{ .baseunit = SECOND,  .exponent = -2 },
								{ .baseunit = NONE,    .exponent = +0 }
							}
					}
			},
			{
				.name   = "Temperature",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = -40.00, .max.flt = 85.00, .digits = 2 },
						{ .min.flt =  -0.00, .max.flt =  0.00, .digits = 0 },
						{ .min.flt =  -0.00, .max.flt =  0.00, .digits = 0 },
						{ .min.flt =  -0.00, .max.flt =  0.00, .digits = 0 },
					},
				.duration = 10,
				.size	= 1,
				.unit   = {
						.name = "Celsius",
						.symbol = "\xB0\x43",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = KELVIN,  .exponent = +1 },
								{ .baseunit = NONE,    .exponent = +0 },
								{ .baseunit = NONE,    .exponent = +0 },
								{ .baseunit = NONE,    .exponent = +0 }
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

// Sensor Interface Functions -------------------------------------------------

static void init_(void)
{
	i2c.sl_addr7bit = SLAVE_ADDR;
	i2c.retransmissions_max = 1;

	sendCmd(CMD_RESET);
	delay_ms(3);

	readCalibration();
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
	if ((measurement < NO_OF_MEASUREMENTS) && (range < sensor_ms5611.measurements[measurement].ranges))
	{
		range_[measurement] = range;

		return 0;
	}

	return 1;
}

static uint8_t get_measurement_(uint8_t num)
{
	uint32_t d1, d2;

	if (num < NO_OF_MEASUREMENTS)
	{
		switch (num)
		{
		case 0:
			// Read Pressure
			sendCmd(CMD_D1_4096);
			delay_ms(10);
			sendCmd(CMD_ADC_READ);

			readData(&d1, 3);

		case 1:
			// Read Temperature
			sendCmd(CMD_D2_4096);
			delay_ms(10);
			sendCmd(CMD_ADC_READ);

			readData(&d2, 3);
		}

		if (num == 0)
			value_[0].flt = (float) calcPressure(d1, d2);
		else
			value_[0].flt = calcTemperature(d2) / 100.0f;

		return 0;
	} else
		return -1;
}


// Sensor Specific Functions -------------------------------------------------

static int32_t sendCmd(uint8_t cmd)
{
	i2c.tx_data = &cmd;
	i2c.tx_length = 1;
	i2c.rx_data = NULL;
	i2c.rx_length = 0;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
		return 0;
	else
		return -1;
}

static int32_t readData(uint32_t *data, uint8_t size)
{
	uint8_t bytes[3];

	i2c.tx_data = NULL;
	i2c.tx_length = 0;
	i2c.rx_data = bytes;
	i2c.rx_length = size;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		if (size == 3)
			*data = (bytes[0] << 16) | (bytes[1] << 8) | (bytes[2] << 0);
		else
			*data = (bytes[0] << 8) | (bytes[1] << 0);

		return 0;
	}
	else
	{
		*data = 0;
		return -1;
	}
}

static int32_t readCalibration(void)
{
	uint8_t c = 0;
	int8_t r = 0;

	for (c = 0; c < 7; c++)
	{
		r += sendCmd(CMD_PROM_READ + ((c + 1) << 1));
		r += readData((uint32_t *)&(calibration[c]), 2);
	}

	return r;
}

static int32_t calcPressure(uint32_t d1, uint32_t d2)
{
	int32_t dT  = d2 - ((uint32_t)calibration[4] << 8); // Diff between actual and reference Temperature

	int64_t	offset = ((int64_t)calibration[1] << 16) + (((int64_t)calibration[3] * dT) >> 7);
	int64_t sens   = ((int64_t)calibration[0] << 15) + (((int64_t)calibration[2] * dT) >> 8);
	int64_t tmp    = ((int64_t)d1 * (sens >> 21)) - offset;

	return ((int32_t)(tmp >> 15));
}

static int32_t calcTemperature(uint32_t d2)
{
	int32_t dT  = d2 - ((uint32_t)calibration[4] << 8); // Diff between actual and reference Temperature
	int64_t tmp = (int64_t)dT * (int64_t)calibration[5] / 8388608;

	return (2000 + (int32_t)(tmp));

	// Result jumps at (tmp>>23) = 254
}
