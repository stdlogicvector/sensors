/*
 * sensor_tcs3472.c
 *
 *  Created on: 21.03.2014
 *      Author: Konstantin
 *
 *    			Interface for TCS3472 Color Sensor
 */

#include "sensor_tcs3472.h"

#include "lpc17xx_i2c.h"
#include "delay_module.h"

// ----------------------------------------------------------------------------

static void init_(void);
static uint8_t set_on_(void);
static uint8_t set_off_(void);
static uint8_t set_range_(uint8_t measurement, uint8_t range);
static uint8_t get_measurement_(uint8_t num);

static value_t value_[3];

// ----------------------------------------------------------------------------

static I2C_M_SETUP_Type i2c;

static int32_t writeReg(uint8_t reg, uint8_t data);
static int32_t readRGB(value_t *value);
static int32_t readWhite(value_t *value);

// ----------------------------------------------------------------------------

#define TCS3472_ADDR	0x29

#define I2C_DEV			LPC_I2C0

#define ENBL_REG		0x00
#define CTRL_REG		0x0F
#define	CNFG_REG		0x0D
#define TMNG_REG		0x01
#define WAIT_REG		0x03

#define CMD_PON			0x01
#define CMD_READY		0x03
#define CMD_SLEEP		0x00

// Sensor Definition ----------------------------------------------------------

#define NO_OF_MEASUREMENTS		2

static uint8_t range_[NO_OF_MEASUREMENTS] = {0};

const sensor_t sensor_tcs3472 =
{
		.name = "Color Sensor",
		.part = "TCS3472",

		.no_of_measurements = NO_OF_MEASUREMENTS,

		.measurements =
		{
			{
				.name   = "RGB",
				.ranges = 1,
				.range  =
					{
						{ .min.u_int = 0, .max.u_int = 0xFFFF, .digits = 0 }
					},
				.duration = 100,
				.size	= 3,
				.type   = TYPE_UINT16,
				.unit   = {
						.name = "Count",
						.symbol = "c",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .dimension = NONE, .exponent = +1 },
								{ .dimension = NONE, .exponent = +0 },
								{ .dimension = NONE, .exponent = +0 },
								{ .dimension = NONE, .exponent = +0 }
							}
					}
			},
			{
				.name   = "White",
				.ranges = 1,
				.range  =
					{
						{ .min.u_int = 0, .max.u_int = 0xFFFF, .digits = 0 }
					},
				.duration = 100,
				.size	= 1,
				.type   = TYPE_UINT16,
				.unit   = {
						.name = "Count",
						.symbol = "c",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .dimension = NONE, .exponent = +1 },
								{ .dimension = NONE, .exponent = +0 },
								{ .dimension = NONE, .exponent = +0 },
								{ .dimension = NONE, .exponent = +0 }
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
	i2c.sl_addr7bit = TCS3472_ADDR;
	i2c.retransmissions_max = 1;

	writeReg(ENBL_REG, CMD_PON);

	delay_ms(3);

	writeReg(TMNG_REG, 0xD5);	// 101ms Integration Time
//	writeReg(WAIT_REG, 0xFF);	// 2.4ms Wait Time (Minimum) (is default reset value)
//	writeReg(CTRL_REG, 0x00);	// Disable WLONG bit	(is default reset value)
	writeReg(ENBL_REG, CMD_SLEEP);
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
	if ((measurement < NO_OF_MEASUREMENTS) && (range < sensor_tcs3472.measurements[measurement].ranges))
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
		writeReg(ENBL_REG, CMD_PON);

		delay_ms(3);

		writeReg(ENBL_REG, CMD_READY);

		delay_ms(105);

		switch (num)
		{
			case 0: readRGB(value_); break;

			case 1: readWhite(&value_[0]); break;
		}

		writeReg(ENBL_REG, CMD_SLEEP);

		return 0;
	} else
		return -1;
}

// Sensor Specific Functions -------------------------------------------------

static int32_t writeReg(uint8_t reg, uint8_t data)
{
	uint8_t cmd[2];

	cmd[0] = 0x80 | (reg & 0x1F);
	cmd[1] = data;

	i2c.tx_data	= cmd;
	i2c.tx_length = 2;
	i2c.rx_data = NULL;
	i2c.rx_length = 0;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
		return 0;
	else
		return -1;
}

static int32_t readRGB(value_t *value)
{
	uint8_t data[6];
	uint8_t cmd = 0b10100000 | 0x16;	// Autoincrement Register Address during reading

	i2c.tx_data	= &cmd;
	i2c.tx_length = 1;
	i2c.rx_data = data;
	i2c.rx_length = 6;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		value[0].u_int = (data[1] << 8) + data[0];	// R
		value[1].u_int = (data[3] << 8) + data[2];	// G
		value[2].u_int = (data[5] << 8) + data[4];	// B

		return 0;
	}
	else
	{
		value[0].u_int = 0;
		value[1].u_int = 0;
		value[2].u_int = 0;

		return -1;
	}
}

static int32_t readWhite(value_t *value)
{
	uint8_t data[2];
	uint8_t cmd = 0b10100000 | 0x14;	// Autoincrement Register Address during reading

	i2c.tx_data	= &cmd;
	i2c.tx_length = 1;
	i2c.rx_data = data;
	i2c.rx_length = 2;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		value->u_int = (data[1] << 8) + data[0];

		return 0;
	}
	else
	{
		value->u_int = 0;

		return -1;
	}
}

