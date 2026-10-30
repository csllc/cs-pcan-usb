'use strict';

// Print the location of the vendor library for binding.gyp.
//
// Usage: node tools/pcan-sdk.js include|lib
//   include: the folder containing the library's header file
//   lib:     the full path of the library file to link against
//
// Windows: the PEAK PCAN-Basic API, from the PCAN_BASIC_SDK environment
//   variable (a folder with PEAK's PCAN-Basic.zip unpacked into it), or else
//   deps/pcan-basic (see `npm run fetch-deps`).
// macOS: the UV Software PCBUSB library, from the PCBUSB_DIR environment
//   variable (a folder with the PCBUSB release unpacked into it), or else
//   deps/pcbusb/PCBUSB (see `npm run fetch-deps`), or else the library
//   installed by PCBUSB's install.sh in /usr/local.

const fs = require('fs');
const path = require('path');

const ROOT = path.join(__dirname, '..');
const what = process.argv[2];

function fail(lines) {
  for (const line of lines) {
    console.error(`cs-pcan-usb: ${line}`);
  }
  process.exit(1);
}

function locate() {
  if (process.platform === 'win32') {
    const sdk = process.env.PCAN_BASIC_SDK || path.join(ROOT, 'deps', 'pcan-basic');
    const found = {
      include: path.join(sdk, 'Include'),
      lib: path.join(sdk, 'x64', 'VC_LIB', 'PCANBasic.lib'),
    };
    if (!fs.existsSync(path.join(found.include, 'PCANBasic.h')) || !fs.existsSync(found.lib)) {
      fail([
        `PEAK PCAN-Basic API not found in ${sdk}.`,
        'Run `npm run fetch-deps` to download it, or download PCAN-Basic.zip from',
        'https://www.peak-system.com/PCAN-Basic.239.0.html, unpack it, and set the',
        'PCAN_BASIC_SDK environment variable to that folder.',
      ]);
    }
    return found;
  }

  if (process.platform === 'darwin') {
    const candidates = [];
    if (process.env.PCBUSB_DIR) {
      candidates.push({ include: process.env.PCBUSB_DIR, libDir: process.env.PCBUSB_DIR });
    }
    candidates.push({ include: path.join(ROOT, 'deps', 'pcbusb', 'PCBUSB'),
      libDir: path.join(ROOT, 'deps', 'pcbusb', 'PCBUSB') });
    candidates.push({ include: '/usr/local/include', libDir: '/usr/local/lib' });

    for (const c of candidates) {
      if (!fs.existsSync(path.join(c.include, 'PCBUSB.h')) || !fs.existsSync(c.libDir)) {
        continue;
      }
      // The release folder has only the versioned file (libPCBUSB.<version>.dylib);
      // install.sh also adds an unversioned symlink
      const lib = fs.readdirSync(c.libDir)
        .filter((f) => /^libPCBUSB(\.[0-9.]+)?\.dylib$/.test(f))
        .sort()[0];
      if (lib) {
        return { include: c.include, lib: path.join(c.libDir, lib) };
      }
    }
    fail([
      'UV Software PCBUSB library not found.',
      'Run `npm run fetch-deps` to download it, or install it from',
      'https://github.com/mac-can/PCBUSB-Library (its install.sh puts it in /usr/local).',
    ]);
  }

  // Not supported on other platforms; print nothing so that gyp can still
  // evaluate binding.gyp
  process.exit(0);
}

if (what !== 'include' && what !== 'lib') {
  fail([ 'usage: node tools/pcan-sdk.js include|lib' ]);
}

// gyp prefers forward slashes
process.stdout.write(locate()[what].replace(/\\/g, '/'));
