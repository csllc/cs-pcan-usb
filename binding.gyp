{
    "variables": {
        "mac_dylib_orig": "libPCBUSB.0.11.1.dylib",
        "mac_dylib": "libPCBUSB.dylib",
        "openssl_fips" : "0"
    },
    "targets": [ {
        "target_name": "<(module_name)",
        "sources": [ "src/pcan.c",
                     "src/pcan_helper.c",
                     "src/napi_helper.c",
                     "src/common_helper.c" ],
        "conditions": [
            [ "OS=='win'", {
                "sources": [ "src/pcan_event_win32.c" ],
                "msvs_settings": { },
                "include_dirs": [ "vendor/pcan_basic-win32-x64/include" ],
                "link_settings": { "libraries": [ "-lPCANBasic" ] },
                "conditions": [
                    [ "target_arch=='ia32'", {
                        "library_dirs": [ "vendor/pcan_basic-win32-x64/lib/32" ]
                    } ],
                    [ "target_arch=='x64'", {
                        "library_dirs": [ "vendor/pcan_basic-win32-x64/lib/64" ]
                    } ]
                ]
            } ],
            [ "OS=='mac'", {
                "sources": [ "src/pcan_event_darwin.c" ],
                "include_dirs": [ "vendor/pcbusb-darwin-x64/include" ],
                "library_dirs": [ "<(module_root_dir)/vendor/pcbusb-darwin-x64/lib" ],
                "link_settings": {
                    "libraries": [

                        "-lPCBUSB"
                    ],
                }
            } ]
        ]
    },
    {
        "target_name": "action_after_build",
        "type": "none",
        "dependencies": [ "<(module_name)" ],
        "copies": [ {
            "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
            "conditions": [
                [ "OS=='win'", {
                    "conditions": [
                        [ "target_arch=='ia32'", {
                            "files": [ "vendor/pcan_basic-win32-x64/bin/32/PCANBasic.dll",
                                       "vendor/pcan_basic-win32-x64/bin/ReadMe.txt" ]
                        } ],
                        [ "target_arch=='x64'", {
                            "files": [ "vendor/pcan_basic-win32-x64/bin/64/PCANBasic.dll",
                                       "vendor/pcan_basic-win32-x64/bin/ReadMe.txt" ]
                        } ]
                    ]

                } ],
                [ "OS=='mac'", {
                    "files": [ "vendor/pcbusb-darwin-x64/COPYRIGHT",
                               "vendor/pcbusb-darwin-x64/LICENSE",
                               "vendor/pcbusb-darwin-x64/README",
                               "vendor/pcbusb-darwin-x64/lib/<(mac_dylib)" ]
                } ],
            ],
            "destination": "<(module_path)"
        } ],
        "conditions": [
            [ "OS=='mac'", {
                
                "actions": [ {
                    "action_name": "install_name_tool",
                    "inputs": [ "<(PRODUCT_DIR)/<(module_name).node" ],
                    "outputs": [ "" ],
                    "action": [ "install_name_tool", "-change", "@rpath/<(mac_dylib_orig)", "@loader_path/<(mac_dylib)", "<(PRODUCT_DIR)/<(module_name).node" ]
                } ]
            } ]
        ]
    } ]
}

