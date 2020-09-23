:: Simple batch script to build a Win32-only test program
:: 2020-09-23

where /q cl
IF ERRORLEVEL 1 (
   call "c:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)
		
cl /I "C:\tools\PCAN-Basic API\Include" main.c pcan_helper.c pcan_thread.c pcan_event.c "C:\Tools\PCAN-Basic API\x64\VC_LIB\PCANBasic.lib" Advapi32.lib
