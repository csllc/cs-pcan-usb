#include <stdio.h>
#include <stdint.h>

#include "PCANBasic.h"
#include "pcan_helper.h"
#include "main.h"

int main(void)
{
    int channel = 0x51;
    TPCANStatus pcan_result;

    pcan_result = CAN_Initialize(channel, 0x001C, 0, 0, 0);
    printf("CAN_Initialize: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));

    uint32_t pcan_AcceptanceCode = 0x81;
    uint32_t pcan_AcceptanceMask = 0xF8;

    uint64_t pcan_AcceptanceFilter = 0;
    pcan_AcceptanceFilter |= ((uint64_t)pcan_AcceptanceCode << 0x20);
    pcan_AcceptanceFilter |= ((uint64_t)(~pcan_AcceptanceMask));

    printf("pcan_AcceptanceFilter = %016llX\n", pcan_AcceptanceFilter);

    pcan_result = CAN_SetValue(channel, PCAN_ACCEPTANCE_FILTER_29BIT,
			       &pcan_AcceptanceFilter, sizeof(pcan_AcceptanceFilter));
    printf("CAN_SetValue (PCAN_ACCEPTANCE_FILTER_29BIT): 0x%02X (%s)\n",
	   pcan_result, pcan_status_lookup(pcan_result));

    pcan_result = CAN_GetStatus(channel);
    printf("CAN_GetStatus: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));
    
    pcan_result = pcan_EnableEvent(channel, &callback);
    printf("pcan_EnableEvent: %i \n", pcan_result);

    Sleep(10000); /* Run for 10 seconds before quitting */

    pcan_result = pcan_DisableEvent(channel);
    printf("pcan_DisableEvent: %i \n", pcan_result);
    
    pcan_result = CAN_Reset(channel);
    printf("CAN_Reset: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));

    pcan_result = CAN_Uninitialize(channel);
    printf("CAN_Uninitialize: 0x%02X (%s)\n", pcan_result, pcan_status_lookup(pcan_result));

    return 0;
}
    



void callback(int channel)
{
    TPCANStatus status = PCAN_ERROR_OK;
    TPCANMsg msg = { 0 };
    TPCANTimestamp timestamp = { 0 };
    
    do {
	ZeroMemory(&msg, sizeof(msg));
	ZeroMemory(&timestamp, sizeof(timestamp));

	status = CAN_Read(channel, &msg, &timestamp);

	if (status == PCAN_ERROR_OK) {
	    //printf("---\n");
	    pcan_dump_msg(&msg);
	    //pcan_dump_timestamp(&timestamp);
	}
    } while (status == PCAN_ERROR_OK);
    
    return;
}

