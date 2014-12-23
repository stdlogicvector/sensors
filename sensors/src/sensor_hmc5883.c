/*
 * sensor_hmc5883.c
 *
 *  Created on: 15.04.2014
 *      Author: Konstantin
 */

#include "sensor_hmc5883.h"

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

static int32_t writeReg(uint8_t addr, uint8_t data);
static int32_t setPointer(uint8_t addr);
static int32_t readData(int16_t *data);

// ----------------------------------------------------------------------------

#define I2C_DEV		LPC_I2C0
#define SLAVE_ADDR	0x1E

#define ADDR_CFG_A	0x00
#define ADDR_CFG_B	0x01
#define ADDR_MODE	0x02
#define ADDR_X_MSB	0x03
#define ADDR_X_LSB	0x04
#define ADDR_Y_MSB	0x05
#define ADDR_Y_LSB	0x06
#define ADDR_Z_MSB	0x07
#define ADDR_Z_LSB	0x08
#define ADDR_STATUS	0x09
#define ADDR_ID_A	0x0A
#define ADDR_ID_B	0x0B
#define ADDR_ID_C	0x0C

static const uint8_t range_setting[1][4] = {{0, 2, 4, 7}};
static const float range_factor[1][4] = {{13.7f, 8.2f, 4.4f, 2.3f}};

// Sensor Definition ----------------------------------------------------------

#define NO_OF_MEASUREMENTS		1

static uint8_t range_[NO_OF_MEASUREMENTS] = {0};

const sensor_t sensor_hmc5883 =
{
		.type = SENSOR_MAGNETIC,
		.name = "Magnetometer",
		.part = "HMC5883",

		.no_of_measurements = NO_OF_MEASUREMENTS,

		.measurements =
		{
			{
				.name   = "Flux Density Vector",
				.type	= FLUX,
				.ranges = 4,
				.range  =
					{
						{ .min.flt = - 88.0f, .max.flt =  +88.0f, .digits = 4 },
						{ .min.flt = -190.0f, .max.flt = +190.0f, .digits = 3 },
						{ .min.flt = -400.0f, .max.flt = +400.0f, .digits = 3 },
						{ .min.flt = -810.0f, .max.flt = +810.0f, .digits = 1 }
					},
				.duration = 7,
				.size	= 3,
				.unit   = {
						.name = "Tesla",
						.symbol = "T",
						.prefix = MICRO,
						.baseunits = {
								{ .baseunit = KILOGRAM, .exponent = +1 },
								{ .baseunit = SECOND,   .exponent = -2 },
								{ .baseunit = AMPERE,   .exponent = -1 },
								{ .baseunit = NONE,     .exponent = +0 }
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

	writeReg(ADDR_CFG_A, 0b01100000);	// Clear CRA7, Average 8 values
	set_range_(0, 0);
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
	if (range < sensor_hmc5883.measurements[measurement].ranges)
	{
		// Only one Measurement
		switch (measurement)
		{
			case 0:
				range_[0] = range;
				writeReg(ADDR_CFG_B, range_setting[0][range] << 5);
				return 0;
				break;

			default :
				return 1;
		}
	}

	return 1;
}

static uint8_t get_measurement_(uint8_t num)
{
	int16_t data[3];

	if (num < NO_OF_MEASUREMENTS)
	{
		writeReg(ADDR_MODE, 0b00000001); // Clear MR7 and initiate Single Measurement
		setPointer(ADDR_X_MSB);
		delay_ms(6);

		readData(data);

		value_[0].flt = (float)data[0] / range_factor[0][range_[0]];
		value_[1].flt = (float)data[1] / range_factor[0][range_[0]];
		value_[2].flt = (float)data[2] / range_factor[0][range_[0]];

		return 0;
	} else
		return -1;
}

// Sensor Specific Functions -------------------------------------------------

static int32_t writeReg(uint8_t addr, uint8_t data)
{
	uint8_t cmd[2];

	cmd[0] = addr;
	cmd[1] = data;

	i2c.tx_data = cmd;
	i2c.tx_length = 2;
	i2c.rx_data = NULL;
	i2c.rx_length = 0;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
		return 0;
	else
		return -1;
}

static int32_t setPointer(uint8_t addr)
{
	i2c.tx_data= &addr;
	i2c.tx_length = 1;
	i2c.rx_data = NULL;
	i2c.rx_length = 0;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
		return 0;
	else
		return -1;
}

static int32_t readData(int16_t *data)
{
	uint8_t bytes[6];

	i2c.tx_data = NULL;
	i2c.tx_length = 0;
	i2c.rx_data = bytes;
	i2c.rx_length = 6;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		data[0] = (bytes[0] << 8) + bytes[1];
		data[1] = (bytes[2] << 8) + bytes[3];
		data[2] = (bytes[4] << 8) + bytes[5];

		return 0;
	}
	else
	{
		data[0] = 0;
		data[1] = 0;
		data[2] = 0;

		return -1;
	}
}

