#ifndef _HELPER_PCAN_H_
#define _HELPER_PCAN_H_

#include <stdint.h>
#include "PCANBasic.h"
#include "pcan_thread.h"

/* PCAN helpers */
char *pcan_status_lookup(TPCANStatus status);
void pcan_dump_msg(TPCANMsg *msg);
void pcan_dump_msg_fd(TPCANMsgFD *msg);
void pcan_dump_buffer(BYTE *buffer, size_t buffer_len);
char *pcan_msgtype_lookup(TPCANMessageType type);
void pcan_dump_timestamp(TPCANTimestamp *timestamp);
void pcan_dump_timestamp_fd(TPCANTimestampFD timestamp);
char *pcan_parameter_lookup(TPCANParameter parameter);
void pcan_dump_thread_params(thread_params_t *thread_params);
uint8_t pcan_dlc_decode(uint8_t dlc);

#endif /* _HELPER_PCAN_H_ */
