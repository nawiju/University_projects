from automatic_player import AutomaticPlayer
from manual_player import ManualPlayer
from game import Game

import sys


"""
This function will be used to get a positive integer from the user.
:return: positive integer
"""


def get_number() -> int:
    while True:
        try:
            number: int = int(input())
            if number < 1:
                print("Please enter a positive integer.")
                continue
            return number
        except ValueError:
            print("Please enter an integer.")
            continue


"""
This function will be used to get the hidden sequence from the user.
Only valid sequences will be accepted.
:param n: the length of the sequence
:param k: the number of colors
:return: the hidden sequence
"""


def get_hidden_sequence(n: int, k: int) -> list[int]:
    print("Enter the hidden sequence:")
    hidden_sequence: list[int] = []

    while True:
        input_sequence: list = [x for x in input().strip().split()]
        if len(input_sequence) != n:
            print(f"Please enter a sequence of length {n}.")
            continue

        valid: bool = True

        for i in input_sequence:
            if not i.isdigit() or int(i) > k or int(i) < 1:
                valid = False
                print(f"Please enter a sequence with numbers from 1 to {k}.")
                break

        if valid:
            hidden_sequence = [int(i) for i in input_sequence]
            break

    return hidden_sequence


"""
This function will be used to get the number of colors and the length
of the sequence from the user. 
:return: tuple of two integers, first one is the number of colors, 
    second one is the length of the sequence
"""


def get_inputs() -> tuple[int, int]:
    print("Enter the number of colors (k):")
    k: int = get_number()

    print("Enter the length of the sequence (n):")
    n = get_number()

    return k, n


if __name__ == "__main__":
    if len(sys.argv) > 1:
        if sys.argv[1] == "-a":
            k, n = get_inputs()
            hidden_sequence: list[int] = get_hidden_sequence(n, k)
            player = AutomaticPlayer(k, n)
        elif sys.argv[1] == "-m":
            k, n = get_inputs()
            hidden_sequence = None
            player = ManualPlayer(k, n)
        else:
            print(
                "Invalid argument. Use -a for automatic player or -m for manual player."
            )
            sys.exit(1)
    else:
        print(
            "No argument provided. Use -a for automatic player or -m for manual player."
        )
        sys.exit(1)

    game = Game(k, n, player, hidden_sequence)
    game.play()
