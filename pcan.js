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
pcan.CAN_GetStatus(0x51);

var msg = new TPCANMsg();
var timestamp = new TPCANTimestamp();
console.log(msg);
console.log(timestamp);

//pcan.CAN_Read(0x51, data_buffer, timestamp_buffer);
pcan.CAN_Read(0x51, msg, timestamp);
//pcan.CAN_Read(0x51, msg, timestamp);

console.log(msg);
console.log(timestamp);

//console.log(data_buffer);
//console.log(timestamp_buffer);

pcan.CAN_Reset(0x51);
pcan.CAN_GetStatus(0x51);

pcan.CAN_Uninitialize(0x51);
