//The following produce output when the native addon is compiled with PCAN_DEBUG defined.

pcan.CAN_GetStatus(channel);

// Filtering messages
let PCAN_MODE_STANDARD = 0x00;
let PCAN_MODE_EXTENDED = 0x02;

pcan.CAN_FilterMessages(channel, 0, 0x00EF817F, PCAN_MODE_EXTENDED);
pcan.CAN_FilterMessages(channel, 0x00EF8181, 0x00FFFFFF, PCAN_MODE_EXTENDED);

pcan.CAN_AcceptanceFilter29Bit(channel, 0x81, 0xF8);


// Setting and getting values
var buffer = Buffer.from([0x56]);
console.log(buffer);
pcan.CAN_SetValue(0x51, 0x01, buffer);

buffer = Buffer.alloc(0x17);
pcan.CAN_GetValue(0x51, 0x0E, buffer); // This API has changed
console.log(buffer);
