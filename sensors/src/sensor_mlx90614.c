/*
 * sensor_mlx90614.c
 *
 *  Created on: 15.04.2014
 *      Author: Konstantin
 */

#include "sensor_mlx90614.h"

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
static uint16_t readRAM(uint8_t addr);
static uint16_t readEEPROM(uint8_t addr);

// ----------------------------------------------------------------------------

#define I2C_DEV		LPC_I2C0
#define SLAVE_ADDR	0x5A

#define CMD_SLEEP	0b11111111
#define CMD_RAM		0b00000000
#define CMD_EEPROM	0b00100000
#define CMD_FLAGS	0b11110000

// RAM Addresses
#define ADDR_CH1_RAW	0x04
#define ADDR_CH2_RAW	0x05
#define ADDR_TA			0x06
#define ADDR_TOBJ1		0x07
#define ADDR_TOBJ2		0x08

// EEPROM Addresses
#define ADDR_TO_MAX		0x00
#define ADDR_TO_MIN		0x01
#define ADDR_PWMCTRL	0x02
#define ADDR_TA_RANGE	0x03
#define ADDR_EMISSIVITY	0x04
#define ADDR_CONFIG		0x05
#define ADDR_SLV_ADDR	0x0E
#define ADDR_ID_0		0x1C
#define ADDR_ID_1		0x1D
#define ADDR_ID_2		0x1E
#define ADDR_ID_3		0x1F


// Sensor Definition ----------------------------------------------------------

#define NO_OF_MEASUREMENTS		3

static uint8_t range_[NO_OF_MEASUREMENTS] = {0};

const sensor_t sensor_mlx90614 =
{
		.name = "IR Thermometer",
		.part = "MLX90614",

		.no_of_measurements = NO_OF_MEASUREMENTS,

		.measurements =
		{
			{
				.name   = "Ambient Temperature",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = -40.00f, .max.flt = 125.0f, .digits = 2 },
						{ .min.flt =    0.0f, .max.flt =   0.0f, .digits = 0 },
						{ .min.flt =    0.0f, .max.flt =   0.0f, .digits = 0 },
						{ .min.flt =    0.0f, .max.flt =   0.0f, .digits = 0 }
					},
				.duration = 100, // ?
				.size	= 1,
				.type   = TYPE_FLOAT,
				.unit   = {
						.name = "Celsius",
						.symbol = "\xB0\x43",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .dimension = KELVIN, .exponent = +1 },
								{ .dimension = NONE,   .exponent = +0 },
								{ .dimension = NONE,   .exponent = +0 },
								{ .dimension = NONE,   .exponent = +0 }
							}
					}
			},
			{
				.name   = "Object Temperature 1",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = -70.0f, .max.flt = +383.0f, .digits = 2 },
						{ .min.flt =   0.0f, .max.flt =    0.0f, .digits = 0 },
						{ .min.flt =   0.0f, .max.flt =    0.0f, .digits = 0 },
						{ .min.flt =   0.0f, .max.flt =    0.0f, .digits = 0 },
					},
				.duration = 100, // ?
				.size	= 1,
				.type   = TYPE_FLOAT,
				.unit   = {
						.name = "Celsius",
						.symbol = "\xB0\x43",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .dimension = KELVIN, .exponent = +1 },
								{ .dimension = NONE,   .exponent = +0 },
								{ .dimension = NONE,   .exponent = +0 },
								{ .dimension = NONE,   .exponent = +0 }
							}
					}
			},
			{
				.name   = "Object Temperature 2",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = -70.0f, .max.flt = +383.0f, .digits = 2 },
						{ .min.flt =   0.0f, .max.flt =    0.0f, .digits = 0 },
						{ .min.flt =   0.0f, .max.flt =    0.0f, .digits = 0 },
						{ .min.flt =   0.0f, .max.flt =    0.0f, .digits = 0 },
					},
				.duration = 100, // ?
				.size	= 1,
				.type   = TYPE_FLOAT,
				.unit   = {
						.name = "Celsius",
						.symbol = "\xB0\x43",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .dimension = KELVIN, .exponent = +1 },
								{ .dimension = NONE,   .exponent = +0 },
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

// Sensor Interface Functions -------------------------------------------------

static void init_(void)
{
	i2c.sl_addr7bit = SLAVE_ADDR;
	i2c.retransmissions_max = 1;
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
	if ((measurement < NO_OF_MEASUREMENTS) && (range < sensor_mlx90614.measurements[measurement].ranges))
	{
		range_[measurement] = range;

		return 0;
	}

	return 1;
}

static uint8_t get_measurement_(uint8_t num)
{
	uint16_t tmp = 0;

	if (num < NO_OF_MEASUREMENTS)
	{
		switch (num)
		{
		case 0: tmp = readRAM(ADDR_TA); break;
		case 1: tmp = readRAM(ADDR_TOBJ1); break;
		case 2: tmp = readRAM(ADDR_TOBJ2); break;
		}

		value_[0].flt = (float)tmp / 50.0f - 273.15f;

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

static uint16_t readRAM(uint8_t addr)
{
	uint8_t data[3];
	uint8_t cmd = CMD_RAM | (0x1F & addr);

	i2c.tx_data = &cmd;
	i2c.tx_length = 1;
	i2c.rx_data = data;
	i2c.rx_length = 3;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		return (data[1] << 8) + data[0]; // Byte 3 is CRC
	}
	else
	{
		return 0;
	}
}

static uint16_t readEEPROM(uint8_t addr)
{
	uint8_t data[3];
	uint8_t cmd = CMD_EEPROM | (0x1F & addr);

	i2c.tx_data = &cmd;
	i2c.tx_length = 1;
	i2c.rx_data = data;
	i2c.rx_length = 3;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		return (data[1] << 8) + data[0]; // Byte 3 is CRC
	}
	else
	{
		return 0;
	}
}


