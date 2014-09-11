/*
 * protocol.h
 *
 *  Created on: 06.09.2014
 *      Author: Konstantin
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>

#define OBJECT_START	'{'
#define OBJECT_END		'}'
#define DELIMITER		'|'
#define SEPARATOR		'~'

typedef enum {
	CMD_NO_CMD 				 = 0,

	CMD_IDENTIFY			 = '@',

	CMD_GET_NO_SENSORS  	 = 'a',
	CMD_GET_SENSOR_INFO 	 = 'b',
	CMD_GET_SENSOR_MEAS_INFO = 'c',
	CMD_GET_SENSOR_MEAS      = 'd',
	CMD_SET_SENSOR_RANGE	 = 'e',
	CMD_SET_SENSOR_OFF  	 = 'f',
	CMD_SET_SENSOR_ON   	 = 'g',

	CMD_GET_NO_ACTORS	  	 = 'A',
	CMD_GET_ACTOR_INFO 		 = 'B',
	CMD_GET_ACTOR_ACTN_INFO	 = 'C',
	CMD_SET_ACTOR_ACTN 	     = 'D',

	CMD_SET_ACTOR_OFF  	 	 = 'F',
	CMD_SET_ACTOR_ON   	 	 = 'G',

	CMD_SEND_I2C			 = '>',
	CMD_RECV_I2C			 = '<'

} protocol_cmd_t;

void protocol_parse(char char_, void (*callback)(char cmd, uint8_t *args));

#endif /* PROTOCOL_H_ */
