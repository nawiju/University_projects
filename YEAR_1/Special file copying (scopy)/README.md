## Special File Copying (Scopy) 

Language: Assembly

Class: Architektura komputerów i systemy operacyjne (Computer architecture and operating systems)

The program takes two file names as parameters ./scopy in_file out_file. For each byte read from in_file with ASCII value 's' or 'S', writes that byte to out_file. For each non-empty sequence of bytes read from in_file that does not contain the ASCII value of 's' or 'S', writes a 16-bit number to out_file. The number represents the count of bytes in the sequence modulo 65536, written in little-endian format. The program ensures the proper handling of incorrect parameters and execution. 
