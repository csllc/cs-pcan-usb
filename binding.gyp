{
  "targets": [
      {
          "target_name": "pcan",
          "sources": [ "pcan.c",
                       "pcan_helper.c",
                       "pcan_event.c",
                       "pcan_thread.c",
                       "napi_helper.c" ],
          "msvs_settings": {
          },

      "include_dirs": [
          "../../../tools/PCAN-Basic API/Include",
      ],

      "library_dirs": [
          "../../../tools/PCAN-Basic API/x64/VC_LIB",
      ],


      "link_settings": {
          "libraries": [
              "-lPCANBasic",
          ]
      },
      },
  ],
}
