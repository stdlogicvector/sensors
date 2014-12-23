/*
 * sensors.h
 *
 *  Created on: 21.03.2014
 *      Author: Konstantin
 */

#ifndef SENSORS_H_
#define SENSORS_H_

#include <stdint.h>

typedef enum {	PICO  = 0,
				NANO,
				MICRO,
				MILLI,
				CENTI,
				DECI,
			    NO_PREFIX,
			    DECA,
			    HECTO,
			    KILO,
			    MEGA,
			    GIGA,
			    TERA
			} prefix_t;

typedef enum {	NONE     = 0,
				METER,
				KILOGRAM,
				SECOND,
				AMPERE,
				KELVIN,
				MOLE,
				CANDELA,
				DEGREE
			} baseunit_t;

typedef enum {	SENSOR_GENERIC = 0,
				SENSOR_ACCELERATION,
				SENSOR_ADC,
				SENSOR_AUDIO,
				SENSOR_COMPASS,
				SENSOR_COLOR,
//				SENSOR_CURRENT,
				SENSOR_DISTANCE,
				SENSOR_GAS,
				SENSOR_HUMIDITY,
				SENSOR_INFRARED,
				SENSOR_LIGHT,
				SENSOR_MAGNETIC,
//				SENSOR_MOTION,
				SENSOR_POLARISATION,
				SENSOR_PRESSURE,
				SENSOR_ROTATION,
				SENSOR_RADIATION,
//				SENSOR_SPECTRAL,
				SENSOR_TEMPERATURE
} sensortype_t;

typedef enum {	GENERIC = 0,
				ACCELERATION,
				ANGLE,
				ANGULAR_SPEED,
				BRIGHTNESS,
				COLOR,
				CONCENTRATION,
				COUNTS,
				DIRECTION,
				DISTANCE,
				CURRENT,
				FLUX,
				FORCE,
				HUMIDITY,
				LOUDNESS,
				TEMPERATURE,
				VOLTAGE,
				PRESSURE
} measurementtype_t;

typedef struct
{
	baseunit_t baseunit;
	int8_t exponent;
} subunit_t;

typedef struct
{
	const char * name;				// Name of the Unit (e.g. 'Pressure')
	const char * symbol;			// Symbol of the Measurement (e.g. 'Pa')
	const prefix_t prefix;			// Metrix Prefix of the Unit (e.g. Hecto)
	const subunit_t baseunits[4];	// e.g. 1 * KILOGRAM^1 * METER^-1 * SECOND^-2  (Unused = 1)
} unit_t;

typedef union
{
	float flt;
	uint32_t u_int;
} value_t;

typedef struct
{
	const value_t max;
	const value_t min;
	const uint8_t digits;		// Decimal Places
} range_t;

typedef struct
{
	const char * name;			// Name of the Measurement (e.g. 'Acceleration')
	const measurementtype_t type;
	const uint8_t ranges;		// Number of Ranges
	const range_t range[4];		// Ranges of the Measurement (e.g. -8g to +8g)
	const uint16_t duration;	// Duration of Measurement in Milliseconds
	const uint8_t size;			// Number of Values
	const unit_t unit;			// Unit of the Measurement (e.g. METER^1 * SECOND^-2)
} measurement_t;

typedef struct
{
	const char * name;
	const range_t range;
	const uint8_t size;
	const unit_t unit;
} action_t;

typedef struct
{
	const sensortype_t type;

	const char * name;
	const char * part;

	const uint8_t no_of_measurements;

	const value_t * value;
	const uint8_t * range;

	void (*const init)(void);

	uint8_t (*const set_on)(void);
	uint8_t (*const set_off)(void);
	uint8_t (*const set_range)(uint8_t, uint8_t);
	uint8_t (*const get_measurement)(uint8_t);

	const measurement_t measurements[];
} sensor_t;

typedef struct
{
	const char * name;
	const char * part;

	const uint8_t no_of_actions;

	const value_t * value;

	void (*const init)(void);

	uint8_t (*const set_on)(void);
	uint8_t (*const set_off)(void);
	uint8_t (*const set_action)(uint8_t);

	const action_t actions[];
} actor_t;

void sensors_init(uint8_t no_of_sensors, sensor_t const **sensors);
void sensor_handler(void);

uint16_t swap_uint16(uint16_t val);
int16_t swap_int16(int16_t val);

#endif /* SENSORS_H_ */
