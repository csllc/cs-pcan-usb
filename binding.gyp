{
    "targets": [
        {
            "target_name": "<(module_name)",
            "sources": [ "src/pcan.c",
                         "src/pcan_helper.c",
                         "src/pcan_event.c",
                         "src/napi_helper.c",
                         "src/common_helper.c" ],
            "msvs_settings": { },
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
        {
            "target_name": "action_after_build",
            "type": "none",
            "dependencies": [ "<(module_name)" ],
            "copies": [
                {
                    "files": [ "<(PRODUCT_DIR)/<(module_name).node",
                               "lib/peak/PCANBasic.dll",
                               "lib/peak/ReadMe.txt" ],
                    "destination": "<(module_path)"
                }
            ]
        },
    ],
}
