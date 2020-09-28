'use strict';

//const events = require('events');
//const process = require('process');


const pcan = require('bindings')('pcan.node');

const { Duplex } = require('stream');

const tpcan = require('lib/tpcan.js');

// Default configuration used unless overridden by caller
const DEFAULT_TIMEOUT = 2000;

const DEFAULT_OPTIONS = {
  canRate: 250000,
  samplePoint: 75, //What is this used for, and is it still relevant?
  filters: [],
  loopback: false,
};

module.exports = class PcanUsb extends Duplex {
  constructor(options) {
    super({ objectMode: true});

    this.setOptions(options);

    this.requestQueue = [];

    this.port = null;

    this.isRead = false;

    this._rxData = '';
  }

  // Sets (or re-sets) the configuration options
  setOptions(options) {
    // Save for later use
    this.options = Object.assign(DEFAULT_OPTIONS, options);
  }

  // Returns a promise that resolves to a list of available CAN ports
  list() {
    
  }


}

