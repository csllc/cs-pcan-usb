{
  "variables": {
    # Vendor library locations (see tools/pcan-sdk.js). Neither library is
    # part of this repository; run `npm run fetch-deps` to download them.
    "pcan_include": "<!(node tools/pcan-sdk.js include)",
    "pcan_lib": "<!(node tools/pcan-sdk.js lib)"
  },
  "targets": [
    {
      "target_name": "cs_pcan_usb",
      "sources": [
        "src/pcan.c",
        "src/pcan_helper.c",
        "src/napi_helper.c",
        "src/common_helper.c"
      ],
      "include_dirs": [
        "<(pcan_include)"
      ],
      "link_settings": {
        "libraries": [
          "<(pcan_lib)"
        ]
      },
      "conditions": [
        [
          "OS=='win'",
          {
            # PEAK PCAN-Basic API (64-bit). PCANBasic.dll itself is installed
            # by PEAK's device driver setup.
            "sources": [
              "src/pcan_event_win32.c"
            ]
          }
        ],
        [
          "OS=='mac'",
          {
            # UV Software PCBUSB library. Look for it first in a PCBUSB folder
            # next to the addon (see tools/bundle-pcbusb.js), then where
            # PCBUSB's install.sh puts it.
            "sources": [
              "src/pcan_event_darwin.c"
            ],
            "xcode_settings": {
              "OTHER_LDFLAGS": [
                "-Wl,-rpath,@loader_path/PCBUSB",
                "-Wl,-rpath,/usr/local/lib"
              ]
            }
          }
        ]
      ]
    }
  ]
}
