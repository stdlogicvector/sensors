/*
===============================================================================
 Name        : main.c
 Author      : Konstantin
 Version     : 0.1
 Copyright   : Copyright (C) Konstantin
 Description :
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include "lpc17xx_i2c.h"

#include "delay_module.h"
#include "serial_module.h"

#include "sensors.h"
#include "sensor_adc.h"
#include "sensor_bh1750.h"
#include "sensor_tsl2561.h"
#include "sensor_tcs3472.h"
#include "sensor_ms5611.h"
#include "sensor_mpu6050.h"
#include "sensor_hmc5883.h"
#include "sensor_sht21.h"
#include "sensor_mlx90614.h"
#include "sensor_mlx90620.h"

#define NO_OF_SENSORS	9

sensor_t const *sensors[NO_OF_SENSORS] = {
		&sensor_bh1750,
		&sensor_tsl2561,
		&sensor_tcs3472,
		&sensor_ms5611,
		&sensor_mpu6050,
		&sensor_hmc5883,
		&sensor_mlx90614,
		&sensor_sht21,
		&sensor_adc,
//		&sensor_mlx90620,
};

int main(void)
{
	delay_init();

	serial_init(9600);

	I2C_Init(LPC_I2C0, 100000);
	I2C_DeInit(LPC_I2C1);
	I2C_DeInit(LPC_I2C2);

	sensors_init(NO_OF_SENSORS, sensors);

	while (1)
	{
		sensor_handler();
	}

	return 0;
}

