/*
 * sensor_tsl2561.c
 *
 *  Created on: 21.03.2014
 *      Author: Konstantin
 *
 *    			Interface for TSL2561 Light Sensor
 */

#include "sensor_tsl2561.h"

#include "lpc17xx_i2c.h"
#include "delay_module.h"

// ----------------------------------------------------------------------------

static void init_(void);
static uint8_t set_on_(void);
static uint8_t set_off_(void);
static uint8_t set_range_(uint8_t measurement, uint8_t range);
static uint8_t get_measurement_(uint8_t num);

static value_t value_[2];

// ----------------------------------------------------------------------------

static I2C_M_SETUP_Type i2c;

static uint8_t channel[6] = {0, 1, 2, 5, 6, 7};

static int32_t selectCh(int8_t ch);
static int32_t writeReg(uint8_t reg, uint8_t data);
static int32_t readData(float *vis, float *ir);

// ----------------------------------------------------------------------------

#define TSL2561_ADDR	0b0101001
#define PCA9547_ADDR	0b1110000

#define I2C_DEV			LPC_I2C0

#define CTRL_REG		0x00
#define TMNG_REG		0x01

#define CMD_PWR_ON		0x03
#define CMD_PWR_OFF		0x00

// Sensor Definition ----------------------------------------------------------

#define NO_OF_MEASUREMENTS		6

static uint8_t range_[NO_OF_MEASUREMENTS] = {0};

const sensor_t sensor_tsl2561 =
{
		.type = POLARISATION,
		.name = "Polarization Sensor",
		.part = "TSL2561",

		.no_of_measurements = NO_OF_MEASUREMENTS,

		.measurements =
		{
			{
				.name   = "Unpolarized",
				.ranges = 1,
				.range  =
					{
						{ .min.u_int = 0, .max.u_int = 0xFFFF, .digits = 0 }
					},
				.duration = 100,
				.size	= 2,
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
			},
			{
				.name   = "Linear 0\xB0",
				.ranges = 1,
				.range  =
					{
						{ .min.u_int = 0, .max.u_int = 0xFFFF, .digits = 0 }
					},
				.duration = 100,
				.size	= 2,
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
			},
			{
				.name   = "Linear 45\xB0",
				.ranges = 1,
				.range  =
					{
						{ .min.u_int = 0, .max.u_int = 0xFFFF, .digits = 0 }
					},
				.duration = 100,
				.size	= 2,
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
			},
			{
				.name   = "Linear 90\xB0",
				.ranges = 1,
				.range  =
					{
						{ .min.u_int = 0, .max.u_int = 0xFFFF, .digits = 0 }
					},
				.duration = 100,
				.size	= 2,
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
			},
			{
				.name   = "Circular R",
				.ranges = 1,
				.range  =
					{
						{ .min.u_int = 0, .max.u_int = 0xFFFF, .digits = 0 }
					},
				.duration = 100,
				.size	= 2,
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
			},
			{
				.name   = "Circular L",
				.ranges = 1,
				.range  =
					{
						{ .min.u_int = 0, .max.u_int = 0xFFFF, .digits = 0 }
					},
				.duration = 100,
				.size	= 2,
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
	i2c.retransmissions_max = 1;
	set_on_();
}

static uint8_t set_on_(void)
{
	uint8_t ch;

	for (ch = 0; ch < 6; ch++)
	{
		selectCh(channel[ch]);
		writeReg(CTRL_REG, CMD_PWR_ON);
//		writeReg(TMNG_REG, 0b00000010);
	}

	selectCh(-1);

	return 0;
}

static uint8_t set_off_(void)
{
	uint8_t ch;

	for (ch = 0; ch < 6; ch++)
	{
		selectCh(channel[ch]);
		writeReg(CTRL_REG, CMD_PWR_OFF);
	}

	selectCh(-1);

	return 0;
}

static uint8_t set_range_(uint8_t measurement, uint8_t range)
{
	if ((measurement < NO_OF_MEASUREMENTS) && (range < sensor_tsl2561.measurements[measurement].ranges))
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
		selectCh(channel[num]);

		readData(&value_[0].flt, &value_[1].flt);

		selectCh(-1);

		return 0;
	} else
		return -1;
}

// Sensor Specific Functions -------------------------------------------------

static int32_t selectCh(int8_t ch)
{
	uint8_t cmd = 0b00000000; 			// Select no Channel

	if (ch >= 0)
		cmd |= 0b1000 | (ch & 0x07);	// Set Enable Bit and Channel

	i2c.sl_addr7bit = PCA9547_ADDR;
	i2c.tx_data	= &cmd;
	i2c.tx_length = 1;
	i2c.rx_data = NULL;
	i2c.rx_length = 0;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
		return 0;
	else
		return -1;
}

static int32_t writeReg(uint8_t reg, uint8_t data)
{
	uint8_t cmd[2];

	cmd[0] = 0x80 | (reg & 0x0F);
	cmd[1] = data;

	i2c.sl_addr7bit = TSL2561_ADDR;
	i2c.tx_data	= cmd;
	i2c.tx_length = 2;
	i2c.rx_data = NULL;
	i2c.rx_length = 0;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
		return 0;
	else
		return -1;
}

// 0xFF47
// 0xD209

// 0x1112
// 0x6302

static int32_t readData(float *vis, float *ir)
{
	uint8_t data[4];
	uint8_t cmd = 0x9C;

	i2c.sl_addr7bit = TSL2561_ADDR;
	i2c.tx_data	= &cmd;
	i2c.tx_length = 1;
	i2c.rx_data = data;
	i2c.rx_length = 4;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		*vis = (float) ((data[1] << 8) + data[0]);
		*ir  = (float) ((data[3] << 8) + data[2]);

		return 0;
	}
	else
	{
		*vis = 0;
		*ir  = 0;
		return -1;
	}
}

