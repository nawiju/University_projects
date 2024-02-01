Language: C

Class: Programowanie współbieżne (Concurrent Programming)

Template provided by @baktrius at https://github.com/baktrius/MIMUW_PW_2nd_lab_assignment

Task description: https://github.com/baktrius/MIMUW_PW_2nd_lab_assignment/blob/main/assignment.md

This project implements "my" version of the MPI communication protocol which serves the purpose of facilitating communication and the exchange of data between processes of concurrent programs. This project implements the procedures MIMPI_Init sets up the environment, MIMPI_Finalize cleans up and closes the environment, process-to-process communication, and group communication in logarithmic time. Group functions include the barrier that synchronizes all processes at a certain point (similar to countdown latch in Java), reduce in which one process collects data from all other processes, and broadcast in which one process sends out data to all other processes. Each group function is a synchronization point. This project implements the receival and sending of messages of any size without "clogging" the communication pipe. It also facilitates the filtering out of messages based on their size and tag.

This is not a project I am particularly proud of despite it receiving 7.5/10 points as the code is admittedly quite messy and in some places inefficient. Despite that, this project allowed me to develop my skills in rendez-vous synchronous communication using sends and receives and expanded my knowledge of the C programming language.  
