from player import Player
from judge import check

import random


"""
This class represents an automatic player in the game. It will generate
a guess based on a genetic algorithm that takes into account the previous
guesses and their results. The player will try to guess the hidden sequence
in the fewest number of rounds possible.
"""


class AutomaticPlayer(Player):
    _max_gen: int = 150  # Maximum number of generations
    _max_size: int = 60  # Maximum size of the population
    _generation_reset: int = 15  # Number of generations without valid guesses

    def __init__(self, k, n):
        super().__init__(k, n)
        # The first guess is fixed; ex. 1, 2, 3, ..., k, 1, 2, 3, ...
        self._first_guess: list = [i % k + 1 for i in range(n)]

    """
    Performs one-point crossover on two parent chromosomes.
    :param parent1: the first parent chromosome
    :param parent2: the second parent chromosome
    :return: a tuple of two child chromosomes
    """

    def _one_point_crossover(
        self, parent1: list[int], parent2: list[int]
    ) -> tuple[list[int], list[int]]:
        crossover_point: int = random.randint(1, self._n - 1)
        child1 = parent1[:crossover_point] + parent2[crossover_point:]
        child2 = parent2[:crossover_point] + parent1[crossover_point:]
        return child1, child2

    """
    Performs two-point crossover on two parent chromosomes.
    :param parent1: the first parent chromosome
    :param parent2: the second parent chromosome
    :return: a tuple of two child chromosomes
    """

    def _two_point_crossver(
        self, parent1: list[int], parent2: list[int]
    ) -> tuple[list[int], list[int]]:
        if self._n < 3:
            return parent1, parent2
        crossover_points: tuple = sorted(
            random.randint(1, self._n - 1) for _ in range(2)
        )
        child1 = (
            parent1[: crossover_points[0]]
            + parent2[crossover_points[0] : crossover_points[1]]
            + parent1[crossover_points[1] :]
        )
        child2 = (
            parent2[: crossover_points[0]]
            + parent1[crossover_points[0] : crossover_points[1]]
            + parent2[crossover_points[1] :]
        )
        return child1, child2

    """
    Mutates a child chromosome with a small probability.
    :param child: the child chromosome
    :return: the mutated child chromosome
    """

    def _mutate(self, child: list[int], chance: bool = False) -> list[int]:
        if random.random() < 0.03 or chance:
            mutation_point: int = random.randint(0, self._n - 1)
            child[mutation_point] = random.randint(1, self._k)
        return child

    """
    Randomly chooses two points in the chromosome and swaps the elements.
    :param child: the child chromosome
    :return: the mutated child chromosome
    """

    def _inversion_mutation(self, child: list[int], chance: bool = False) -> list[int]:
        if random.random() < 0.02 or chance:
            mutation_points: tuple = sorted(random.sample(range(self._n), 2))
            child[mutation_points[0] : mutation_points[1]] = reversed(
                child[mutation_points[0] : mutation_points[1]]
            )
        return child

    """
    Generates a random guess for the population.
    :return: the generated guess
    """

    def _generate_random_guess(self) -> list[int]:
        return [random.randint(1, self._k) for _ in range(self._n)]

    """
    Calculates the fitness of a chromosome based on the previous guesses and 
    their results. This formula has been shown in "Efficient solutions for 
    Mastermind using genetic algorithms" by Lotte Berghman, Dries Goossens, 
    Roel Leus. 
    :param chromosome: the chromosome to assess
    :param round_number: the current round number
    :return: a tuple containing the fitness score and a boolean 
        indicating if the chromosome is a valid guess
    """

    def _assess_fitness(
        self, chromosome: list[int], round_number: int
    ) -> tuple[int, bool]:
        weight: int = 2 * self._n * (round_number - 1)
        full_matches_difference: int = 0
        partial_matches_difference: int = 0

        for previous_guess, result in zip(
            self._previous_queries, self._previous_results
        ):
            full_matches, partial_matches = check(self._k, chromosome, previous_guess)
            full_matches_difference += abs(full_matches - result[0])
            partial_matches_difference += abs(partial_matches - result[1])

        # A guess is valid if the sum of the full and partial matches
        # is equal to 0 as it means the guess could be the hidden sequence
        return (
            1 / (weight + full_matches_difference + partial_matches_difference),
            full_matches_difference + partial_matches_difference == 0,
        )

    """
    This function is only used when the genetic algorithm fails to find a
    valid guess. It generates a guess by brute force by trying all possible
    combinations of the sequence and checking if the sequence is valid or 
    possible.
    :param round_number: the current round number
    :return: the generated guess
    """

    def _brute_force_guess(self, round_number: int) -> list[int]:
        def backtrack(sequence: list[int], index: int) -> list[int]:
            if index == self._n:
                if self._assess_fitness(sequence, round_number)[1]:
                    return sequence
                return

            for i in range(1, self._k + 1):
                sequence[index] = i
                result = backtrack(sequence, index + 1)
                if result:
                    return result

        return backtrack([1] * self._n, 0)

    """
    Generates a new population based on the previous population and the
    fitness scores of the chromosomes. The new population is generated
    using one-point and two-point crossovers, mutations, and inversion
    mutations.
    :param population: the previous population
    :param round_number: the current round number
    :return: the new population
    """

    def _generate_new_population(self, population: set, round_number: int) -> set:
        new_population = set()

        # The fitness scores determine the probability of a chromosome
        # being selected as a parent. The higher the fitness score,
        # the higher the probability of the chromosome being selected
        fitness_scores: int = sum([chromosome[1][0] for chromosome in population])
        probabilities: list[float] = [
            chromosome[1][0] / fitness_scores for chromosome in population
        ]

        for _ in range(self._max_size):
            # We select two parents based on their fitness scores
            parent1 = random.choices(population, weights=probabilities, k=1)[0][0]
            parent2 = random.choices(population, weights=probabilities, k=1)[0][0]

            # We apply one-point or two-point crossover based on a random
            # probability
            if random.random() < 0.5:
                child1, child2 = self._one_point_crossover(list(parent1), list(parent2))
            else:
                child1, child2 = self._two_point_crossver(list(parent1), list(parent2))

            # We apply mutations and inversion mutations to the children
            child1 = self._mutate(child1)
            child2 = self._mutate(child2)

            child1 = self._inversion_mutation(child1)
            child2 = self._inversion_mutation(child2)

            # We add the children to the new population
            new_population.add(
                (
                    tuple(child1),
                    self._assess_fitness(list(child1), round_number),
                )
            )
            new_population.add(
                (
                    tuple(child2),
                    self._assess_fitness(list(child2), round_number),
                )
            )

        return new_population

    """
    Generates a guess based on the previous guesses and their results using 
    a genetic algorithm that has been described in "Efficient solutions for 
    Mastermind using genetic algorithms" by Lotte Berghman, Dries Goossens, 
    Roel Leus. 
    :param round_number: the current round number
    :return: the generated guess
    """

    def _generate_guess(self, round_number: int) -> list[int]:
        h: int = 1  # Generation number
        generation_without_valid_guesses: int = 0
        valid_guesses_size: int = 0
        population: list = list()
        valid_guesses: set = set()

        # The algorithm will run until the maximum number of generations is
        # reached or a sufficient number of valid guesses are found
        while h <= self._max_gen and len(valid_guesses) <= self._max_size:
            new_population = set()

            # If it is the first generation, we generate random guesses
            # for the population
            if h == 1 or generation_without_valid_guesses >= self._generation_reset:
                for _ in range(self._max_size):
                    new_guess: list[int] = self._generate_random_guess()
                    new_population.add(
                        (
                            tuple(new_guess),
                            self._assess_fitness(new_guess, round_number),
                        )
                    )

                generation_without_valid_guesses = 0
                population = list(new_population)
            else:
                new_population = self._generate_new_population(population, round_number)

                # We add new random guesses to the population to ensure that
                # the population is diverse
                for _ in range(self._max_size // 3):
                    new_guess: list[int] = self._generate_random_guess()
                    new_population.add(
                        (
                            tuple(new_guess),
                            self._assess_fitness(new_guess, round_number),
                        )
                    )

                # We keep the best chromosomes from the previous generation to
                # ensure that the best chromosomes are not lost in mutations
                # and crossovers
                elitism_population = sorted(population, key=lambda x: -x[1][0])[
                    : max(1, len(population) // 10)
                ]
                population = list(new_population)
                population.extend(elitism_population)

                # We keep the best chromosomes from the current generation
                population = sorted(population, key=lambda x: -x[1][0])[
                    : self._max_size
                ]

            # We add the valid guesses to the set of valid guesses
            for chromosome in population:
                if chromosome[1][1]:
                    valid_guesses.add(chromosome)

            # We generate a new population if no valid guesses are found
            # in 15 consecutive generations
            if valid_guesses_size == len(valid_guesses):
                generation_without_valid_guesses += 1
            else:
                generation_without_valid_guesses = 0
                valid_guesses_size = len(valid_guesses)

            h += 1

        # If no valid guesses are found, we generate a guess by brute force
        if len(valid_guesses) == 0:
            return self._brute_force_guess(round_number)

        return list(sorted(valid_guesses, key=lambda x: x[1][0]))[-1][0]

    """
    Returns the guess of the player for the current round. If it is the first 
    round, the player will return a fixed guess. Otherwise, the player will 
    generate a guess based on the previous guesses and their results.
    :param round_number: the current round number
    :return: the guess of the player
    """

    def get_next_query(self, round_number: int) -> list[int]:
        if round_number == 1:
            print(f"Guess {round_number}: ", self._first_guess)
            return self._first_guess
        else:
            guess = self._generate_guess(round_number)
            print(f"Guess {round_number}: ", guess)
            return guess

    def update_query_result(self, query: list[int], result: tuple[int, int]) -> None:
        super().update_query_result(query, result)
