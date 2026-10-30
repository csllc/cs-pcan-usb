'use strict';

// macOS only: copy the complete PCBUSB release folder next to the built
// addon, as <addon folder>/PCBUSB/. The addon looks for libPCBUSB there first
// (rpath @loader_path/PCBUSB), then in /usr/local/lib, where PCBUSB's own
// install.sh puts it.
//
// PCBUSB's license (see PCBUSB/LICENSE) allows redistributing it, provided
// each copy is a true and complete copy that includes all copyright and
// trademark notices and the license itself. So the whole release folder is
// copied unchanged; don't remove or modify anything in it.
//
// Usage: node tools/bundle-pcbusb.js [--prebuilds] [folder...]
//   --prebuilds  copy into every prebuilds/darwin-* folder, and fail if the
//                release folder isn't available (used when publishing)
//   folder...    copy into these folders (e.g. build/Release); if the release
//                folder isn't available, print a note and skip

const fs = require('fs');
const path = require('path');

const ROOT = path.join(__dirname, '..');

// Files that identify a complete PCBUSB release folder
const RELEASE_FILES = [ 'LICENSE', 'COPYRIGHT', 'README', 'PCBUSB.h', 'install.sh' ];

if (process.platform !== 'darwin') {
  process.exit(0);
}

const args = process.argv.slice(2);
const prebuilds = args.includes('--prebuilds');
let targets = args.filter((a) => a !== '--prebuilds').map((a) => path.resolve(ROOT, a));

if (prebuilds) {
  const dir = path.join(ROOT, 'prebuilds');
  const found = fs.existsSync(dir)
    ? fs.readdirSync(dir).filter((d) => d.startsWith('darwin-')).map((d) => path.join(dir, d))
    : [];
  if (found.length === 0) {
    console.error('bundle-pcbusb: no prebuilds/darwin-* folder; run prebuildify first');
    process.exit(1);
  }
  targets = targets.concat(found);
}

const src = process.env.PCBUSB_DIR || path.join(ROOT, 'deps', 'pcbusb', 'PCBUSB');
const missing = RELEASE_FILES.filter((f) => !fs.existsSync(path.join(src, f)));
const hasLib = fs.existsSync(src) &&
  fs.readdirSync(src).some((f) => /^libPCBUSB\.[0-9.]+\.dylib$/.test(f));

if (missing.length > 0 || !hasLib) {
  const msg = `PCBUSB release folder not found in ${src}` +
    (missing.length ? ` (missing ${missing.join(', ')})` : '') + '.';
  if (prebuilds) {
    console.error(`bundle-pcbusb: ${msg} Run \`npm run fetch-deps\` first.`);
    process.exit(1);
  }
  console.log(`bundle-pcbusb: ${msg} Not bundling it; the addon will use the copy in /usr/local/lib.`);
  process.exit(0);
}

for (const target of targets) {
  const dest = path.join(target, 'PCBUSB');
  fs.rmSync(dest, { recursive: true, force: true });
  fs.cpSync(src, dest, { recursive: true, preserveTimestamps: true });
  console.log(`bundle-pcbusb: copied ${src} to ${path.relative(ROOT, dest)}`);
}
