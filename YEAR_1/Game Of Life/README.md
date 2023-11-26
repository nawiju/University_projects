## Game of Life Simulator

Language: C

Class: Wstęp do programowania (Introduction to programming)

John Conway's Game of Life is a cellular automaton, simulating a world of cells. The simulation takes place on a board consisting of infinitely many rows and columns. Rows and columns of the board are numbered with integers. Each cell on the board is in one of two states: alive or dead. The total state of all cells is called a generation. We assume that a cell in row i and column j is adjacent to eight other cells on the board, with row numbers ranging from i - 1 to i + 1 and column numbers ranging from j - 1 to j + 1. In the next generation, a cell will be alive if and only if: in the current generation, it is alive and has exactly two or three live neighbors, or in the current generation, it is dead and has exactly three live neighbors.

The program's input consists of the description of the initial generation and a sequence of commands. The generation description indicates live cells. It takes the form of a sequence of rows starting with the '/' (slash) character. In the last row of the generation description, there is only the '/' character. In all other rows after the '/' character, there is an integer, the row number of the board. Following it is an ascending sequence of non-empty integers, which are column numbers. Each of these numbers is preceded by a space. 

The program recognizes the following commands:
Quit the program: .
Calculate the N'th generation: N
Calculate the next generation: [empty line]
Show the state of the current generation: 0
Shift the window (ie. the fragment of the board currently shown to the user): w k
