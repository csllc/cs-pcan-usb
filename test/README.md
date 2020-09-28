# Win32 Test Program

This directory contains a test program that only uses the PCAN-Basic library and Win32 API. It initializes the CAN USB adapter, and prints received messages to stdout for 10 seconds before quitting.

As the main module code is updated, it may fall behind and be prone to breakage.

To build main.exe, run `build_main.bat`. It will compile the program with `PCAN_WIN32_ONLY` defined.
