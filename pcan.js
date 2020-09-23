function TPCANMsg(id = 0,
		  msgtype = 0,
		  len = 0,
		  data = 0) {
  this.id = id;
  this.msgtype = msgtype;
  this.len = len;
  this.data = data;
}

function TPCANTimestamp(millis = 0,
			millis_overflow = 0, 
			micros = 0) {
  this.millis = millis;
  this.millis_overflow = millis_overflow;
  this.micros = 0;
}

var pcan = require('bindings')('pcan.node')

pcan.CAN_GetStatus(0x51);
pcan.CAN_Initialize(0x51, 0x001C); /* 500kbaud */

/*
var msg = new TPCANMsg();
var timestamp = new TPCANTimestamp();
pcan.CAN_Read(0x51, msg, timestamp);

console.log(msg);
console.log(timestamp);

for (var byte in msg.data) {
  msg.data[byte] += 1;
}

pcan.CAN_Write(0x51, msg);

var buffer = Buffer.from([0x56]);
console.log(buffer);
pcan.CAN_SetValue(0x51, 0x01, buffer);

buffer = Buffer.alloc(0x17);
pcan.CAN_GetValue(0x51, 0x0E, buffer);
console.log(buffer);
*/

pcan.CAN_EnableEvent(0x51);

for (;;) {}

pcan.CAN_DisableEvent(0x51);




pcan.CAN_Reset(0x51);
pcan.CAN_GetStatus(0x51);

pcan.CAN_Uninitialize(0x51);
