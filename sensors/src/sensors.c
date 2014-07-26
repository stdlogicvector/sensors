/*
 * sensors.c
 *
 *  Created on: 22.03.2014
 *      Author: Konstantin
 */

#include "sensors.h"
#include "btm222_module.h"

#define CMD_START_CHAR		'{'
#define CMD_STOP_CHAR		'}'
#define CMD_DELIMITER		'|'

typedef enum {	CMD_STATE_IDLE = 0,
				CMD_STATE_START,
				CMD_STATE_ARG,
				CMD_STATE_END
			 } cmd_state_t;

typedef enum {  CMD_NO_CMD 				 = 0,
				CMD_GET_NO_SENSORS  	 = 'a',
				CMD_GET_SENSOR_INFO 	 = 'b',
				CMD_GET_SENSOR_MEAS_INFO = 'c',
				CMD_GET_SENSOR_UNIT_INFO = 'd',
				CMD_GET_SENSOR_MEAS      = 'e',
				CMD_SET_SENSOR_RANGE	 = 'f',
				CMD_SET_SENSOR_OFF  	 = 'g',
				CMD_SET_SENSOR_ON   	 = 'h',

				CMD_GET_NO_ACTORS	  	 = 'A',
				CMD_GET_ACTOR_INFO 		 = 'B',
				CMD_GET_ACTOR_ACTN_INFO	 = 'C',
				CMD_GET_ACTOR_UNIT_INFO  = 'D',
				CMD_SET_ACTOR_ACTN 	     = 'E',

				CMD_SET_ACTOR_OFF  	 	 = 'G',
				CMD_SET_ACTOR_ON   	 	 = 'H',

				CMD_SEND_I2C			 = 'S',
				CMD_RECV_I2C			 = 'R'
			} cmd_t;

typedef struct
{
	uint8_t b[5];
} ascii85_t;

extern volatile uint8_t btm222_uart_buffer[BTM222_BUFSIZE];
extern volatile uint32_t btm222_uart_count;

void sensor_cmd_process(char char_);
void sensor_cmd_run(char cmd, uint8_t *args);
void sensor_unit_string(const unit_t *unit, char*);

ascii85_t sensor_encode(value_t data);

uint8_t no_of_sensors_ = 0;
sensor_t const **sensors_;

uint8_t no_of_actors_ = 0;
actor_t const **actors_;

void sensors_init(uint8_t no_of_sensors, sensor_t const **sensors)
{
	uint8_t s;

	no_of_sensors_ = no_of_sensors;
	sensors_ = sensors;

	for (s = 0; s < no_of_sensors_; s++)
		sensors_[s]->init();
}

void actorss_init(uint8_t no_of_actors, actor_t const **actors)
{
	no_of_actors_ = no_of_actors;
	actors_ = actors;
}

void sensor_handler(void)
{
	static uint32_t buffer_position = 0;

	while (buffer_position != btm222_uart_count)
	{
		sensor_cmd_process(btm222_uart_buffer[buffer_position]);

		buffer_position++;

		if (buffer_position == BTM222_BUFSIZE)
			buffer_position = 0;
	}
}

void sensor_cmd_process(char char_)
{
	static cmd_state_t cmd_state = CMD_STATE_START;
	static char cmd = CMD_NO_CMD;
	static uint8_t arg[3] = {0, 0, 0};
	static uint8_t count = 0;

	switch (cmd_state)
	{
	case CMD_STATE_IDLE:
		if (char_ == CMD_START_CHAR)
			cmd_state = CMD_STATE_START;
		break;

	case CMD_STATE_START:
		switch (char_)
		{
		case CMD_DELIMITER:
			cmd_state = CMD_STATE_ARG;
			count = 0;
			arg[0] = 0;
			arg[1] = 0;
			arg[2] = 0;
			break;
		case CMD_STOP_CHAR:
			cmd_state = CMD_STATE_END; break;
		default:
			cmd = char_; break;
		}
		break;

	case CMD_STATE_ARG:
		switch (char_)
		{
		case CMD_DELIMITER:
			count++;
			break;
		case CMD_STOP_CHAR:
			cmd_state = CMD_STATE_END; break;
		default:
			arg[count] = char_; break;
		}
		break;

	case CMD_STATE_END:
		sensor_cmd_run(cmd, arg);
		count = 0;
		arg[0] = 0;
		arg[1] = 0;
		arg[2] = 0;
		cmd_state = CMD_STATE_IDLE;
		break;

	default:
		cmd_state = CMD_STATE_IDLE;
		count = 0;
		arg[0] = 0;
		arg[1] = 0;
		arg[2] = 0;
		cmd = CMD_NO_CMD;
		break;

	}
}

void sensor_cmd_run(char cmd, uint8_t *args)
{
	char unitstring[20];
	uint8_t v;
	ascii85_t data;

	//DEBUG
	args[0] -= '0';
	args[1] -= '0';
	//DEBUG

	switch (cmd)
	{
	case CMD_GET_NO_SENSORS:
		btm222_putc(CMD_START_CHAR);
		btm222_putc(CMD_GET_NO_SENSORS);
		btm222_putc(CMD_DELIMITER);

		btm222_putc('0' + no_of_sensors_);
		btm222_putc(CMD_STOP_CHAR);
		break;

	case CMD_GET_SENSOR_INFO:
		if (args[0] < no_of_sensors_)
		{
			btm222_putc(CMD_START_CHAR);
			btm222_putc(CMD_GET_SENSOR_INFO);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[0]);
			btm222_putc(CMD_DELIMITER);

			btm222_puts(sensors_[args[0]]->name);
			btm222_putc(CMD_DELIMITER);
			btm222_puts(sensors_[args[0]]->part);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + sensors_[args[0]]->no_of_measurements);
			btm222_putc(CMD_STOP_CHAR);
		}
		break;

	case CMD_GET_SENSOR_MEAS_INFO:
		if (args[0] < no_of_sensors_ && args[1] < sensors_[args[0]]->no_of_measurements)
		{
			btm222_putc(CMD_START_CHAR);
			btm222_putc(CMD_GET_SENSOR_MEAS_INFO);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[0]);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[1]);
			btm222_putc(CMD_DELIMITER);

			btm222_puts(sensors_[args[0]]->measurements[args[1]].name);
			btm222_putc(CMD_DELIMITER);

			v = sensors_[args[0]]->measurements[args[1]].duration;

			btm222_putc('0' + (v / 100));
			v %= 100;
			btm222_putc('0' + (v / 10));
			v %= 10;
			btm222_putc('0' + (v));

			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + (char)sensors_[args[0]]->measurements[args[1]].type);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + (char)sensors_[args[0]]->measurements[args[1]].size);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + (char)sensors_[args[0]]->measurements[args[1]].ranges);

			for (v = 0; v < sensors_[args[0]]->measurements[args[1]].ranges; v++)
			{
				btm222_putc(CMD_DELIMITER);

				data = sensor_encode(sensors_[args[0]]->measurements[args[1]].range[v].min);
				btm222_send(data.b, 5);

				btm222_putc(CMD_DELIMITER);

				data = sensor_encode(sensors_[args[0]]->measurements[args[1]].range[v].max);
				btm222_send(data.b, 5);

				btm222_putc(CMD_DELIMITER);
				btm222_putc('0' + (char)sensors_[args[0]]->measurements[args[1]].range[v].digits);
			}

			btm222_putc(CMD_STOP_CHAR);
		}
		break;

	case CMD_GET_SENSOR_UNIT_INFO:
		if (args[0] < no_of_sensors_ && args[1] < sensors_[args[0]]->no_of_measurements)
		{
			sensor_unit_string(&(sensors_[args[0]]->measurements[args[1]].unit), unitstring);

			btm222_putc(CMD_START_CHAR);
			btm222_putc(CMD_GET_SENSOR_UNIT_INFO);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[0]);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[1]);
			btm222_putc(CMD_DELIMITER);

			btm222_puts(sensors_[args[0]]->measurements[args[1]].unit.name);
			btm222_putc(CMD_DELIMITER);
			btm222_puts(sensors_[args[0]]->measurements[args[1]].unit.symbol);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + (char)sensors_[args[0]]->measurements[args[1]].unit.prefix);
			btm222_putc(CMD_DELIMITER);
			btm222_send((uint8_t*)unitstring, 20);
			btm222_putc(CMD_STOP_CHAR);
		}
		break;

	case CMD_GET_SENSOR_MEAS:
		if (args[0] < no_of_sensors_ && args[1] < sensors_[args[0]]->no_of_measurements)
		{
			sensors_[args[0]]->get_measurement(args[1]);

			btm222_putc(CMD_START_CHAR);
			btm222_putc(CMD_GET_SENSOR_MEAS);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[0]);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[1]);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + sensors_[args[0]]->range[args[1]]);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + sensors_[args[0]]->measurements[args[1]].size);

			for (v = 0; v < sensors_[args[0]]->measurements[args[1]].size; v++)
			{
				btm222_putc(CMD_DELIMITER);

				data = sensor_encode(sensors_[args[0]]->value[v]);

				btm222_send(data.b, 5);
			}

			btm222_putc(CMD_STOP_CHAR);
		}
		break;

	case CMD_SET_SENSOR_RANGE:
		if (args[0] < no_of_sensors_ && args[1] < sensors_[args[0]]->no_of_measurements)
		{
			sensors_[args[0]]->set_range(args[1], (args[2] - '0'));

			btm222_putc(CMD_START_CHAR);
			btm222_putc(CMD_SET_SENSOR_RANGE);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[0]);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[1]);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + sensors_[args[0]]->range[args[1]]);
			btm222_putc(CMD_STOP_CHAR);
		}
		break;

	case CMD_SET_SENSOR_OFF:
		if (args[0] < no_of_sensors_)
		{
			sensors_[args[0]]->set_off();
		}
		break;

	case CMD_SET_SENSOR_ON:
		if (args[0] < no_of_sensors_)
		{
			sensors_[args[0]]->set_on();
		}
		break;

// ----------------------------------------------------------------------------

	case CMD_GET_NO_ACTORS:
		btm222_putc(CMD_START_CHAR);
		btm222_putc(CMD_GET_NO_ACTORS);
		btm222_putc(CMD_DELIMITER);

		btm222_putc('0' + no_of_actors_);
		btm222_putc(CMD_STOP_CHAR);
		break;

	case CMD_GET_ACTOR_INFO:
		if (args[0] < no_of_actors_)
		{
			btm222_putc(CMD_START_CHAR);
			btm222_putc(CMD_GET_ACTOR_INFO);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[0]);
			btm222_putc(CMD_DELIMITER);

			btm222_puts(actors_[args[0]]->name);
			btm222_putc(CMD_DELIMITER);
			btm222_puts(actors_[args[0]]->part);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + actors_[args[0]]->no_of_actions);
			btm222_putc(CMD_STOP_CHAR);
		}
		break;

	case CMD_GET_ACTOR_ACTN_INFO:
		if (args[0] < no_of_actors_ && args[1] < actors_[args[0]]->no_of_actions)
		{
			btm222_putc(CMD_START_CHAR);
			btm222_putc(CMD_GET_ACTOR_ACTN_INFO);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[0]);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + args[1]);
			btm222_putc(CMD_DELIMITER);

			btm222_puts(actors_[args[0]]->actions[args[1]].name);

			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + (char)actors_[args[0]]->actions[args[1]].type);
			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + (char)actors_[args[0]]->actions[args[1]].size);
			btm222_putc(CMD_DELIMITER);

			data = sensor_encode(actors_[args[0]]->actions[args[1]].range.min);
			btm222_send(data.b, 5);

			btm222_putc(CMD_DELIMITER);

			data = sensor_encode(actors_[args[0]]->actions[args[1]].range.max);
			btm222_send(data.b, 5);

			btm222_putc(CMD_DELIMITER);
			btm222_putc('0' + (char)actors_[args[0]]->actions[args[1]].range.digits);


			btm222_putc(CMD_STOP_CHAR);
		}
		break;

	case CMD_NO_CMD:
	default:
		break;
	}
	btm222_putc(10);
}

void sensor_unit_string(const unit_t *unit, char *unit_string)
{
	uint8_t i;

	for (i = 0; i < 4; i++)
	{
		unit_string[i*5+0] = '0' + unit->baseunits[i].dimension;
		unit_string[i*5+1] = '^';

		if (unit->baseunits[i].exponent >= 0)
		{
			unit_string[i*5+2] = '+';
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

inline uint16_t swap_uint16(uint16_t val)
{
	return ((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8);
}

inline int16_t swap_int16(int16_t val)
{
	return ((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8);
}

ascii85_t sensor_encode(value_t data)
{
    uint8_t i;
    ascii85_t a;

    for (i = 5; i--;)
    {
        a.b[i]   = (data.u_int % 85) + 33;
        data.u_int /= 85;
    }

    return a;
}
