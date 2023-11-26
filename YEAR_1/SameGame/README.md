## SamegGme

Language: C

Class: Wstęp do programowania (Introduction to programming)

The rectangular board for the single-player SameGame is divided into fields, organized in rows and columns. Each field is either empty or contains a block of a specific type. The player removes groups of adjacent blocks of the same type from the board. Removal of a block group is only possible if there are at least two blocks in the group. After removing a group, the blocks fall into empty spaces in the rows below and columns with blocks are shifted to the left, filling the positions of the empty columns. The program's data, read from input, is the current state of the board. The program's result, written to output, is the state of the board after executing the player's command that is parametrized by the coordinates (x, y) chosen by the player, both starting at 0. 
