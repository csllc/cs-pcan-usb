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
                "ext/pcan_basic-win32-x64/include",
            ],
            "library_dirs": [
                "ext/pcan_basic-win32-x64/lib",
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
                               "ext/pcan_basic-win32-x64/bin/PCANBasic.dll",
                               "ext/pcan_basic-win32-x64/bin/ReadMe.txt" ],
                    "destination": "<(module_path)"
                }
            ]
        },
    ],
}
