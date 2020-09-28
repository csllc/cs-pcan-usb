/* Defines structures passed to and from the PCAN-Basic API 

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/

'use strict';


function TPCANMsg(id = 0,
		  msgtype = 0,
		  len = 0,
		  data = 0) {
  this.id = id;
  this.msgtype = msgtype;
  this.len = len;
  this.data = data;
};


function TPCANTimestamp(millis = 0,
			millis_overflow = 0, 
			micros = 0) {
  this.millis = millis;
  this.millis_overflow = millis_overflow;
  this.micros = 0;
};


function TPCANMsgFD(id = 0,
		  msgtype = 0,
		  dlc = 0,
		  data = 0) {
  this.id = id;
  this.msgtype = msgtype;
  this.dlc = dlc;
  this.data = data;
};


function TPCANTimestampFD(timestamp = 0) {
  this.timestamp = timestamp;
}


function TPCANChannelInfo(channel_handle = 0,
			  device_type = 0,
			  controller_number = 0,
			  device_features = 0,
			  device_name = "",
			  device_id = 0,
			  channel_condition = 0) {
  
  this.channel_handle = channel_handle;
  this.device_type = device_type;
  this.controller_number = controller_number;
  this.device_features = device_features;
  this.device_name = device_name;
  this.device_id = device_id;
  this.channel_condition = channel_condition;
}


module.exports = {
  TPCANMsg: TPCANMsg,
  TPCANTimestamp: TPCANTimestamp,
  TPCANMsgFD: TPCANMsgFD,
  TPCANTimestampFD: TPCANTimestampFD,
  TPCANChannelInfo: TPCANChannelInfo,
};
