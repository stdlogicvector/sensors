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

static value_t value_[64];

// ----------------------------------------------------------------------------

static I2C_M_SETUP_Type i2c;

static void dumpEEPROM(uint8_t *eeprom);
static void writeTrim(uint8_t trim);

static uint16_t readConfig(void);
static void writeConfig(uint8_t rate);

static uint16_t getPixelComp(void);
static uint16_t getAmbientTemp(void);
static void getPixels(void);

static void calcConstants(uint8_t *eeprom);

float Tambient;

uint16_t raw_pixel[64];

int32_t v_th;
int8_t a_cp;
int8_t b_cp;
int8_t tgc;
int8_t b_i_scale;

float k_t1;
float k_t2;
float emissivity;

int8_t a_ij[64];
int8_t b_ij[64];


// ----------------------------------------------------------------------------

#define I2C_DEV			LPC_I2C0
#define SLAVE_ADDR_RAM	0x60

#define CMD_WRITE_TRIM	0x04
#define CMD_WRITE_CONF	0x03
#define CMD_READ_RAM	0x02

#define SLAVE_ADDR_EEPROM	0x50
#define CMD_READ_EEPROM		0x00

#define TRIM_CHECK_BYTE	0xAA
#define CONF_CHECK_BYTE 0x55

// EEPROM Addresses
#define VTH_L			0xDA
#define VTH_H			0xDB
#define VT1_L			0xDC
#define VT1_H			0xDD
#define VT2_L			0xDE
#define VT2_H			0xDF

#define PIXEL_OFFSET	0x00
#define PIXEL_SLOPE		0x40
#define PIXEL_COEFF		0x80

#define ACP				0xD4
#define BCP				0xD5
#define ALPHA_CP_L		0xD6
#define ALPHA_CP_H		0xD7
#define TGC				0xD8
#define BI				0xD9
#define ALPHA_0_L		0xE0
#define ALPHA_0_H		0xE1
#define ALPHA_0_SCALE	0xE2
#define D_ALPHA_0_SCALE	0xE3
#define EPSILON_L		0xE4
#define EPSILON_H		0xE5

#define OSC_TRIM		0xF7

// RAM Addresses
#define PIXEL			0x00
#define PTAT_VALUE		0x90
#define TGC_VALUE		0x91
#define CONFIG			0x92
#define TRIMMING		0x93

// Sensor Definition ----------------------------------------------------------

#define NO_OF_MEASUREMENTS		2

static uint8_t range_[NO_OF_MEASUREMENTS] = {0};

const sensor_t sensor_mlx90620 =
{
		.name = "16x4 IR Array",
		.part = "MLX90620",

		.no_of_measurements = NO_OF_MEASUREMENTS,

		.measurements =
		{
			{
				.name   = "Ambient Temperature",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = -40.00f, .max.flt = 85.0f, .digits = 2 },
						{ .min.flt =    0.0f, .max.flt =  0.0f, .digits = 0 },
						{ .min.flt =    0.0f, .max.flt =  0.0f, .digits = 0 },
						{ .min.flt =    0.0f, .max.flt =  0.0f, .digits = 0 }
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
				.name   = "Pixel Temperature",
				.ranges = 1,
				.range  =
					{
						{ .min.flt = -20.0f, .max.flt = +300.0f, .digits = 2 },
						{ .min.flt =   0.0f, .max.flt =    0.0f, .digits = 0 },
						{ .min.flt =   0.0f, .max.flt =    0.0f, .digits = 0 },
						{ .min.flt =   0.0f, .max.flt =    0.0f, .digits = 0 },
					},
				.duration = 100, // ?
				.size	= 64,
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
	uint8_t eeprom[256];

	i2c.retransmissions_max = 1;

	dumpEEPROM(eeprom);
	calcConstants(eeprom);
	writeTrim(eeprom[OSC_TRIM]);
	writeConfig(2);
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
		default: break;
		}

		value_[0].flt = 0;

		return 0;
	} else
		return -1;
}

// Sensor Specific Functions -------------------------------------------------

static uint16_t getAmbientTemp(void)
{
	uint8_t cmd[4];
	uint8_t ta[2];

	i2c.sl_addr7bit = SLAVE_ADDR_RAM;
	i2c.tx_length = 4;
	i2c.rx_length = 2;
	i2c.tx_data = cmd;
	i2c.rx_data = ta;

	cmd[0] = CMD_READ_RAM;	// CMD
	cmd[1] = PTAT_VALUE;	// Start Address
	cmd[2] = 0x00;			// Address step
	cmd[3] = 0x01;			// Number of Reads

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		return ((ta[1] << 8) + ta[0]);
	} else
	{
		return 0;
	}
}

static void getPixels(void)
{
	uint8_t p;
	uint8_t cmd[4];
	uint8_t pxl[128];

	i2c.sl_addr7bit = SLAVE_ADDR_RAM;
	i2c.tx_length = 4;
	i2c.rx_length = 64;
	i2c.tx_data = cmd;
	i2c.rx_data = pxl;

	cmd[0] = CMD_READ_RAM;	// CMD
	cmd[1] = PIXEL;			// Start Address
	cmd[2] = 0x01;			// Address step
	cmd[3] = 0x40;			// Number of Reads

	I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING);

	for (p = 0; p < 64; p++)
	{
		raw_pixel[p] = (uint16_t) ((pxl[p+1] << 8) + pxl[p]);
	}
}

static uint16_t getPixelComp(void)
{
	uint8_t cmd[4];
	uint8_t cmp[2];

	i2c.sl_addr7bit = SLAVE_ADDR_RAM;
	i2c.tx_length = 4;
	i2c.rx_length = 2;
	i2c.tx_data = cmd;
	i2c.rx_data = cmp;

	cmd[0] = CMD_READ_RAM;	// CMD
	cmd[1] = TGC_VALUE;		// Start Address
	cmd[2] = 0x00;			// Address step
	cmd[3] = 0x01;			// Number of Reads

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		return ((cmp[1] << 8) + cmp[0]);
	} else
	{
		return 0;
	}
}

static uint16_t readConfig(void)
{
	uint8_t cmd[4];
	uint8_t cnf[2];

	i2c.sl_addr7bit = SLAVE_ADDR_RAM;
	i2c.tx_length = 4;
	i2c.rx_length = 2;
	i2c.tx_data = cmd;
	i2c.rx_data = cnf;
	i2c.retransmissions_max = 1;

	cmd[0] = CMD_READ_RAM;	// CMD
	cmd[1] = CONFIG;		// Start Address
	cmd[2] = 0x00;			// Address step
	cmd[3] = 0x01;			// Number of Reads

	if (I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		return ((cnf[1] << 8) + cnf[0]);
	} else
	{
		return 0;
	}
}

static void dumpEEPROM(uint8_t *eeprom)
{
	uint8_t cmd = CMD_READ_EEPROM;

	i2c.sl_addr7bit = SLAVE_ADDR_EEPROM;
	i2c.tx_length = 1;
	i2c.tx_data = &cmd;
	i2c.rx_data = eeprom;
	i2c.rx_length = 256;

	I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING);
}

static void writeTrim(uint8_t trim)
{
	uint8_t cmd[5];

	i2c.sl_addr7bit = SLAVE_ADDR_RAM;
	i2c.tx_length = 5;
	i2c.rx_length = 0;
	i2c.tx_data = cmd;
	i2c.rx_data = NULL;

	cmd[0] = CMD_WRITE_TRIM;
	cmd[1] = trim - TRIM_CHECK_BYTE;
	cmd[2] = trim;
	cmd[3] = 0x56;
	cmd[4] = 0x00;

	I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING);
}

static void writeConfig(uint8_t rate)
{
	uint8_t cmd[5];
	uint8_t rate_lsb;
	uint8_t conf;

	i2c.sl_addr7bit = SLAVE_ADDR_RAM;
	i2c.tx_length = 5;
	i2c.rx_length = 0;
	i2c.tx_data = cmd;
	i2c.rx_data = NULL;

	switch(rate)
	{
	case 0:
		rate_lsb = 0b00001111;
		break;
	case 1:
		rate_lsb = 0b00001110;
		break;
	case 2:
		rate_lsb = 0b00001101;
		break;
	case 4:
		rate_lsb = 0b00001100;
		break;
	case 8:
		rate_lsb = 0b00001011;
		break;
	case 16:
		rate_lsb = 0b00001010;
		break;
	case 32:
		rate_lsb = 0b00001001;
		break;
	default:
		rate_lsb = 0b00001110;
	}

	conf = 0b01110100;	// NA = 0, ADC low ref, Ta-Refresh 2Hz

	cmd[0] = CMD_WRITE_CONF;
	cmd[1] = rate_lsb - CONF_CHECK_BYTE;
	cmd[2] = rate_lsb;
	cmd[3] = conf - CONF_CHECK_BYTE;
	cmd[4] = conf;

	I2C_MasterTransferData(I2C_DEV, &i2c, I2C_TRANSFER_POLLING);
}

static void calcConstants(uint8_t *eeprom)
{
	uint8_t p;

	v_th = (float)((eeprom[VTH_H] << 8) * eeprom[VTH_L]);
	k_t1 = (float)((eeprom[VT1_H] << 8) * eeprom[VT1_L]);
	k_t2 = (float)((eeprom[VT2_H] << 8) * eeprom[VT2_L]);

	emissivity = ((float)(eeprom[EPSILON_H] << 8) * eeprom[EPSILON_L]) / (1 << 15);

	a_cp = eeprom[ACP];
	b_cp = eeprom[BCP];
	tgc  = eeprom[TGC];

	b_i_scale = eeprom[BI];

	for (p = 0; p < 64; p++)
	{
		a_ij[p] = eeprom[PIXEL_OFFSET + p];
		b_ij[p] = eeprom[PIXEL_SLOPE  + p];
	}
}


