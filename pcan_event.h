#ifndef _PCAN_EVENT_H_
#define _PCAN_EVENT_H_

#include "PCANBasic.h"

int pcan_EnableEvent(TPCANHandle pcan_Channel, void *callback);
int pcan_DisableEvent(TPCANHandle pcan_Channel);

#endif /* _PCAN_EVENT_H_ */
