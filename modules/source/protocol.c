/*
 * protocol.c
 *
 *  Created on: 06.09.2014
 *      Author: Konstantin
 */

#include "protocol.h"

typedef enum {
	CMD_STATE_IDLE = 0,
	CMD_STATE_START,
	CMD_STATE_ARG,
	CMD_STATE_END
} cmd_state_t;

/*
 * Incoming Commands:
 * <identifier> = Command		(Single Character)
 * <argument 1> = SensorID		(Single Character (starting from '0'))
 * <argument 2> = MeasurmentID	(Single Character (starting from '0'))
 * <argument 3> = Range/OnOff	(Single Character (starting from '0'))
 * <argument 4> = Parameter		(5 Bytes, ASCII85 encoded)
 *
 */

void protocol_parse(char char_, void (*callback)(char cmd, uint8_t *args))
{
	static cmd_state_t cmd_state = CMD_STATE_START;
	static char cmd = CMD_NO_CMD;

	static uint8_t arg[3] = {0, 0, 0};
	static uint8_t count = 0;

	switch (cmd_state)
	{
	case CMD_STATE_IDLE:
		if (char_ == OBJECT_START)
			cmd_state = CMD_STATE_START;
		break;

	case CMD_STATE_START:
		switch (char_)
		{
		case DELIMITER:
			cmd_state = CMD_STATE_ARG;
			count = 0;
			arg[0] = 0;
			arg[1] = 0;
			arg[2] = 0;
			break;
		case OBJECT_END:
			cmd_state = CMD_STATE_END; break;
		default:
			cmd = char_; break;
		}
		break;

	case CMD_STATE_ARG:
		switch (char_)
		{
		case DELIMITER:
			count++;
			break;
		case OBJECT_END:
			cmd_state = CMD_STATE_END; break;
		default:
			arg[count] = char_; break;
		}
		break;

	case CMD_STATE_END:
		callback(cmd, arg);
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

