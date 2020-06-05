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


## Client Design

- Send SYN Packet

- Wait to receive SYN ACK Packet

- Start sending data in pipelined manner until window is full

- Then wait for ACKs and then free space in window accordingly

- Loop until all data sent

- Send FIN

- Wait to receive FIN (log all ACKs received while waiting)

- Send ACK for FIN

- Terminate

## Server Design

- Wait for SYN

- Send SYN ACK

- Receive Packet and Send ACK until Fin received

- Send FIN (note FIN ACKed in previous step)

- Wait for ACK for FIN

- Loop for next client

## Challenges

- Getting the header which had seq num, ack num, data len as bytes rather 
than each of their chars to work correctly. Had lots of issues with this.
- Dealing with the window correctly
- Parsing Flags