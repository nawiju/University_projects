# Mastermind Game

## Overview
This project is an implementation of the **Mastermind** game in Python, featuring two modes:  
1. **Interactive gameplay** - where a user can play the game either via console.  
2. **Automated simulation** - where an algorithm plays the game by attempting to deduce a hidden sequence provided by the user.  

The program generalizes the Mastermind rules to sequences of length **n** with **k** possible colors (represented by numbers from 1 to k). It consists of three main modules:  
- **Judge Module**: Validates guesses and provides feedback.  
- **Automatic Solver**: An algorithm to guess the hidden sequence.  
- **Interactive Player Interface**: Allows a user to play the game manually.  

## Features
### Judge Module
- Validates guesses and provides feedback on:
  - **Exact matches**: Correct colors in the correct positions.
  - **Color-only matches**: Correct colors in incorrect positions.

### Automatic Solver
- Generates guesses based on feedback from the judge.
- Avoids repeating invalid guesses by leveraging information from previous attempts.
- Can solve sequences of reasonable size without keeping all possible guesses in memory by utilizing a genetic algorithm based on ["Efficient solutions for Mastermind using genetic
algorithms"](https://web.archive.org/web/20140909031305/https://lirias.kuleuven.be/bitstream/123456789/164803/1/kbi_0806.pdf) by Lotte Berghman, Dries Goossens and Roel Leus. 

### Interactive Player
- **Console Interface**:  
  - Users can play against the computer by inputting guesses.  
  - The program validates input and provides feedback for each guess.  
  - Allows the user to surrender the game by pressing `Ctrl-D`.  

## How to play
### Manual player
Use this command to launch the program ```python3 mastermind.py -m```\
```
Enter the number of colors (k):
> [[ your desired number of colours written as a number ]]

Enter the length of the sequence (n):
> [[ your desired length written as a number ]]

Enter your guess (1):
> [[ write n numbers from 1 to k separated by spaces ]]
Round 1: 1 correct, 0 misplaced. # feedback from the judge
...
Congratulations! The sequence has been guessed in X rounds.

```

You can give up by pressing control+D.

### Automatic player
Use this command to launch the program ```python3 mastermind.py -a```\

```
Enter the number of colors (k):
> [[ your desired number of colours written as a number ]]

Enter the length of the sequence (n):
> [[ your desired length written as a number ]]

Enter the hidden sequence:
> [[ enter a sequence of length n ]]

```

From here the algorithm will play Mastermind and show its guesses as well as the judge's feedback

## Project structure

```
Mastermind/
├── automatic_player.py
├── docs.md
├── game.py
├── judge.py
├── manual_player.py
├── mastermind.py
├── player.py
└── README.md

```

**automatic_player.py** - Implements an automatic player module that uses strategies to solve the Mastermind game efficiently. It interacts with the judge module to refine guesses.

**game.py** - Contains the core logic for running the game. This includes managing game flow, validating moves, and coordinating interactions between players and the judge.

**judge.py** - Implements the judge module, responsible for evaluating guesses by comparing them to the hidden sequence and returning feedback (correct position and color match).

**manual_player.py** - Implements a manual player module that allows a human player to interact with the game via console inputs.

**mastermind.py** - The main executable file that brings together all components to run the Mastermind game. It integrates the judge and player modules for either interactive or automated gameplay.

**player.py** - Defines the Player abstract class, serving as a blueprint for implementing different types of players (manual or automatic). This class ensures consistency and extensibility in player implementations.