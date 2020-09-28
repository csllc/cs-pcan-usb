const events = require('events');
const pcan = require('bindings')('pcan.node');
const tpcan = require('./lib/tpcan');

let channel = 0x51;
let baud = 0x001C; /* 500 kbaud */

const canEmitter = new events.EventEmitter();

var msg = new tpcan.TPCANMsg();
var timestamp = new tpcan.TPCANTimestamp();


process.on('SIGINT', function () {
  console.log("SIGINT received");
  pcan.CAN_DisableEvent(0x51);
  
  pcan.CAN_Reset(0x51);
  pcan.CAN_GetStatus(0x51);

  pcan.CAN_Uninitialize(0x51);
  process.exit(0);
});


channels = pcan.CAN_ChannelInfo();
console.log(channels);

pcan.CAN_Initialize(channel, baud);
pcan.CAN_SetValue(channel, 0x04 /* PCAN_MESSAGE_FILTER */, Buffer.from([0x00]));
pcan.CAN_AcceptanceFilter29Bit(channel, 0x81, 0xF8);
pcan.CAN_GetStatus(channel);

var error = {};
error.buffer = "";
pcan.CAN_GetErrorText(0, 0, error);
console.log(error);


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

//let PCAN_MODE_STANDARD = 0x00;
//let PCAN_MODE_EXTENDED = 0x02;

//buffer = Buffer.alloc(0x01);
//pcan.CAN_GetValue(channel, 0x04 /* PCAN_MESSAGE_FILTER */, buffer);
//console.log(buffer);

//pcan.CAN_FilterMessages(channel, 0, 0x00FFFFFF, PCAN_MODE_EXTENDED);
//pcan.CAN_FilterMessages(channel, 0x00EF8180, 0x00EF8180, PCAN_MODE_EXTENDED);
//pcan.CAN_FilterMessages(channel, 0, 0, PCAN_MODE_EXTENDED);
//buffer = Buffer.from([0x00])
//pcan.CAN_SetValue(channel, 0x04, buffer);

//pcan.CAN_GetValue(channel, 0x04 /* PCAN_MESSAGE_FILTER */, buffer);
//console.log(buffer);

//pcan.CAN_FilterMessages(channel, 0, 0x00EF817F, PCAN_MODE_EXTENDED);
//pcan.CAN_FilterMessages(channel, 0x00EF8181, 0x00FFFFFF, PCAN_MODE_EXTENDED);



pcan.CAN_EnableEvent(channel, function () {
  canEmitter.emit('receive');
});

canEmitter.on('receive', function () {
  pcan.CAN_Read(channel, msg, timestamp);
  console.log(JSON.stringify(msg, (key, value) => {
    if ((typeof value === 'number') && (key == "id")) {
      return value.toString(16).toUpperCase().padStart(8, '0');
    } else if (typeof value === 'number') {
      return value.toString(16).toUpperCase().padStart(2, '0');
    } else {
      return value;
    }
  }));
});



