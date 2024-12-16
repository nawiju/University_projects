from player import Player
from manual_player import ManualPlayer
from judge import check
from typing import Optional

import random
import sys

"""
This class will be used to play the game. It will generate a hidden
sequence and will ask the player to guess the sequence. It will then
evaluate the guess and return the number of pawns in the correct
position and the number of pawns of a correct color but in the wrong
position. The game will continue until the player guesses the sequence
correctly or gives up.
"""


class Game:
    def __init__(
        self, k: int, n: int, player: Player, hidden_sequence: Optional[list[int]]
    ) -> None:
        self._k: int = k
        self._n: int = n
        self._hidden_sequence: list[int] = hidden_sequence if hidden_sequence else []
        self._player: Player = player

    """
    This function will be used to play the game. It generates a hidden
    sequence, asks for the player's guess, evaluates it and responds 
    appropriately. The game will continue until the player guesses the
    sequence correctly or gives up.
    """

    def play(self) -> int:
        if not self._hidden_sequence:
            self.generate_hidden_sequence()
        round_number: int = 1

        while True:
            try:
                query: list[int] = self._player.get_next_query(round_number)
            except EOFError:
                # If the player gives up, the hidden sequence is revealed
                print(
                    f"Aw, you gave up! The hidden sequence was: {self._hidden_sequence}."
                )
                sys.exit(0)
            except Exception as e:
                continue

            result: tuple[int, int] = check(self._k, self._hidden_sequence, query)
            self._player.update_query_result(query, result)

            print(f"Round {round_number}: {result[0]} correct, {result[1]} misplaced.")
            if result[0] == self._n:
                break

            round_number += 1

        print(
            f"Congratulations! The sequence has been guessed in {round_number} rounds."
        )
        return round_number

    def generate_hidden_sequence(self) -> None:
        self._hidden_sequence = [random.randint(1, self._k) for _ in range(self._n)]
