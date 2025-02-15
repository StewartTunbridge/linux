;TEST Server

;Specify a device
Server
-8080
#-127.0.0.1
-log

;Macros
1 = 11 22
2 = 22

;Auto responces
#11 22  > 33 44 55 66
#22     > 11 22 33 44
