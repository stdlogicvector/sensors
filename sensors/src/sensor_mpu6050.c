/*
 * sensor_mpu6050.c
 *
 *  Created on: 11.04.2014
 *      Author: Konstantin
 */

#include "sensor_mpu6050.h"

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
static int32_t readReg(uint8_t addr, uint8_t *data);
static int32_t readBurst(uint8_t addr, uint8_t *data, uint8_t count);

// ----------------------------------------------------------------------------

static int32_t getAcceleration();
static int32_t getAngularRate();
static float getTemperature();

#define MPU6050_ADDR		0
#define I2C_DEV	LPC_I2C0

#if (MPU6050_ADDR == 0)
#	define SLAVE_ADDR	0b1101000
#else
#	define SLAVE_ADDR	0b1101001
#endif


#define ADDR_GYRO_CONFIG	0x1B
#define ADDR_ACCEL_CONFIG	0x1C
#define ADDR_INT_PIN_CFG	0x37

#define ADDR_ACCEL_XOUT_H	0x3B
#define ADDR_ACCEL_XOUT_L	0x3C
#define ADDR_ACCEL_YOUT_H	0x3D
#define ADDR_ACCEL_YOUT_L	0x3E
#define ADDR_ACCEL_ZOUT_H	0x3F
#define ADDR_ACCEL_ZOUT_L	0x40

#define ADDR_TEMP_OUT_H		0x41
#define ADDR_TEMP_OUT_L		0x42

#define ADDR_GYRO_XOUT_H	0x43
#define ADDR_GYRO_XOUT_L	0x44
#define ADDR_GYRO_YOUT_H	0x45
#define ADDR_GYRO_YOUT_L	0x46
#define ADDR_GYRO_ZOUT_H	0x47
#define ADDR_GYRO_ZOUT_L	0x48


#define ADDR_USER_CTRL		0x6A
#define ADDR_PWR_MGMT_1		0x6B

static const uint8_t range_setting[2][4] = {{0, 1, 2, 3}, {0, 1, 2, 3}};
static const float range_factor[2][4] = {{16384.0f, 8192.0f, 4096.0f, 2048.0f}, {131.0f, 65.5f, 32.8f, 16.4f}};

// Sensor Definition ----------------------------------------------------------

#define NO_OF_MEASUREMENTS		3

static uint8_t range_[NO_OF_MEASUREMENTS] = {0};

const sensor_t sensor_mpu6050 =
{
		.type = ACCELERATION,
		.name = "IMU",
		.part = "MPU6050",

		.no_of_measurements = NO_OF_MEASUREMENTS,

		.measurements =
		{
			{
				.name   = "Acceleration Vector",
				.ranges = 4,
				.range  =
					{
							{ .min.flt =  -2.0f, .max.flt =  +2.0f, .digits = 5},
							{ .min.flt =  -4.0f, .max.flt =  +4.0f, .digits = 4},
							{ .min.flt =  -8.0f, .max.flt =  +8.0f, .digits = 4},
							{ .min.flt = -16.0f, .max.flt = +16.0f, .digits = 4}
					},
				.duration = 1,
				.size	= 3,
				.unit   = {
						.name = "g",
						.symbol = "a",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = METER,  .exponent = +1 },
								{ .baseunit = SECOND, .exponent = -2 },
								{ .baseunit = NONE,   .exponent = +0 },
								{ .baseunit = NONE,   .exponent = +0 }
							}
					}
			},
			{
				.name   = "Angular Rate Vector",
				.ranges = 4,
				.range  =
					{
						{ .min.flt =  -250.0f, .max.flt =  +250.0f, .digits = 3 },
						{ .min.flt =  -500.0f, .max.flt =  +500.0f, .digits = 2 },
						{ .min.flt = -1000.0f, .max.flt = +1000.0f, .digits = 2 },
						{ .min.flt = -2000.0f, .max.flt = +2000.0f, .digits = 2 },
					},
				.duration = 1,
				.size	= 3,
				.unit   = {
						.name = "\xB0/s",
						.symbol = "\xB0/s",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = DEGREE, .exponent = +1 },
								{ .baseunit = SECOND, .exponent = -1 },
								{ .baseunit = NONE,   .exponent = +0 },
								{ .baseunit = NONE,   .exponent = +0 }
							}
					}
			},
			{
				.name   = "Temperature",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = -40.0f, .max.flt = +85.0f, .digits = 2 },
						{ .min.flt =  -0.0f, .max.flt =  +0.0f, .digits = 0 },
						{ .min.flt =  -0.0f, .max.flt =  +0.0f, .digits = 0 },
						{ .min.flt =  -0.0f, .max.flt =  +0.0f, .digits = 0 },
					},
				.duration = 1,
				.size	= 1,
				.unit   = {
						.name = "Celsius",
						.symbol = "\xB0\x43",
						.prefix = NO_PREFIX,
						.baseunits = {
								{ .baseunit = KELVIN, .exponent = +1 },
								{ .baseunit = NONE,   .exponent = +0 },
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

	writeReg(ADDR_PWR_MGMT_1,  0b00000001);	//Reset, Disable Sleep, X-Gyro as Clock
	writeReg(ADDR_USER_CTRL,   0b00000000);
	writeReg(ADDR_INT_PIN_CFG, 0b00000010);
}

static uint8_t set_on_(void)
{
	return writeReg(ADDR_PWR_MGMT_1,  0b00000001);	//Disable Sleep
}

static uint8_t set_off_(void)
{
	return writeReg(ADDR_PWR_MGMT_1,  0b01000001);	//Enable Sleep
}

static uint8_t set_range_(uint8_t measurement, uint8_t range)
{
	if (range < sensor_mpu6050.measurements[measurement].ranges)
	{
		switch (measurement)
		{
			case 0:
				range_[0] = range;
				writeReg(ADDR_ACCEL_CONFIG, range_setting[0][range] << 3);
				return 0;
				break;

			case 1:
				range_[1] = range;
				writeReg(ADDR_GYRO_CONFIG, range_setting[1][range] << 3);
				return 0;
				break;

			default:
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
		switch (num)
		{
		case 0 :	// Accel Vector

			getAcceleration(data);

			value_[0].flt = (float)swap_int16(data[0]) / range_factor[0][range_[0]];
			value_[1].flt = (float)swap_int16(data[1]) / range_factor[0][range_[0]];
			value_[2].flt = (float)swap_int16(data[2]) / range_factor[0][range_[0]];

		break;

		case 1 :	// Gyro Vector

			getAngularRate(data);

			value_[0].flt = (float)swap_int16(data[0]) / range_factor[1][range_[1]];
			value_[1].flt = (float)swap_int16(data[1]) / range_factor[1][range_[1]];
			value_[2].flt = (float)swap_int16(data[2]) / range_factor[1][range_[1]];

		break;

		case 2 : 	// Temperature
			value_[0].flt = getTemperature();
		break;

		}

		return 0;
	} else
		return -1;
}

static int32_t getAcceleration(int16_t *accel)
{
	return readBurst(ADDR_ACCEL_XOUT_H, (uint8_t*)accel, 6);
}

static int32_t getAngularRate(int16_t *gyro)
{
	return readBurst(ADDR_GYRO_XOUT_H, (uint8_t*)gyro, 6);
}

static float getTemperature()
{
	int16_t temp;

	readBurst(ADDR_TEMP_OUT_H, (uint8_t*)&temp, 2);

	return ((float)swap_int16(temp) / 340.0f + 36.53f);
}

static int32_t writeReg(uint8_t addr, uint8_t data)
{
	uint8_t cmd[2];

	cmd[0] = addr;
	cmd[1] = data;

	i2c.tx_length = 2;
	i2c.tx_data = cmd;
	i2c.rx_data = NULL;
	i2c.rx_length = 0;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
		return 0;
	else
		return -1;

}

static int32_t readReg(uint8_t addr, uint8_t *data)
{
	i2c.tx_length = 1;
	i2c.tx_data = &addr;
	i2c.rx_length = 1;
	i2c.rx_data = data;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
		return 0;
	else
		return -1;
}

static int32_t readBurst(uint8_t addr, uint8_t *data, uint8_t count)
{
	i2c.tx_length = 1;
	i2c.tx_data = &addr;
	i2c.rx_length = count;
	i2c.rx_data = data;

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
		return 0;
	else
		return -1;
}
