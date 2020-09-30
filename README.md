# PCAN-USB Package for NodeJS
A drop-in replacement package allowing the PCAN-USB adapter to work in place of CAN-USB-COM.

Currently for Windows only

## Filtering

The PCAN-USB adapter effectively supports one message filter at a time. If multiple filters are applied, the resulting filter spans the range of the highest to lowest IDs of all filters.

Three filter formats are supported by the package:
	- `code` and `mask` format, directly specifying the aceptance filter with a code and mask
		`code: 0x10EF0000, mask: 0xFFFF0000`
	- `fromID` and `toID` format, specifying a range of IDs to be accepted by the filter:
		`fromID: 0x10EF0000, toID: 0x10EFFFFF`
	- `id` format, specifying a string with a range of IDs to be accepted by the filter:
		`id: '10EF0000 10EFFFFF'`

All of the above examples will result in the same acceptance filter being applied by the CAN controller. 

When using the `start`/`stop` or `id` formats, it is not guaranteed that all messages outside of the specified range will be filtered out.

Given a range of IDs to allow, it is possible to convert to an acceptance code and mask pair:

`acceptance_code = fromID & toID`
`acceptance_mask = ~(fromID ^ toID)`

## Functionality not implemented
Public and private methods implemented in `can-usb-com` but not `pcan-usb`
	- `_resolveRequest`
	- `_sendAndWait`
	- `_waitFor`
	- `_flushRequestQueue`

## Quesitonable functionality
	- No error is thrown when the PCAN-USB dongle is unpluged.
	- PCAN-USB hardare does not offer a loopback ("self-receive") mode. This is simulated in `pcan-usb` by pushing data passed to the write method back into the receive data stream.

## Untested functionality
	- All FD functions (`CAN_InitializeFD`, `CAN_ReadFD`, `CAN_WriteFD`)
	- Enumerating more than one PCAN device connected to a computer

