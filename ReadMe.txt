-----------------------
Simuator
-----------------------
Simuator is a program to send message as a testing way to check the 
communication line. The simulator can build mulitple devices to send 
message by serial port and TCP connection way. 

-----------------------
Master
-----------------------
Master is a program to receive multiple devices data from tcp or serial port and forward to other master by 3 channels building with them. Master will crate a message queue for every devices, the master will buffer every message receive from simulator or true device.

