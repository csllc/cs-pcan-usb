{
  "targets": [
      {
          "target_name": "pcan",
          "sources": [ "pcan.c",
                       "pcan_helper.c",
                       "pcan_event.c",
                       "napi_helper.c",
                       "common_helper.c" ],
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
