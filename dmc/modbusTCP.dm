;MODBUS EMULATOR for CallHandler

;Specify a device
;ttyUSB0
Client
-192.168.1.30
-502

;Define some macros

;Read Coils 0000-0001
1 = ff ff ff ff   00 06   08 01 00 00 00 01

;Read Inputs 0000-0001
2 = ff ff ff ff   00 06   08 02 00 00 00 01

;Read Registers (0000-0003)
3 = ff ff ff ff   00 06   08 04 00 00 00 04
