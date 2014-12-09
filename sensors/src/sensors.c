/*
 * sensors.c
 *
 *  Created on: 22.03.2014
 *      Author: Konstantin
 */

#include "sensors.h"
#include "serial_module.h"
#include "protocol.h"
#include "ascii85.h"

void sensor_cmd(char cmd, uint8_t *args);

void handleUnknownCommand(char cmd);
void handleIdentify();
void handleGetNoOfSensors();
void handleGetSensor(uint8_t sensorId);
void handleGetNoOfMeasurements(uint8_t sensorId);
void handleGetMeasurement(uint8_t sensorId, uint8_t measurementId);
void handleGetValue(uint8_t sensorId, uint8_t measurementId);
void handleSetSensorRange(uint8_t sensorId, uint8_t measurementId, uint8_t range);
void handleSetSensorOff(uint8_t sensorId);
void handleSetSensorOn(uint8_t sensorId);

void send_encoded(uint32_t data);
void make_unit_string(const unit_t *unit, char*);

uint8_t no_of_sensors_ = 0;
sensor_t const **sensors_;

uint8_t no_of_actors_ = 0;
actor_t const **actors_;

extern volatile uint8_t serial_uart_buffer[SERIAL_BUFSIZE];
extern volatile uint32_t serial_uart_count;

void sensors_init(uint8_t no_of_sensors, sensor_t const **sensors)
{
	uint8_t s;

	no_of_sensors_ = no_of_sensors;
	sensors_ = sensors;

	for (s = 0; s < no_of_sensors_; s++)
		sensors_[s]->init();
}

void actors_init(uint8_t no_of_actors, actor_t const **actors)
{
	uint8_t a;

	no_of_actors_ = no_of_actors;
	actors_ = actors;

	for (a = 0; a < no_of_actors_; a++)
		actors_[a]->init();
}

void sensor_handler(void)
{
	static uint32_t buffer_position = 0;

	while (buffer_position != serial_uart_count)
	{
		protocol_parse((char) serial_uart_buffer[buffer_position], &sensor_cmd);

		buffer_position++;

		if (buffer_position == SERIAL_BUFSIZE)
			buffer_position = 0;
	}
}

void sensor_cmd(char cmd, uint8_t *args)
{
	//DEBUG
	args[0] -= '0';
	args[1] -= '0';
	args[2] -= '0';
	//DEBUG

	switch (cmd)
	{
	case CMD_IDENTIFY:				handleIdentify(); break;
	case CMD_GET_NO_SENSORS:		handleGetNoOfSensors(); break;
	case CMD_GET_SENSOR:			handleGetSensor(args[0]); break;
	case CMD_GET_NO_MEAS:			handleGetNoOfMeasurements(args[0]); break;
	case CMD_GET_SENSOR_MEAS:		handleGetMeasurement(args[0], args[1]); break;
	case CMD_GET_SENSOR_VALUE:		handleGetValue(args[0], args[1]); break;
	case CMD_SET_SENSOR_RANGE:		handleSetSensorRange(args[0], args[1], args[2]); break;
	case CMD_SET_SENSOR_OFF:		handleSetSensorOff(args[0]); break;
	case CMD_SET_SENSOR_ON:			handleSetSensorOn(args[0]); break;

// ----------------------------------------------------------------------------

	default:
	case CMD_NO_CMD: handleUnknownCommand(cmd); break;
	}

	serial_putc(10);
}

void send_encoded(uint32_t data)
{
	ascii85_t encoded = ascii85_encode(data);
	serial_send(encoded.b, 5);
}

void handleUnknownCommand(char cmd)
{

}

void handleIdentify()
{
	serial_putc(OBJECT_START);
	serial_putc(CMD_IDENTIFY);
	serial_putc(DELIMITER);
	serial_puts("SensorArray");
	serial_putc(OBJECT_END);
}

void handleGetNoOfSensors()
{
	serial_putc(OBJECT_START);
	serial_putc(CMD_GET_NO_SENSORS);
	serial_putc(DELIMITER);

	serial_putc('0' + no_of_sensors_);
	serial_putc(OBJECT_END);
}

void handleGetSensor(uint8_t sensorId)
{
	if (sensorId < no_of_sensors_)
	{
		serial_putc(OBJECT_START);
		serial_putc(CMD_GET_SENSOR);
		serial_putc(DELIMITER);
		serial_putc('0' + sensorId);
		serial_putc(DELIMITER);

		serial_putc('0' + (uint8_t) sensors_[sensorId]->type);
		serial_putc(DELIMITER);
		serial_puts(sensors_[sensorId]->name);
		serial_putc(DELIMITER);
		serial_puts(sensors_[sensorId]->part);
		serial_putc(OBJECT_END);
	}
}


void handleGetNoOfMeasurements(uint8_t sensorId)
{
	if (sensorId < no_of_sensors_)
	{
		serial_putc(OBJECT_START);
		serial_putc(CMD_GET_NO_MEAS);
		serial_putc(DELIMITER);
		serial_putc('0' + sensorId);
		serial_putc(DELIMITER);

		serial_putc('0' + sensors_[sensorId]->no_of_measurements);
		serial_putc(OBJECT_END);
	}
}

void handleGetMeasurement(uint8_t sensorId, uint8_t measurementId)
{
	uint8_t r;
	char unitstring[20];

	if (sensorId < no_of_sensors_ && measurementId < sensors_[sensorId]->no_of_measurements)
	{
		make_unit_string(&(sensors_[sensorId]->measurements[measurementId].unit), unitstring);

		serial_putc(OBJECT_START);
		serial_putc(CMD_GET_SENSOR_MEAS);		// 0
		serial_putc(DELIMITER);
		serial_putc('0' + sensorId);			// 1
		serial_putc(DELIMITER);
		serial_putc('0' + measurementId);		// 2
		serial_putc(DELIMITER);

		serial_puts(sensors_[sensorId]->measurements[measurementId].name);				// 3
		serial_putc(DELIMITER);
		serial_putc('0' + (char)sensors_[sensorId]->measurements[measurementId].size);	// 4
		serial_putc(DELIMITER);
		send_encoded(sensors_[sensorId]->measurements[measurementId].duration);			// 5
		serial_putc(DELIMITER);

		serial_puts(sensors_[sensorId]->measurements[measurementId].unit.name);			// 6
		serial_putc(DELIMITER);
		serial_puts(sensors_[sensorId]->measurements[measurementId].unit.symbol);		// 7
		serial_putc(DELIMITER);
		serial_putc('0' + (char)sensors_[sensorId]->measurements[measurementId].unit.prefix);	// 8
		serial_putc(DELIMITER);
		serial_send((uint8_t*)unitstring, 20);											// 9
		serial_putc(DELIMITER);

		serial_putc('0' + (char)sensors_[sensorId]->measurements[measurementId].ranges);

		for (r = 0; r < sensors_[sensorId]->measurements[measurementId].ranges; r++)
		{
			serial_putc(DELIMITER);
			send_encoded(sensors_[sensorId]->measurements[measurementId].range[r].min.u_int);
			serial_putc(DELIMITER);
			send_encoded(sensors_[sensorId]->measurements[measurementId].range[r].max.u_int);
			serial_putc(DELIMITER);
			serial_putc('0' + (char)sensors_[sensorId]->measurements[measurementId].range[r].digits);
		}

		serial_putc(OBJECT_END);
	}
}

void handleGetValue(uint8_t sensorId, uint8_t measurementId)
{
	uint8_t v;

	if (sensorId < no_of_sensors_ && measurementId < sensors_[sensorId]->no_of_measurements)
	{
		sensors_[sensorId]->get_measurement(measurementId);

		serial_putc(OBJECT_START);
		serial_putc(CMD_GET_SENSOR_VALUE);
		serial_putc(DELIMITER);
		serial_putc('0' + sensorId);
		serial_putc(DELIMITER);
		serial_putc('0' + measurementId);
		serial_putc(DELIMITER);
		serial_putc('0' + sensors_[sensorId]->range[measurementId]);
		serial_putc(DELIMITER);
		serial_putc('0' + sensors_[sensorId]->measurements[measurementId].size);

		for (v = 0; v < sensors_[sensorId]->measurements[measurementId].size; v++)
		{
			serial_putc(DELIMITER);
			send_encoded(sensors_[sensorId]->value[v].u_int);
		}

		serial_putc(OBJECT_END);
	}
}

void handleSetSensorRange(uint8_t sensorId, uint8_t measurementId, uint8_t range)
{
	if (sensorId < no_of_sensors_ && measurementId < sensors_[sensorId]->no_of_measurements)
	{
		sensors_[sensorId]->set_range(measurementId, range);

		serial_putc(OBJECT_START);
		serial_putc(CMD_SET_SENSOR_RANGE);
		serial_putc(DELIMITER);
		serial_putc('0' + sensorId);
		serial_putc(DELIMITER);
		serial_putc('0' + measurementId);
		serial_putc(DELIMITER);
		serial_putc('0' + sensors_[sensorId]->range[measurementId]);
		serial_putc(OBJECT_END);
	}
}

void handleSetSensorOff(uint8_t sensorId)
{
	if (sensorId < no_of_sensors_)
	{
		sensors_[sensorId]->set_off();
	}
}

void handleSetSensorOn(uint8_t sensorId)
{
	if (sensorId < no_of_sensors_)
	{
		sensors_[sensorId]->set_on();
	}
}

void make_unit_string(const unit_t *unit, char *unit_string)
{
	uint8_t i;

	for (i = 0; i < 4; i++)
	{
		unit_string[i*5+0] = '0' + unit->baseunits[i].baseunit;
		unit_string[i*5+1] = '^';

		if (unit->baseunits[i].exponent >= 0)
		{
			unit_string[i*5+2] = ' ';
			unit_string[i*5+3] = '0' + (unit->baseunits[i].exponent % 10);
		}
		else
		{
			unit_string[i*5+2] = '-';
			unit_string[i*5+3] = '0' + (-unit->baseunits[i].exponent % 10);
		}

		unit_string[i*5+4] = ';';
	}
}


/*
	case CMD_GET_NO_ACTORS:
		serial_putc(OBJECT_START);
		serial_putc(CMD_GET_NO_ACTORS);
		serial_putc(DELIMITER);

		serial_putc('0' + no_of_actors_);
		serial_putc(OBJECT_END);
		break;

	case CMD_GET_ACTOR_INFO:
		if (args[0] < no_of_actors_)
		{
			serial_putc(OBJECT_START);
			serial_putc(CMD_GET_ACTOR_INFO);
			serial_putc(DELIMITER);
			serial_putc('0' + args[0]);
			serial_putc(DELIMITER);

			serial_puts(actors_[args[0]]->name);
			serial_putc(DELIMITER);
			serial_puts(actors_[args[0]]->part);
			serial_putc(DELIMITER);
			serial_putc('0' + actors_[args[0]]->no_of_actions);
			serial_putc(OBJECT_END);
		}
		break;

	case CMD_GET_ACTOR_ACTN_INFO:
		if (args[0] < no_of_actors_ && args[1] < actors_[args[0]]->no_of_actions)
		{
			serial_putc(OBJECT_START);
			serial_putc(CMD_GET_ACTOR_ACTN_INFO);
			serial_putc(DELIMITER);
			serial_putc('0' + args[0]);
			serial_putc(DELIMITER);
			serial_putc('0' + args[1]);
			serial_putc(DELIMITER);

			serial_puts(actors_[args[0]]->actions[args[1]].name);

			serial_putc(DELIMITER);
			serial_putc('0' + (char)actors_[args[0]]->actions[args[1]].type);
			serial_putc(DELIMITER);
			serial_putc('0' + (char)actors_[args[0]]->actions[args[1]].size);
			serial_putc(DELIMITER);

			data = sensor_encode(actors_[args[0]]->actions[args[1]].range.min);
			serial_send(data.b, 5);

			serial_putc(DELIMITER);

			data = sensor_encode(actors_[args[0]]->actions[args[1]].range.max);
			serial_send(data.b, 5);

			serial_putc(DELIMITER);
			serial_putc('0' + (char)actors_[args[0]]->actions[args[1]].range.digits);

			serial_putc(OBJECT_END);
		}
		break;
 */


