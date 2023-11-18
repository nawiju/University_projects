Macchiato Lungo - Macchiato language debugger
Language: Java
Class: Programowanie Obiektowe (Object-Oriented Programming)

This project implements classes that represent a set of instruction of a simple programming language called Macchiato. The programs in Macchiato consist of a single block, and the goal is to execute these programs, along with providing a debugging feature. One can run the program in two modes: 
(1) Execution without debugging: The program runs from start to finish without interruption unless there is a runtime error. In case of an error, a corresponding message is printed, and the execution stops.
(2) Execution with debugging: The program pauses before executing the first instruction, waiting for commands from standard input. The debugger supports commands such as continue, step <number>, display <level>, and exit.
    	
The Macchiato language includes instructions like blocks, for loops, if statements, variable assignments, and print statements. It also introduces procedures, similar to void functions, with parameter passing by value (note: they support recursion). The language expressions involve literals, variables, addition, subtraction, multiplication, division, and modulo operations. Additionally, the task extends Macchiato with procedures and introduces a new debugger command, dump, for memory dump. The task also introduces an additional feature in the debugger, allowing the creation of a memory dump to a file. The project is accompanied by JUnit tests covering each syntactic construct of the Macchiato language.
