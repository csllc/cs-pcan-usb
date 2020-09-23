#include <stdio.h>
#include <stdint.h>
#include <string.h>


#include "pcan_helper.h"
#include "PCANBasic.h"




struct {
    int status;
    char* string;
} statuses[] = {
    { PCAN_ERROR_OK, "PCAN_ERROR_OK" },
    { PCAN_ERROR_XMTFULL, "PCAN_ERROR_XMTFULL" },
    { PCAN_ERROR_OVERRUN, "PCAN_ERROR_OVERRUN" },
    { PCAN_ERROR_BUSLIGHT, "PCAN_ERROR_BUSLIGHT" },
    { PCAN_ERROR_BUSHEAVY, "PCAN_ERROR_BUSHEAVY" },
    { PCAN_ERROR_BUSWARNING, "PCAN_ERROR_BUSWARNING" },
    { PCAN_ERROR_BUSPASSIVE, "PCAN_ERROR_BUSPASSIVE" },
    { PCAN_ERROR_BUSOFF, "PCAN_ERROR_BUSOFF" },
    { PCAN_ERROR_ANYBUSERR, "PCAN_ERROR_ANYBUSERR" },
    { PCAN_ERROR_QRCVEMPTY, "PCAN_ERROR_QRCVEMPTY" },
    { PCAN_ERROR_QOVERRUN, "PCAN_ERROR_QOVERRUN" },
    { PCAN_ERROR_QXMTFULL, "PCAN_ERROR_QXMTFULL" },
    { PCAN_ERROR_REGTEST, "PCAN_ERROR_REGTEST" },
    { PCAN_ERROR_NODRIVER, "PCAN_ERROR_NODRIVER" },
    { PCAN_ERROR_HWINUSE, "PCAN_ERROR_HWINUSE" },
    { PCAN_ERROR_NETINUSE, "PCAN_ERROR_NETINUSE" },
    { PCAN_ERROR_ILLHW, "PCAN_ERROR_ILLHW" },
    { PCAN_ERROR_ILLNET, "PCAN_ERROR_ILLNET" },
    { PCAN_ERROR_ILLCLIENT, "PCAN_ERROR_ILLCLIENT" },
    { PCAN_ERROR_ILLHANDLE, "PCAN_ERROR_ILLHANDLE" },
    { PCAN_ERROR_RESOURCE, "PCAN_ERROR_RESOURCE" },
    { PCAN_ERROR_ILLPARAMTYPE, "PCAN_ERROR_ILLPARAMTYPE" },
    { PCAN_ERROR_ILLPARAMVAL, "PCAN_ERROR_ILLPARAMVAL" },
    { PCAN_ERROR_UNKNOWN, "PCAN_ERROR_UNKNOWN" },
    { PCAN_ERROR_ILLDATA, "PCAN_ERROR_ILLDATA" },
    { PCAN_ERROR_ILLMODE, "PCAN_ERROR_ILLMODE" },
    { PCAN_ERROR_CAUTION, "PCAN_ERROR_CAUTION" },
    { PCAN_ERROR_INITIALIZE, "PCAN_ERROR_INITIALIZE" },
    { PCAN_ERROR_ILLOPERATION, "PCAN_ERROR_ILLOPERATION" },
};




char *pcan_status_lookup(TPCANStatus status)
{
    int i;

    for (i = 0; statuses[i].string; i++) {
	if (statuses[i].status == status) {
	    return statuses[i].string;
	}
    }

    return "status unknown";
}




void pcan_dump_msg(TPCANMsg *msg)
{
    int i;

    printf("msg { \n"
	     "  ID = 0x%02X\n"
	     "  MSGTYPE = 0x%02X (%s)\n"
	     "  LEN = 0x%02X\n"
	     "  DATA = {",
	   msg->ID,
	   msg->MSGTYPE,
	   pcan_msgtype_lookup(msg->MSGTYPE),
	   msg->LEN);

    for (i=0; i < (msg->LEN); i++) {
	printf("0x%02X", msg->DATA[i]);
	if (i != ((msg->LEN) - 1)) {
	    printf(", ");
	}
    }

    printf("}\n"
	   "}\n");

    return;
}




void pcan_dump_buffer(BYTE *buffer, size_t buffer_len)
{
    int i;

    printf("{");

    for (i = 0; i < buffer_len; i++) {
	printf("0x%02X", buffer[i]);
	if (i != (buffer_len - 1)) {
	    printf(", ");
	}
    }

    printf("}");

    return;
}




struct {
    int type;
    char* string;
} types [] = {
    { PCAN_MESSAGE_STANDARD, "PCAN_MESSAGE_STANDARD" },
    { PCAN_MESSAGE_RTR, "PCAN_MESSAGE_RTR" },
    { PCAN_MESSAGE_EXTENDED, "PCAN_MESSAGE_EXTENDED" },
    { PCAN_MESSAGE_FD, "PCAN_MESSAGE_FD" },
    { PCAN_MESSAGE_BRS, "PCAN_MESSAGE_BRS" },
    { PCAN_MESSAGE_ESI, "PCAN_MESSAGE_ESI" },
    { PCAN_MESSAGE_ERRFRAME, "PCAN_MESSAGE_ERRFRAME" },
    { PCAN_MESSAGE_STATUS, "PCAN_MESSAGE_STATUS" },
    { 0, 0 }
};




char *pcan_msgtype_lookup(TPCANMessageType type)
{
    int i;

    for (i = 0; types[i].string; i++) {
	if (types[i].type == type) {
	    return types[i].string;
	}
    }

    return "msgtype unknown";
}




void pcan_dump_timestamp(TPCANTimestamp *timestamp)
{
    printf("timestamp {\n"
	   "  millis = %i\n"
	   "  millis_overflow = %i\n"
	   "  micros = %i\n"
	   "}\n",
	   timestamp->millis,
	   timestamp->millis_overflow,
	   timestamp->micros);

    return;
}




struct {
    int parameter;
    char* string;
} parameters [] = {
    { PCAN_DEVICE_ID, "PCAN_DEVICE_ID" },
    { PCAN_5VOLTS_POWER, "PCAN_5VOLTS_POWER" },
    { PCAN_RECEIVE_EVENT, "PCAN_RECEIVE_EVENT" },
    { PCAN_MESSAGE_FILTER, "PCAN_MESSAGE_FILTER" },
    { PCAN_API_VERSION, "PCAN_API_VERSION" },
    { PCAN_CHANNEL_VERSION, "PCAN_CHANNEL_VERSION" },
    { PCAN_BUSOFF_AUTORESET, "PCAN_BUSOFF_AUTORESET" },
    { PCAN_LISTEN_ONLY, "PCAN_LISTEN_ONLY" },
    { PCAN_LOG_LOCATION, "PCAN_LOG_LOCATION" },
    { PCAN_LOG_STATUS, "PCAN_LOG_STATUS" },
    { PCAN_LOG_CONFIGURE, "PCAN_LOG_CONFIGURE" },
    { PCAN_LOG_TEXT, "PCAN_LOG_TEXT" },
    { PCAN_CHANNEL_CONDITION, "PCAN_CHANNEL_CONDITION" },
    { PCAN_HARDWARE_NAME, "PCAN_HARDWARE_NAME" },
    { PCAN_RECEIVE_STATUS, "PCAN_RECEIVE_STATUS" },
    { PCAN_CONTROLLER_NUMBER, "PCAN_CONTROLLER_NUMBER" },
    { PCAN_TRACE_LOCATION, "PCAN_TRACE_LOCATION" },
    { PCAN_TRACE_STATUS, "PCAN_TRACE_STATUS" },
    { PCAN_TRACE_SIZE, "PCAN_TRACE_SIZE" },
    { PCAN_TRACE_CONFIGURE, "PCAN_TRACE_CONFIGURE" },
    { PCAN_CHANNEL_IDENTIFYING, "PCAN_CHANNEL_IDENTIFYING" },
    { PCAN_CHANNEL_FEATURES, "PCAN_CHANNEL_FEATURES" },
    { PCAN_BITRATE_ADAPTING, "PCAN_BITRATE_ADAPTING" },
    { PCAN_BITRATE_INFO, "PCAN_BITRATE_INFO" },
    { PCAN_BITRATE_INFO_FD, "PCAN_BITRATE_INFO_FD" },
    { PCAN_BUSSPEED_NOMINAL, "PCAN_BUSSPEED_NOMINAL" },
    { PCAN_BUSSPEED_DATA, "PCAN_BUSSPEED_DATA" },
    { PCAN_IP_ADDRESS, "PCAN_IP_ADDRESS" },
    { PCAN_LAN_SERVICE_STATUS, "PCAN_LAN_SERVICE_STATUS" },
    { PCAN_ALLOW_STATUS_FRAMES, "PCAN_ALLOW_STATUS_FRAMES" },
    { PCAN_ALLOW_RTR_FRAMES, "PCAN_ALLOW_RTR_FRAMES" },
    { PCAN_ALLOW_ERROR_FRAMES, "PCAN_ALLOW_ERROR_FRAMES" },
    { PCAN_INTERFRAME_DELAY, "PCAN_INTERFRAME_DELAY" },
    { PCAN_ACCEPTANCE_FILTER_11BIT, "PCAN_ACCEPTANCE_FILTER_11BIT" },
    { PCAN_ACCEPTANCE_FILTER_29BIT, "PCAN_ACCEPTANCE_FILTER_29BIT" },
    { PCAN_IO_DIGITAL_CONFIGURATION, "PCAN_IO_DIGITAL_CONFIGURATION" },
    { PCAN_IO_DIGITAL_VALUE, "PCAN_IO_DIGITAL_VALUE" },
    { PCAN_IO_DIGITAL_SET, "PCAN_IO_DIGITAL_SET" },
    { PCAN_IO_DIGITAL_CLEAR, "PCAN_IO_DIGITAL_CLEAR" },
    { PCAN_IO_ANALOG_VALUE, "PCAN_IO_ANALOG_VALUE" },
    { PCAN_FIRMWARE_VERSION, "PCAN_FIRMWARE_VERSION" },
    { PCAN_ATTACHED_CHANNELS_COUNT, "PCAN_ATTACHED_CHANNELS_COUNT" },
    { PCAN_ATTACHED_CHANNELS, "PCAN_ATTACHED_CHANNELS" },
    { 0, 0 }
};




char *pcan_parameter_lookup(TPCANParameter parameter)
{
    int i;

    for (i = 0; parameters[i].string; i++) {
	if (parameters[i].parameter == parameter) {
	    return parameters[i].string;
	}
    }

    return "parameter unknown";
}

    


void pcan_dump_thread_params(thread_params_t *thread_params)
{
    printf("thread_params {\n"
	   "  can_channel = 0x%02X\n"
	   "  callback = %p\n"
	   "}\n",
	   thread_params->can_channel,
	   thread_params->callback);

    return;
}

