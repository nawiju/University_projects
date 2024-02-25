from .helper_functions import check_validity_of_function_call
from .data_processing_punctuality import plot_lateness
from .data_processing_speed import (
    plot_street_percentage, 
    plot_district_percentage, 
    plot_percentage_per_district, 
    plot_number_of_infractions, 
    draw_map
)
from .data_processing_theaters import plot_theaters_map


def analyze_all(data_set: int, streets: bool = False) -> None:
    '''Analyzes the data from the given data set and plots the results'''

    # Checks whether the function call is valid, ie. whether the data
    # set exists and whether the data set has the required data
    valid_call = check_validity_of_function_call(
        data_set, streets, True)

    if not valid_call:
        raise ValueError("Invalid function call")

    plot_theaters_map(data_set)

    analyze_punctuality(data_set, False)
    analyze_speed(data_set, streets, False)


def analyze_speed(data_set: int, streets: bool = False,
                  check_validity: bool = True) -> None:
    '''Only analyzes the speed of the busses in the given data
    set and plots the results
    '''

    if check_validity:
        valid_call = check_validity_of_function_call(
            data_set, streets, False)

        if not valid_call:
            raise ValueError("Invalid function call")

    if streets:
        plot_street_percentage(data_set)

    plot_district_percentage(data_set)
    plot_percentage_per_district(data_set)
    plot_number_of_infractions(data_set)
    draw_map(data_set)


def analyze_punctuality(data_set: int, check_validity: bool = True) -> None:
    '''Only analyzes the punctuality of the busses in the given data'''

    if check_validity:
        valid_call = check_validity_of_function_call(
            data_set, False, True)

        if not valid_call:
            raise ValueError("Invalid function call")

    plot_lateness(data_set)


def analyze_theaters(data_set: int, check_validity: bool = True) -> None:
    '''Only analyzes the theaters in the given data set
    and plots the results
    '''

    if check_validity:
        valid_call = check_validity_of_function_call(
            data_set, False, False)

        if not valid_call:
            raise ValueError("Invalid function call")

    plot_theaters_map(data_set)
    