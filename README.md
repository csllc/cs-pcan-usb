# NodeJS PCAN-USB Module

This module provides a NodeJS interface to the [PEAK-System](https://www.peak-system.com/) [PCAN-USB](https://www.peak-system.com/PCAN-USB.199.0.html?&L=1) adapter, and possibly others that are supported by the [PCAN-Basic library](https://www.peak-system.com/PCAN-Basic.239.0.html?&L=1).

This module is meant to be compatible with a wrapper like @clssc/cs-canbus-universal, providing application support for PCAN-USB adapters.

This module uses native code through Node-API and supports Windows x86_64 and macOS (Intel and Apple Silicon). Node 24 or later is recommended.

## Getting Started

The following assumes that [NodeJS](https://nodejs.org) is already installed.

On **Windows**, install the [PEAK-System device driver package](https://www.peak-system.com/support/downloads/drivers/driver-packages/), including the PCAN-Basic API. It provides `PCANBasic.dll`, which is not distributed with this module.

On **macOS**, nothing else is needed: the [PCBUSB library](https://github.com/mac-can/PCBUSB-Library) is bundled with the prebuilt binary.

To install this module, run:
```
npm install @csllc/cs-pcan-usb
```

If the PCAN library can't be loaded (for example, the PEAK driver is not installed), the module still loads: `list()` returns an empty list, and other calls fail with an error whose `code` is `ERR_PCAN_NOT_INSTALLED`. This lets applications that support several kinds of CAN adapter keep working on machines without PCAN hardware.

The following is a sample script that opens the CAN port, writes a single message to the bus, and listens for and prints received messages from the bus.

```js
const Can = require('@csllc/cs-pcan-usb');

let can = new Can({
  canRate: 250000,
});

can.list()
  .then((ports) => {
    console.log(ports);
    return can.open(ports[0].path)
      .catch((err) => {
	console.error(err);
	process.exit(1);
      });
  })
  .then(() => {
    can.write({ id: 0x10EF8001,
                ext: true,
                buf: Buffer.from([0x45, 0x00, 0x00, 0x04, 0x00, 0x00]) });
  })
  .catch((err) => {
    console.error(err);
    can.close();
    process.exit(1);
  });


can.on('data', function(msg) {
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

```

## Examples

Both JavaScript and Win32 examples are available in the `example` folder.

## Configuration

The constructor accepts an object that specifies the desired configuration. The CAN adapter is set up before the `open` method resolves, so once it is complete, the CAN interface is ready for use.

The default options are shown in the following example. Any of them may be omitted from the constructor's option object to use their default value.

```js
let can = new Can({

  // bit rate on the CAN bus
  canRate: 250000,

  // filters for incoming packets
  filters: [
  ],

  // useful for testing, each sent packet is also received
  loopback: false,
  });
```

### Streaming

`cs-pcan-usb` extends the NodeJS stream interface, so it can be piped into other stream instances.

### Filtering

> :warning: Due to limitations of MacCAN and PCBUSB, CAN message filtering is not supported on macOS.

By default, all CAN message are captured, but this can be limited by applying a message filter. The following are examples of filter definitions that are passed to the `open` method.

```js
{
  ext: true, // Applies to extended (29-bit IDs)
  id: '10EF0000 10EFFFFF', // Allow IDs 0x10EF0000 through 0x10EFFFFF
}

{
  ext: true, // Applies to extended (29-bit IDs)
  fromID: 0x10EF0000, // Allow IDs 0x10EF0000 through 0x10EF7777
  toID: 0x10EF7777,
}

{
  ext: false, // Applies to standard (11-bit IDs)
  code: 0x081, // Acceptance code
  mask: 0x0F8, // Acceptance mask
}

```

The PCAN-USB adapter supports one message filter at a time. If multiple filters are applied, the resulting filter spans the range of the highest to lowest IDs of all filters.

Three filter formats are supported by the package:

 - `code` and `mask` format, directly specifying the acceptance filter with a code and mask

        `code: 0x10EF0000, mask: 0xFFFF0000`

 - `fromID` and `toID` format, specifying a range of IDs to be accepted by the filter:

        `fromID: 0x10EF0000, toID: 0x10EFFFFF`

 - `id` format, specifying a string with a range of IDs to be accepted by the filter:

        `id: '10EF0000 10EFFFFF'`

When using the `start`/`stop` or `id` formats, it is not guaranteed that all messages outside of the specified range will be filtered out. This is due to a hardware limitation reported in the PCAN-Basic documentation.

Given a range of IDs to allow, it is possible to convert to an acceptance code and mask pair:

```
acceptance_code = fromID & toID
acceptance_mask = ~(fromID ^ toID)
```

#### Mixing Filter Types

Internally, the PCAN-USB adapter does not differentiate between standard and extended ID filtering. The `ext` property is only made available for compatibility with `can-usb-com` and `cs-canlib`. 

As such, a filer specification like

```js
{ ext: true, id: '00000000 FFFFFFFFF' }
```

will allow all messages with standard IDs (`000` through `7FF`) through to the receive buffer. This **is not** consistent with `can-usb-com` or `cs-canlib`, which follow [CAN-USB-COM's specification](https://gridconnect.box.com/shared/static/c7l5u2vgsid1a24y6r0a0wv5hwaunr71.pdf):
 - When [filtering is] disabled, all CAN messages are received regardless of the definitions in the list
 - When [filtering is] enabled, only messages which match filter definitions in the list will be passed through.


### JavaScript Events

The module emits the following events:

 - `open` when the serial port is successfully opened
 - `error` if an error occurs
 - `data` when an incoming CAN bus frame is received
 - `write` when an outgoing CAN bus frame is sent to the device (the event is emitted before the frame is actually sent on the bus)
 - `close` when the port is closed

To listen for the events, use the typical NodeJS EventEmitter pattern:

```js
  can.on('open', function(){
    console.log( 'CAN bus opened');
  })

  can.on('data', function(msg){
    console.log( msg );
  })

  can.on('close', function(err){
    if( err && err.disconnected ) {
      console.log( 'CAN bus disconnected' );
    }
    else {
      console.log( 'CAN bus closed by application' );
    }
  })

  can.on('error', function(err){
    console.log( 'CAN bus error: ', err );
  })
```

### Asynchronous Message Reception and Callbacks

This module takes advantage of the asynchronous message receive notification offered by the PCAN-Basic API. It implements a 'light' callback function in the native code, which simply calls the JavaScript function provided when the event is enabled using `pcan.EventEnable()`. The JavaScript function is then expected to call `pcan.Read()` to retrieve the newly received message, which uses the API's `CAN_Read()` function behind-the-scenes.

In the future, it may make sense to move the call to `CAN_Read()` directly into the native code to improve performance.

#### Win32 Events

The Windows version of this module provides a Win32 event to the PCAN-Basic API through the `CAN_SetValue(PCAN_RECEIVE_EVENT)` API call, which gets signaled upon receipt of a CAN message that is accepted by the message filter. A Win32 thread waits on the event using `WaitForMultipleObjects`, invokes a callback function when signaled, and resets itself for the next message.

#### Unix Pipes

Under the MacCAN PCBUSB library, this module uses the read-end of a POSIX pipe, received through the `CAN_GetValue(PCAN_RECEIVE_EVENT)` API call. A POSIX thread waits for data to be written to the pipe using `select(2)`, invokes a callback function when signaled, and resets itself for the next message.

## API

This module's API functions generally return Promises, which are resolved or rejected when a request is complete. 

Refer to the [documentation on Promises](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise) for details on how to chain requests together, detect errors, etc. 

Refer to the PCAN-Basic documentation for details on the full API that this module implements.

## Development

Before updating or modifying this package, please

 - Lint any changes using `eslint`.
 - Confirm that unit tests pass, noting the exceptions described in the next section.

In order to run unit tests (`npm test`), at least one PCAN-USB device must be connected to the computer.

The `pingpong` test is currently configured to use `cs-pcan-usb` for one adapter, and `can-usb-com` for the other. These must be connected via a properly terminated bus.

Do not run any tests on a bus with active traffic, since receiving unexpected CAN packets may confuse the tests.

### Build Environment Setup

The vendor libraries needed to build the module are not part of this repository. `npm run fetch-deps` downloads them from their publishers into `deps/` (which git ignores), checking each against a pinned SHA-256 hash:

 - Windows: the [PEAK PCAN-Basic API](https://www.peak-system.com/PCAN-Basic.239.0.html) (header and import library). PEAK only publishes the latest version at a fixed URL, so when PEAK releases a new version the hash check fails. Check the new version, then update the version and hash in `tools/fetch-deps.js`. Alternatively, unpack `PCAN-Basic.zip` yourself and set the `PCAN_BASIC_SDK` environment variable to that folder.
 - macOS: the [PCBUSB library](https://github.com/mac-can/PCBUSB-Library) release, pinned to version 0.11.1. Alternatively, set the `PCBUSB_DIR` environment variable to an unpacked PCBUSB release folder, or install PCBUSB with its own `install.sh`, which puts it in `/usr/local`.

#### Windows x86_64

 1. Install NodeJS 24 or later. Ensure that `node` and `npm` are found when invoked through the Command Prompt.
 1. Install the native build tools, either by checking "Automatically install the necessary tools" in the NodeJS installer, or manually:
    - Visual Studio (or Visual Studio Build Tools) with the "Desktop development with C++" workload
    - Python 3
 1. Install the [PEAK-System device driver package](https://www.peak-system.com/support/downloads/drivers/driver-packages/), including the PCAN-Basic API, to run the module and its tests.
 1. Clone the `cs-pcan-usb` repository:
    `git clone https://github.com/csllc/cs-pcan-usb`
 1. Download the PCAN-Basic API, then install dependencies (which also builds the module):
    - `cd cs-pcan-usb\`
    - `npm run fetch-deps`
    - `npm install`
 1. Build:
    - `npm run rebuild`
 1. Test:
    - `npm test`

#### macOS (Intel or Apple Silicon)

 1. Install NodeJS 24 or later.
 1. Install the Xcode Command Line Tools: `xcode-select --install`
 1. Clone the `cs-pcan-usb` repository:
    `git clone https://github.com/csllc/cs-pcan-usb`
 1. Download the PCBUSB library, then install dependencies (which also builds the module):
    - `cd cs-pcan-usb`
    - `npm run fetch-deps`
    - `npm install`
 1. Build:
    - `npm run rebuild`
 1. Test:
    - `npm test`

#### Build Notes

The module is loaded with [`node-gyp-build`](https://github.com/prebuild/node-gyp-build), which uses a prebuilt binary from `prebuilds/<platform>-<arch>/` if one is present and otherwise falls back to `build/Release/`. When working on the C code, delete the `prebuilds/` folder (if you made one) so that your local build is the one that gets loaded.

On macOS, `npm run build` and `npm run rebuild` copy the complete PCBUSB release folder into `build/Release/PCBUSB/`. The addon looks for `libPCBUSB` there first, then in `/usr/local/lib`.

If the test stage fails, useful error messages may be hidden by `mocha`. When tracking down the cause of an error, running the example code directly, for example `node example/js/test.js`, may be helpful.

It may also be helpful to recompile the module with `PCAN_DEBUG` (and, on macOS, `PCAN_EVENT_DARWIN_DEBUG`) defined to enable debugging messages.

#### Editor Support

A `compile_flags.txt` is included for [clangd](https://clangd.llvm.org/). It uses the Node-API headers from the `node-api-headers` dev dependency and the vendor headers in `deps/`, so run `npm run fetch-deps -- --all` and `npm install` first.

## Distribution

The module uses only Node-API, so a single binary works with every supported NodeJS version. The published npm package includes prebuilt binaries in `prebuilds/`. If no prebuilt binary matches, `npm install` falls back to building from source, which requires the build environment described above.

The macOS prebuilt binaries include the complete, unmodified PCBUSB release folder in `prebuilds/darwin-*/PCBUSB/`. If you bundle this module into an application (for example with Electron), keep that whole folder next to the `.node` file, outside of any `.asar` archive: PCBUSB's license only allows redistributing complete copies, including its `LICENSE` file.

To make the prebuilt binary locally (after `npm run fetch-deps`):

 - `npm run prebuildify`

## Functionality Limitations

### Questionable Functionality

The following have been observed as potential sources of unexpected behavior in the future.

 - No error is thrown when the PCAN-USB dongle is unplugged while the module is running.
 - The PCAN-Basic API does not provide the means to enable a CAN adapter's loopback ("self-receive" or "echo") feature. This is instead simulated in `cs-pcan-usb` by pushing data passed to the write method back into the receive data stream. 
 - As a result of the above, the expected outcome of the test units (`npm test`) is that all pass except those in `filter.test.js` that test `should.not.emit('data')`. This does not indicate that the filter functionality does not work.
 - The macOS build of this addon does not support filtering at all, and any attempts to use `pcan.FilterMessages`, `pcan.AcceptanceFilter11Bit`, and `pcan.AcceptanceFilter29Bit` will fail silently.
 - The PCAN-USB adapter does not differentiate between extended and standard filters; see the notes under Filtering, above.

### Untested Functionality

The following have not been tested due to hardware availability.
	
 - All FD functions (`CAN_InitializeFD`, `CAN_ReadFD`, `CAN_WriteFD`)

## Licensing and Dependencies

This project is licensed under the MIT license - see the [LICENSE](LICENSE) file for details.

It uses the following third-party components:

* [PEAK PCAN-Basic API](https://www.peak-system.com/PCAN-Basic.239.0.html) - Copyright PEAK-System Technik GmbH, Germany. On Windows, the module is compiled against PCAN-Basic's header file and import library, and uses `PCANBasic.dll` at runtime. These are subject to PEAK-System's license and are not included in this repository or in the npm package. Install the PEAK-System device driver package to use the module, and see Build Environment Setup to build it.
* [PCBUSB library](https://github.com/mac-can/PCBUSB-Library) - Copyright UV Software, Berlin. On macOS, the module is compiled against PCBUSB and uses `libPCBUSB` at runtime. PCBUSB is not included in this repository. As its license permits, a complete, unmodified copy of the PCBUSB release, including its license, is included with the macOS prebuilt binaries in the npm package.
