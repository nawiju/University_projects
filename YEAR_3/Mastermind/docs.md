# Algorithms used

### Automatic player
#### Some definitions
- A **genetic algorithm (GA)** is an optimization technique inspired by the principles of natural selection and genetics. It iteratively evolves a population of candidate solutions using operations such as selection, crossover, and mutation, aiming to find the best or most optimal solution to a given problem.
- **Chromosome** represents a single solution to the problem, typically encoded as a string or array of genes (e.g., binary, numeric, or symbolic values). It undergoes genetic operations like crossover and mutation to explore the solution space.
- **Population** refers to the set of all candidate solutions (chromosomes) being evaluated and evolved in a genetic algorithm during a single generation. It serves as the pool from which parents are selected and offspring are generated.
- **Fitness** measures how well a solution (chromosome) performs in solving the given problem, often represented as a score or value that the algorithm seeks to maximize or minimize. It evaluates the quality of the solution in the context of the objective function.
- **One-point crossover** splits two parent chromosomes at a single random point, exchanging their segments beyond this point to produce offspring. This operation introduces genetic diversity by combining traits from both parents.
- **Two-point crossover** selects two random points in the parent chromosomes, exchanging the segments between these points to create offspring. It allows for greater mixing of genetic material than one-point crossover.
- **Elitism** ensures that the best-performing individuals (chromosomes) in the population are directly carried over to the next generation without modification. This strategy helps preserve high-quality solutions and prevents losing the best candidates during reproduction.


The automatic player generates the next guess using a genetic algorithm inspired by ["Efficient solutions for Mastermind using genetic
algorithms"](https://web.archive.org/web/20140909031305/https://lirias.kuleuven.be/bitstream/123456789/164803/1/kbi_0806.pdf) by Lotte Berghman, Dries Goossens and Roel Leus. 

If this is the first round, the automatic player guesses a predefined guess which includes all colours in order. If the sequence's length is smaller than the number of colours, then the guess will be ``1 2 ... n``; if it is larger then the guess will be ``1 2 ... k ... 1 2 ... min(n, lcm(k, n))`` (lcm = lowest common multiple). 

The genetic algorithm generates a new, random population of guesses as the first population or if the algorithm did not generate any new guesses for 15 generations. It calculates the fitness scores of each chromosome in the following manner. For each previous guess, the question "if this chromosome were the answer, what would the judge's result be?" is asked. We use the judge to check what the number of full and partial matches is if the chromosome is the hidden sequence and the previous guess is the guess. The lower the difference between the results obtained in the past and in the function, the higher the fitness score. If the difference is zero, the guess is valid. Based on these finess scores, two parents are selected from the population and one- or two-point crossovers are performed and potentially a mutation and/or inversion will be performed on the children chromosomes. The best 10% of the population is carried over to the next generation (elitism).

A brute force backtracking algorithm is used if the genetic algorithm did not produce any valid guesses. This is necessary so that the automatic player can always generate a valid guess. Valid meaning that it is possible given the infomation collected throught the game. 

For n = 4, k = 6, the average number of guesses necessary for the automatic player is 4.645 with this being the distribution: [1: 1, 2: 14,3: 90, 4: 433, 5: 572, 6: 173, 7: 13].


## Game
1. Generates or accepts a hidden sequence of integers (representing colors).
2. Iteratively interacts with a **Player** instance (human or automated) to collect guesses. Checks whether the player gave up and if so, reveals the sequence and shuts down cleanly.
3. Evaluates the player's guesses using the `check()` function from the `judge` module.
4. Provides feedback about the correctness of each guess (correct positions and misplaced colors).
5. Ends the game when the sequence is correctly guessed or the player gives up.
