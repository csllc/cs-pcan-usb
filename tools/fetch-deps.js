'use strict';

// Download the vendor libraries needed to build this module into deps/.
//
// Neither library is part of this repository:
//  - Windows: PEAK-System's PCAN-Basic API (header and import library). Its
//    license does not allow redistributing it; PCANBasic.dll itself is
//    installed on users' machines by PEAK's device driver setup.
//  - macOS: UV Software's PCBUSB library. Its license allows redistributing
//    complete, unmodified copies, so the whole release folder is bundled with
//    the macOS prebuilt binary (see tools/bundle-pcbusb.js).
//
// Each download is checked against a pinned SHA-256 hash.
//
// Usage: node tools/fetch-deps.js [--all]
//   Fetches the library for the current platform, or both with --all
//   (useful for editor support on either platform).

const crypto = require('crypto');
const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');

const DEPS = path.join(__dirname, '..', 'deps');

const PACKAGES = {
  win32: {
    name: 'PEAK PCAN-Basic API',
    // PEAK only publishes the latest version, at a fixed URL. When PEAK
    // updates it, the hash check below fails; check the new version, then
    // update the version and hash here.
    version: '5.1.0.1194',
    url: 'https://www.peak-system.com/fileadmin/media/files/PCAN-Basic.zip',
    sha256: '22745ffb13e7e96185de8003cd93f2b3b62b8bfb939efdf6954cdd5ed2c28852',
    archive: 'PCAN-Basic.zip',
    dir: 'pcan-basic',
    // Unpack into deps/pcan-basic (the ZIP has no top-level folder)
    unpackInto: 'pcan-basic',
    check: path.join('pcan-basic', 'Include', 'PCANBasic.h'),
    overrideVar: 'PCAN_BASIC_SDK',
  },
  darwin: {
    name: 'UV Software PCBUSB library',
    version: '0.11.1',
    url: 'https://github.com/mac-can/PCBUSB-Library/releases/download/v0.11.1/macOS_Library_for_PCANUSB_v0.11.1.tar.gz',
    sha256: '3160d1f5200151e3c6541c8c7994f6a8859b59d2b971912ca8c85c131af90a22',
    archive: 'PCBUSB-0.11.1.tar.gz',
    dir: 'pcbusb',
    // The tarball has a top-level PCBUSB/ folder, so this gives deps/pcbusb/PCBUSB
    unpackInto: 'pcbusb',
    check: path.join('pcbusb', 'PCBUSB', 'PCBUSB.h'),
    overrideVar: 'PCBUSB_DIR',
  },
};

function fail(msg) {
  console.error(`fetch-deps: ${msg}`);
  process.exit(1);
}

async function fetchPackage(platform) {
  const pkg = PACKAGES[platform];
  const archivePath = path.join(DEPS, pkg.archive);
  const unpackDir = path.join(DEPS, pkg.unpackInto);

  if (fs.existsSync(path.join(DEPS, pkg.check))) {
    console.log(`fetch-deps: ${pkg.name} ${pkg.version} already in deps/${pkg.dir}`);
    return;
  }

  fs.mkdirSync(DEPS, { recursive: true });

  console.log(`fetch-deps: downloading ${pkg.name} ${pkg.version} from ${pkg.url}`);
  const res = await fetch(pkg.url);
  if (!res.ok) {
    fail(`download failed: HTTP ${res.status} ${res.statusText}`);
  }
  const data = Buffer.from(await res.arrayBuffer());

  const hash = crypto.createHash('sha256').update(data).digest('hex');
  if (hash !== pkg.sha256) {
    fail(`${pkg.name}: SHA-256 mismatch.\n` +
      `  expected ${pkg.sha256}\n` +
      `  got      ${hash}\n` +
      (platform === 'win32'
        ? 'PEAK may have published a new PCAN-Basic version. Check it, then update the\n' +
          'version and hash in tools/fetch-deps.js. '
        : '') +
      `Alternatively, set ${pkg.overrideVar} to a folder containing the library.`);
  }
  fs.writeFileSync(archivePath, data);

  fs.rmSync(unpackDir, { recursive: true, force: true });
  fs.mkdirSync(unpackDir, { recursive: true });

  // bsdtar (built into Windows 10+ and macOS) unpacks both .zip and .tar.gz
  const tar = spawnSync('tar', [ '-xf', archivePath, '-C', unpackDir ], { stdio: 'inherit' });
  if (tar.status !== 0) {
    fail(`could not unpack ${archivePath}`);
  }
  fs.rmSync(archivePath);

  if (!fs.existsSync(path.join(DEPS, pkg.check))) {
    fail(`${pkg.name}: unpacked, but ${pkg.check} is missing`);
  }
  console.log(`fetch-deps: ${pkg.name} ${pkg.version} unpacked to deps/${pkg.dir}`);
}

async function main() {
  const platforms = process.argv.includes('--all')
    ? Object.keys(PACKAGES)
    : [ process.platform ];

  for (const platform of platforms) {
    if (!PACKAGES[platform]) {
      fail(`${platform} is not supported (only Windows and macOS)`);
    }
    await fetchPackage(platform);
  }
}

main().catch((err) => fail(err.stack || String(err)));
