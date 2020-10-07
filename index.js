'use strict';

const binary = require('node-pre-gyp');
const path = require('path');
const pcan_path = binary.find(path.resolve(path.join(__dirname,'./package.json')));
const pcan = require(pcan_path);

const { Duplex } = require('stream');
const tpcan = require('./lib/tpcan.js');

// Default configuration used unless overridden by caller
const DEFAULT_OPTIONS = {
  canRate: 250000,
  filters: [],
  loopback: false,
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
    var me = this;

    return new Promise(function(resolve) {
      if (process.platform == "win32") {
        // If running on Windows, get channel data through the ChannelInfo method
        resolve(pcan.ChannelInfo());
      } else if (process.platform == "darwin") {
        // If running on macOS, we fake it by resolving with an object we create
        resolve([{channel_handle: 0x51}]);
      }
      
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
        me.emit('error', err);
        throw new Error(err);
      });
  }

  open(port) { 
    var me = this;

    return new Promise(function(resolve) {
      // Initialize CAN port (aka "channel")
      pcan.Initialize(port, pcan.TranslateBaud(me.options.canRate));

      me.port = port;
      me._configure();
      me.isReady = true;
      me.emit('open');
      resolve();
    })
      .then(function() {
        // Close event handler
        me.on('close', function() {
          me.close();
        });

        // Data receive event handler
        me.on('_data', me._onData.bind(me));
        
        // Enable data event
        pcan.EnableEvent(port, function() {
          me.emit('_data');
        });

        // Enable busoff auto-reset
        if (process.platform == "win32") {
          pcan.SetValue(port, 0x07, Buffer.from([0x01]));
        }

      })
      .catch(function(err) {
        me.emit('error', err);
        throw new Error(err);
      });
  }

  close() {
    let me = this;

    return new Promise(function(resolve) {
      // Remove listeners
      me.removeAllListeners();

      if (me.isOpen()) {
        pcan.DisableEvent(me.port);
        pcan.Reset(me.port);
        pcan.Uninitialize(me.port);

      }
      // Close stream
      me.emit('close');
      resolve();

    })
      .then(function() {
        me.port = null;
        me.isReady = false;
        me.push(null);
      })
      .catch(function(err) {
        me.emit('error', err);
        throw new Error(err);
      });
  }

  write(msg) {
    let me = this;
    
    return new Promise(function(resolve) {
      if (msg.buf.length <= 8) {
        let tpcan_buffer = tpcan.toTPCANMsg(msg);
        // Write message to bus
        pcan.Write(me.port, tpcan_buffer);
        me.emit('write', msg);
        // If loopback is enabled, push message back into receive stream
        if (me.options.loopback) {
          me.push(tpcan.toMsg(tpcan_buffer));
        }
        resolve();
      } else {
        throw new Error("Tried to send invalid CAN data");
      }
    })
      .catch(function(err) {
        me.emit('error', err);
        throw new Error(err);
      });
  }

  status() {
    let me = this;

    return new Promise(function(resolve) {
      let status = pcan.Status(me.port);
      resolve(status);
    })
      .catch(function(err) {
        me.emit('error', err);
        throw new Error(err);
      });
  }

  // Required function for cs-modbus GenericConnection
  isOpen() {
    return this.port && this.isReady;
  }

  // Required function for cs-modbus GenericConnection
  isConnected() {
    return this.isOpen();
  }

  _onData() {
    let me = this;
    
    try {
      let read_obj = pcan.Read(me.port);
      me.push(tpcan.toMsg(read_obj.message)) // Emits 'data' event
    } catch(err) {
      if (err.code != "PCAN_ERROR_QRCVEMPTY") {
        console.error(err.code);
      }
    }
  }

  // Required for a readable stream; we don't need to do anything
  _read() {}

  // Required for a writable stream; we don't need to do anything
  _write() {}

  _configure() {
    let me = this;

    // Set filters
    if (me.options.filters.length > 0) {
      // Ensure extended/standard filters aren't being mixed, because this
      // isn't supported by the PCAN-USB hardware.
      let ext_option = me.options.filters[0].ext;

      for (let i in me.options.filters) {
        if (me.options.filters[i].ext != ext_option) {
          throw new Error("PCAN-USB does not support mixing extended and standard filters.");
        }
      }
      
      for (let i in me.options.filters) {
        if ((me.options.filters[i].code) && (me.options.filters[i].mask)) {
          if (me.options.filters[i].ext) {
            pcan.AcceptanceFilter29Bit(me.port, me.options.filters[i].code,
                                       me.options.filters[i].mask);
          } else {
            pcan.AcceptanceFilter11Bit(me.port, me.options.filters[i].code,
                                       me.options.filters[i].mask);
          }
        } else if ((me.options.filters[i].fromID) && (me.options.filters[i].toID)){

          pcan.FilterMessages(me.port, me.options.filters[i].fromID,
                              me.options.filters[i].toID, 0x02);

        } else if (me.options.filters[i].id) {

          // Split start/stop range and convert them to acceptance code + mask
          let fromID = parseInt(me.options.filters[i].id.split(' ')[0], 16);
          let toID = parseInt(me.options.filters[i].id.split(' ')[1], 16);

          pcan.FilterMessages(me.port, fromID, toID, 0x02);

        } else {
          throw new Error("Unknown or unspecified filter format.");
        }
      }
    }
  }
}

