from abc import ABC, abstractmethod

"""
This class will be used to represent a player in the game. It will
contain the methods that the player must implement in order to play
the game.
"""


class Player(ABC):
    def __init__(self, k: int, n: int):
        self._k: int = k
        self._n: int = n
        self._previous_queries: list[list[int]] = []
        self._previous_results: list[tuple[int, int]] = []
        self._previous_queries_dict: list[dict[int, int]] = []

    @property
    def previous_queries(self) -> list[list[int]]:
        return self._previous_queries

    """
    Gets the player's next move.
    :param round_number: the current round number
    :return: the player's query
    """

    @abstractmethod
    def get_next_query(self, round_number: int) -> list[int]:
        pass

    @abstractmethod
    def update_query_result(self, query: list[int], result: tuple[int, int]) -> None:
        self._previous_results.append(result)
        self._previous_queries.append(query)
        self._previous_queries_dict.append({i: query.count(i) for i in set(query)})
