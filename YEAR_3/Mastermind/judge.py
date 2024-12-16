"""
This function will be used to check the guess of the player and 
return the number of pawns in the correct position and the number
of pawns in the wrong position. 
:param k: number of colors from 1 to k
:param hidden: list of integers representing the hidden sequence
:param query: list of integers representing the query sequence
:return: tuple of two integers, first one is the number of
    pawns in the correct position, second one is the number
    of pawns in the wrong position
"""


def check(k: int, hidden: list[int], query: list[int]) -> tuple[int, int]:
    fully_correct: int = 0
    partially_correct: int = 0
    """
    This function fills a dictionary with the number of occurences
    of each element in the list
    :param lst: list of integers
    :return: dictionary with the number of occurences of each element
    """

    def fill_dict(lst: list[int]) -> dict:
        d: dict = dict()
        for i in lst:
            if i in d:
                d[i] += 1
            else:
                d[i] = 1
        return d

    # Dictionaries or the number of occurences of each element in the
    # hidden and query lists
    hidden_dict: dict = fill_dict(hidden)
    query_dict: dict = fill_dict(query)

    assert len(hidden) == len(query)

    for i in range(len(hidden)):
        assert hidden[i] <= k and hidden[i] > 0

        # If the pawns are in the correct position we increment the
        # fully_correct counter and decrement the number of occurences
        # of the element in the dictionaries
        if hidden[i] == query[i]:
            assert hidden_dict[hidden[i]] > 0
            assert query_dict[query[i]] > 0
            fully_correct += 1
            hidden_dict[hidden[i]] -= 1
            query_dict[query[i]] -= 1

    # We calculate the number of pawns in the wrong position by
    # summing the minimum number of occurences of each element in
    # the hidden dictionary and the query dictionary if the element
    # is present in both dictionaries
    partially_correct = sum(
        [
            min(hidden_dict[i], query_dict[i])
            for i in hidden_dict.keys()
            if i in query_dict.keys()
        ]
    )

    return (fully_correct, partially_correct)
