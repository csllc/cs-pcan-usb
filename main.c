#include <stdio.h>

#include "PCANBasic.h"
#include "pcan_helper.h"

int main(void) {
    int channel = 0x51;
    TPCANStatus pcan_result;

    pcan_result = CAN_Initialize(channel, 0x001C, 0, 0, 0);
    printf("CAN_Initialize: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));

    pcan_result = CAN_GetStatus(channel);
    printf("CAN_GetStatus: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));
    
    pcan_result = pcan_EnableEvent(channel);
    printf("pcan_EnableEvent: %i \n", pcan_result);

    Sleep(10000);

    pcan_result = pcan_DisableEvent(channel);
    printf("pcan_DisableEvent: %i \n", pcan_result);
    
    pcan_result = CAN_Reset(channel);
    printf("CAN_Reset: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));

    pcan_result = CAN_Uninitialize(channel);
    printf("CAN_Uninitialize: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));

    return 0;
}
    
