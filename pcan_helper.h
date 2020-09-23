#ifndef _HELPER_PCAN_H_
#define _HELPER_PCAN_H_

#include "PCANBasic.h"
#include "pcan_thread.h"

/* PCAN helpers */
char *pcan_status_lookup(TPCANStatus status);
void pcan_dump_msg(TPCANMsg *msg);
void pcan_dump_buffer(BYTE *buffer, size_t buffer_len);
char *pcan_msgtype_lookup(TPCANMessageType type);
void pcan_dump_timestamp(TPCANTimestamp *timestamp);
char *pcan_parameter_lookup(TPCANParameter parameter);
void pcan_dump_thread_data(thread_data_t *thread_data);

#endif /* _HELPER_PCAN,_H_ */
