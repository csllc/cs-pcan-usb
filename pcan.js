var pcan = require('bindings')('pcan.node')

console.log("pcan.js: test");

status = pcan.CAN_initialize(0x2093234, 0x0014);
console.log("status: " + status);
