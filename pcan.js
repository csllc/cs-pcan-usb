const events = require('events');
const process = require('process');

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

let channel = 0x51;
let baud = 0x001C; /* 500 kbaud */

var pcan = require('bindings')('pcan.node');

const canEmitter = new events.EventEmitter();

var msg = new TPCANMsg();
var timestamp = new TPCANTimestamp();

/*
process.on('SIGINT', function () {
  console.log("SIGINT received");
  pcan.CAN_DisableEvent(0x51);
  
  pcan.CAN_Reset(0x51);
  pcan.CAN_GetStatus(0x51);

  pcan.CAN_Uninitialize(0x51);
  process.exit(0);
});
*/

pcan.CAN_Initialize(channel, baud);
pcan.CAN_GetStatus(channel);

/*
pcan.CAN_Read(channel, msg, timestamp);

console.log(msg);
console.log(timestamp);

for (var byte in msg.data) {
  msg.data[byte] += 1;
}

pcan.CAN_Write(0x51, msg);
*/

/*
var buffer = Buffer.from([0x56]);
console.log(buffer);
pcan.CAN_SetValue(0x51, 0x01, buffer);

buffer = Buffer.alloc(0x17);
pcan.CAN_GetValue(0x51, 0x0E, buffer);
console.log(buffer);
*/


pcan.CAN_EnableEvent(channel, function () {
  canEmitter.emit('receive');
});

canEmitter.on('receive', function () {
  pcan.CAN_Read(channel, msg, timestamp);
  //console.log(msg);
});

