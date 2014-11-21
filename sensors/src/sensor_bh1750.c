/*
 * sensor_bh1750.c
 *
 *  Created on: 21.03.2014
 *      Author: Konstantin
 *
 *    			Interface for BH1750 Light Sensor
 */

#include "sensor_bh1750.h"

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
static int32_t readData(uint16_t *data);

// ----------------------------------------------------------------------------

#define BH1750_ADDR		0
#define I2C_DEV	LPC_I2C0

#if (BH1750_ADDR == 0)
#	define SLAVE_ADDR	0b0100011
#else
#	define SLAVE_ADDR	0b1011100
#endif

#define CMD_PWR_DOWN	0b00000000
#define CMD_PWR_ON		0b00000001
#define CMD_RESET		0b00000111
#define CMD_CONT_H		0b00010000	// 1lx resolution, 120ms Sample Time
#define CMD_CONT_H2		0b00010001	// 0.5lx resolution, 120ms Sample Time
#define CMD_SNGL_H		0b00100000	// 1lx resolution, 120ms Sample Time, PowerDown automatically
#define CMD_SNGL_H2		0b00100001	// 0.5lx resolution, 120ms Sample Time, PowerDown automatically
#define CMD_SNGL_L		0b00100011	// 4lx resolution, 16ms Sample Time, PowerDown automatically
#define CMD_CNG_MT_H	0b01000000
#define CMD_CNG_MT_L	0b01100000

// Sensor Definition ----------------------------------------------------------

#define NO_OF_MEASUREMENTS		1

static uint8_t range_[NO_OF_MEASUREMENTS] = {0};

const sensor_t sensor_bh1750 =
{
		.type = LIGHT,
		.name = "Light Sensor",
		.part = "BH1750",

		.no_of_measurements = NO_OF_MEASUREMENTS,

		.measurements =
		{
			{
				.name   = "Illuminance",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = 0.8f, .max.flt = 54613.0f, .digits = 1 },
						{ .min.flt = 0.0f, .max.flt =     0.0f, .digits = 0 },
						{ .min.flt = 0.0f, .max.flt =     0.0f, .digits = 0 },
						{ .min.flt = 0.0f, .max.flt =     0.0f, .digits = 0 }
					},
				.duration = 120,
				.size	= 1,
				.unit   = {
						.name = "Lux",
						.symbol = "lx",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = CANDELA,.exponent = +1 },
								{ .baseunit = METER,  .exponent = -2 },
								{ .baseunit = NONE,   .exponent = +0 },
								{ .baseunit = NONE,   .exponent = +0 }
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
	sendCmd(CMD_CONT_H);
	delay_ms(180);
}

static uint8_t set_on_(void)
{
	sendCmd(CMD_PWR_ON);
	return 0;
}

static uint8_t set_off_(void)
{
	sendCmd(CMD_PWR_DOWN);
	return 0;
}

static uint8_t set_range_(uint8_t measurement, uint8_t range)
{
	if ((measurement < NO_OF_MEASUREMENTS) && (range < sensor_bh1750.measurements[measurement].ranges))
	{
		range_[measurement] = range;

		return 0;
	}

	return 1;
}

static uint8_t get_measurement_(uint8_t num)
{
	uint16_t tmp;

	if (num < NO_OF_MEASUREMENTS)
	{
		readData(&tmp);

		value_[0].flt = (float)tmp / 1.2f;

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

static int32_t readData(uint16_t *data)
{
	uint8_t bytes[2];

	i2c.tx_data = NULL;
	i2c.tx_length = 0;
	i2c.rx_data = bytes;
	i2c.rx_length = 2;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		*data = (bytes[0] << 8) + bytes[1];

		return 0;
	}
	else
	{
		*data = 0;
		return -1;
	}
}

