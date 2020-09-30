'use strict';

//const events = require('events');
//const process = require('process');


const pcan = require('bindings')('pcan.node');

const { Duplex } = require('stream');

const tpcan = require('./lib/tpcan.js');

// Default configuration used unless overridden by caller
const DEFAULT_TIMEOUT = 2000;

const DEFAULT_OPTIONS = {
  canRate: 500000,
  filters: [],
  loopback: false, // TODO: Implement this
};

module.exports = class PcanUsb extends Duplex {

  constructor(options) {

    super({ objectMode: true});

    this.setOptions(options);
    this.requestQueue = [];
    this.port = null;
    this.isReady = false;
  }

  // Sets (or re-sets) the configuration options
  setOptions(options) {
    // Save for later use
    this.options = Object.assign(DEFAULT_OPTIONS, options);
  }

  // Returns a promise that resolves to a list of available CAN ports
  list() {
    return new Promise(function(resolve, reject) {
      resolve(pcan.CAN_ChannelInfo());
    })
      .then(function(channels) {
	// For compatibility, create a new property 'path' that is the
	// identifier for the CAN device in the system and can be passed
	// to the open() method
	for (var idx in channels) {
	  channels[idx].path = channels[idx].channel_handle;
	}
	return channels;
      })
      .catch(function(err) {
	reject(err);//TODO: Make sure this works as expected
      });
  };

  open(port) {
    var me = this;

    return new Promise(function(resolve, reject) {
      // Initialize CAN port (aka "channel")
      let result = pcan.CAN_Initialize(port, pcan.CAN_TranslateBaud(me.options.canRate));
      if (result != 0) {
	reject(result);
      } else {
	me.port = port;
	me._configure();
	resolve(result);
      }
    })
      .then(function() {
	// Set up data event handler
	me.on('_data', me._onData);

	// Set up close event handler
	me.on('close', function() {
	  pcan.CAN_DisableEvent(me.port);
	  pcan.CAN_Reset(me.port);
	  pcan.CAN_Uninitialize(me.port);
	  
	  me.isReady = false;
	  me.port = null;
	  this.push(null);
	});
      
	// Enable data event
	let result = pcan.CAN_EnableEvent(port, function() {
	  me.emit('_data');
	});
	if (result != 0) {
	  throw new Error("Error at pcan.CAN_EnableEvent.");
	}
	me.isReady = true;

      })
      .catch(function(err) {
	me.emit('error', err);
	throw new Error(err);
      });
  };

  close() {
    let me = this;

    if (this.isOpen()) {
      try {
	me.emit('close');
	pcan.CAN_DisableEvent(me.port);
	pcan.CAN_Reset(me.port);
	pcan.CAN_Uninitialize(me.port);
      } catch (err) {
	me.emit('error', err);
	console.error(err);
      }
    }
    this.port = null;
    this.isReady = false;
  }

  write(msg) {
    let me = this;

    if (msg.length <= 8) {
      let tpcan_buffer = tpcan.toTPCANMsg(msg);
      // Write message to bus
      try {
	pcan.CAN_Write(me.port, tpcan_buffer);
	me.emit('write', mmsg);
      } catch (err) {
	me.emit('error');
	console.error(err);
      }
      // If loopback is enabled, push message back into receive stream
      if (me.options.loopback) {
	me.push(msg);
      }
    } else {
      throw new Error("Tried to send invalid CAN data");
    }
  }

  status() {
    let me = this;

    try {
      return pcan.CAN_Status(me.port);
    } catch (err) {
      me.emit('error', err);
      console.error(err);
    }
  }

  // Required function for cs-modbus GenericConnection
  isOpen() {
    return this.port && this.port.isOpen && this.isReady;
  }

  // Required function for cs-modbus GenericConnection
  isConnected() {
    return this.isOpen();
  }

  _onData() {
    let me = this;
    
    try {
      let read_obj = pcan.CAN_Read(me.port);
      me.push(tpcan.toMsg(read_obj.message)) // Emits 'data' event
    } catch(err) {
      if (err.code != "PCAN_ERROR_QRCVEMPTY") {
	console.error(err.code);
      }
    }
  }

  // required functions for a readable stream, we don't need to do anything
  _read() {}

  // required functions for a writable stream, we don't need to do anything
  _write() {}

  _configure() {
    let me = this;

    // Set filters
    if (me.options.filters.length > 0) {
      // Ensure extended/standard filters aren't being mixed, because this
      // isn't supported by the PCAN-USB hardware.
      let ext_option = me.options.filters[0].ext;

      for (var i in me.options.filters) {
	if (me.options.filters[i].ext != ext_option) {
	  throw new Error("PCAN-USB does not support mixing extended and standard filters.");
	}
      }
      
      for (var i in me.options.filters) {
	if ((me.options.filters[i].code) && (me.options.filters[i].mask)) {
	  if (me.options.filters[i].ext) {
	    pcan.CAN_AcceptanceFilter29Bit(me.port, me.options.filters[i].code,
					   me.options.filters[i].mask);
	    // TODO: Will this thrown exceptions as expected?
	  } else {
	    pcan.CAN_AcceptanceFilter11Bit(me.port, me.options.filters[i].code,
					   me.options.filters[i].mask);
	  }
	} else if ((me.options.filters[i].fromID) && (me.options.filters[i].toID)){

	  pcan.CAN_FilterMessages(me.port, fromID, toID, 0x02);

	} else if (me.options.filters[i].id) {

	  // Split start/stop range and convert them to acceptance code + mask
	  let fromID = parseInt(me.options.filters[i].id.split(' ')[0], 16);
	  let toID = parseInt(me.options.filters[i].id.split(' ')[1], 16);

	  pcan.CAN_FilterMessages(me.port, fromID, toID, 0x02);

	} else {
	  throw new Error("Unknown or unspecified filter format.");
	}
      }
    }
  }
}

