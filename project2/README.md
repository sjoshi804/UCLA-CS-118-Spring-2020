# CS 118 Project 2

## Header Design

- Limited to 12 Bytes
- Header needs to contain ACK #, SEQ #, ACK , SYN , and FIN flags
- Max SEQ # is 25600 ~ 2 bytes

Header Format:
| SEQ Number = 2 BYTES | ACK Number = 2 BYTES | FLAGS = 1 BYTES | LENGTH = 2 Bytes | 5 NULL BYTES | 

FLAG = 1 for SYN ONLY
FLAG = 2 for ACK ONLY
FLAG = 3 for SYN & ACK 
FLAG = 5 for SYN & FIN
FLAG = 6 for ACK & FUN
FLAG = 7 for all 3

LOGICAL OR of the flags you want to set is the byte that forms flags
