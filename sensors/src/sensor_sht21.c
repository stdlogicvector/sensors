/*
 * sensor_sht21.c
 *
 *  Created on: 11.04.2014
 *      Author: Konstantin
 */

#include "sensor_sht21.h"

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

static uint16_t getHumidity();
static uint16_t getTemperature();

#define I2C_DEV		LPC_I2C0
#define SLAVE_ADDR	0b1000000

#define CMD_RESET	0b11111110
#define CMD_MEAS_T	0b11110011
#define CMD_MEAS_RH	0b11110101
#define CMD_WRITE	0b11100110
#define CMD_READ	0b11100111

// Sensor Definition ----------------------------------------------------------

#define NO_OF_MEASUREMENTS		2

static uint8_t range_[NO_OF_MEASUREMENTS] = {0};

const sensor_t sensor_sht21 =
{
		.name = "Humidity Sensor",
		.part = "SHT21",

		.no_of_measurements = NO_OF_MEASUREMENTS,

		.measurements =
		{
			{
				.name   = "Humidity",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = 0.0f, .max.flt = 100.0f, .digits = 2 },
						{ .min.flt = 0.0f, .max.flt =   0.0f, .digits = 0 },
						{ .min.flt = 0.0f, .max.flt =   0.0f, .digits = 0 },
						{ .min.flt = 0.0f, .max.flt =   0.0f, .digits = 0 }
					},
				.duration = 40,
				.size	= 1,
				.type   = TYPE_FLOAT,
				.unit   = {
						.name = "Relative Humidity",
						.symbol = "%RH",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .dimension = NONE,   .exponent = +1 },
								{ .dimension = NONE,   .exponent = +0 },
								{ .dimension = NONE,   .exponent = +0 },
								{ .dimension = NONE,   .exponent = +0 }
							}
					}
			},
			{
				.name   = "Temperature",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = -40.0f, .max.flt = +125.0f, .digits = 2 },
						{ .min.flt =  0.0f, .max.flt =     0.0f, .digits = 0 },
						{ .min.flt =  0.0f, .max.flt =     0.0f, .digits = 0 },
						{ .min.flt =  0.0f, .max.flt =     0.0f, .digits = 0 }
					},
				.duration = 95,
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
	if ((measurement < NO_OF_MEASUREMENTS) && (range < sensor_sht21.measurements[measurement].ranges))
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
		if (num == 0)
		{
			tmp = getHumidity();
			value_[0].flt =  -6.00f + 125.00f * (float)tmp / (1 << 16);
		} else
		{
			tmp = getTemperature();
			value_[0].flt = -46.85f + 175.72f * (float)tmp / (1 << 16);
		}

		return 0;
	} else
		return 1;
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
		return 1;
}

static int32_t readData(uint16_t *data)
{
	uint8_t bytes[3];

	i2c.tx_data = NULL;
	i2c.tx_length = 0;
	i2c.rx_data = bytes;
	i2c.rx_length = 3;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		*data  = (bytes[0] << 8) + bytes[1]; // Byte 3 is CRC

		return 0;
	}
	else
	{
		*data = 0;
		return 1;
	}
}

static uint16_t getHumidity()
{
	uint16_t data;

	sendCmd(CMD_MEAS_RH);
	delay_ms(29);
	readData(&data);

	data &= ~0x0003;	// Mask away two LSB (Statusbits)

	return data;
}

static uint16_t getTemperature()
{
	uint16_t data;

	sendCmd(CMD_MEAS_T);
	delay_ms(85);
	readData(&data);

	data &= ~0x0003;	// Mask away two LSB (Statusbits)

	return data;
}
