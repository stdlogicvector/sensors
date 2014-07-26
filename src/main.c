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

#include "debug_module.h"
#include "delay_module.h"
#include "btm222_module.h"

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
sensor_t const *sensors[NO_OF_SENSORS];

void init_interfaces(void);
void init_sensors(void);

void init_interfaces(void)
{
	delay_init();
	btm222_init(115200);

	I2C_Init(LPC_I2C0, 100000);
	I2C_DeInit(LPC_I2C1);
	I2C_DeInit(LPC_I2C2);
}

void init_sensors(void)
{
	sensors[0] = &sensor_bh1750;
	sensors[1] = &sensor_tsl2561;
	sensors[2] = &sensor_tcs3472;
	sensors[3] = &sensor_ms5611;
	sensors[4] = &sensor_mpu6050;
	sensors[5] = &sensor_hmc5883;
	sensors[6] = &sensor_mlx90614;
	sensors[7] = &sensor_sht21;
	sensors[8] = &sensor_adc;
	//sensors[8] = &sensor_mlx90620;

	sensors_init(NO_OF_SENSORS, sensors);
}

int main(void)
{
	init_interfaces();
	init_sensors();

	while(1)
	{
		sensor_handler();
	}

	return 0;
}

