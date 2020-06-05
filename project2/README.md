# CS 118 Project 2

## Header Design

- Limited to 12 Bytes
- Header needs to contain ACK #, SEQ #, ACK , SYN , and FIN flags
- Max SEQ # is 25600 ~ 2 bytes

Header Format:
| SEQ Number = 2 BYTES | ACK Number = 2 BYTES | FLAGS = 1 BYTES | LENGTH = 2 Bytes | 5 NULL BYTES | 

FLAG = 0 0 0 0 0 0 0 1 - SYN
FLAG = 0 0 0 0 0 0 1 0 - ACK
FLAG = 0 0 0 0 0 1 0 0 - FIN

LOGICAL OR of the flags you want to set is the byte that forms flags
