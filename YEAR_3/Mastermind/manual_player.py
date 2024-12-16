from player import Player


"""
This class represents a manual player in the game. It will ask the user
to enter a valid query and will return it to the game. 
"""


class ManualPlayer(Player):
    def __init__(self, k: int, n: int):
        super().__init__(k, n)
        self._query: list[int] = []

    """
    This function will be used to play the next move. It will ask the user
    to enter a valid query and will return True if the query is valid,
    :param round_number: the current round number
    :return: True if the query is valid, False otherwise
    """

    def play_next_move(self, round_number: int) -> bool:
        self._query = []

        print(f"Enter your guess ({round_number}):")
        query: list = [x for x in input().split()]

        if len(query) != self._n:
            print(f"Please enter a query of length {self._n}.")
            return False

        for i in query:
            if not i.isdigit() or int(i) > self._k or int(i) < 1:
                print(f"Please enter a query with numbers from 1 to {self._k}.")
                return False
            self._query.append(int(i))

        return True

    def get_next_query(self, round_number: int) -> list[int]:
        while not self.play_next_move(round_number):
            continue
        return self._query

    def update_query_result(self, query: list[int], result: tuple[int, int]) -> None:
        pass
