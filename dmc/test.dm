;NULL

;Specify a device
null
#Client
#-58.178.93.22
-7777
-ascii
-log
-t250

;Define some macros

;Log In
1= "action:login" d "username:commendaus" d "secret:Commend321" d d xx
2= "action:login" a "username:commendaus" a "secret:Commend321" a a
    80   =     "up arrow"
   ^A = control-A
   ^b = "control-B" 
   ^c = "control-C" 

;Cycle continuous output
\ 1000 ^b ^c

;Auto responces
11 > 11 22 33 44 
11 22 > 33 44 55 66 xx

;Filter unwanted
! "InvalidAccountID@@"
