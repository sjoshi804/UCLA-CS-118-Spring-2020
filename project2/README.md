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

- Open File for writing 

- Receive Packet, Extract Data, Write to File, Send ACK for Packet received

- Send FIN (note FIN ACKed in previous step)

- Wait for ACK for FIN

- Loop for next client

## Challenges

- Getting the header which had seq num, ack num, data len as bytes rather 
than each of their chars to work correctly. 
The bytes kept getting corrupted and this problem was fixed by switching 
to unsigned char arrays for the buffers

- Dealing with the window correctly
Updating the window used was a bit tricky and required some thought.

- Parsing Flags
I had issues with an initial implementation of flag parsing and 
hence switched to a less elegant but reliable checking of all cases
to make this easy. 

- Dealing with Large Files
I had issues initially when I was using a array to copy the contents of
the data into and then writing this to file. Instead, I chose a more scalable
method where the file is written incrementally at every time step.
This ensures that the implementation can deal with files of arbitrary size with 
a negligible memory footprint. 