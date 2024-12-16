from automatic_player import AutomaticPlayer
from game import Game
from itertools import product
from collections import defaultdict
from alive_progress import alive_bar

import pytest

"""
Simulates a game using all possible sequences of length `n` with numbers in range 1 to `k`.
:param n: The length of each sequence.
:param k: The range of numbers (1 to k) in the sequence.
"""


def simulate_game(n: int, k: int) -> None:
    # Generate all possible permutations of the sequence
    sequences = list(product(range(1, k + 1), repeat=n))
    cnt = defaultdict(int)
    data = []

    with alive_bar(len(sequences)) as bar:
        for seq in sequences:
            player = AutomaticPlayer(k, n)
            game = Game(k, n, player, list(seq))
            rounds = game.play()
            cnt[rounds] += 1
            data.append(rounds)
            bar()

    average_rounds = sum(data) / len(data)
    max_rounds = max(data)

    return cnt, average_rounds, max_rounds


if __name__ == "__main__":
    n = 4
    k = 6
    cnt, average_rounds, max_rounds = simulate_game(n, k)
    print(f"Number of rounds: {cnt}")
    print(f"Average number of rounds: {average_rounds}")
    print(f"Maximum number of rounds: {max_rounds}")
