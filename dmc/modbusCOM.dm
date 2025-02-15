;MODBUS EMULATOR for CallHandler

;Specify a device
ttyUSB0
-9600
;Client
;-192.168.1.30
;-502

;Define some macros

;Read Coils 0000-0001
1 = 08 01 00 00 00 01 fd 53

;Read Inputs 0000-0001
2 = 08 02 00 00 00 01 b9 53

;Read Registers (0000-0003)
3 = 08 04 00 00 00 04 f1 50

;Read Registers (0000-0003) BAD CRC
0 = 08 04 00 00 00 04 f1 51
