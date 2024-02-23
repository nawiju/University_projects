import json
import os
import warnings
import geopy.distance
import requests
from .helper_functions import load_geojson
from typing import Tuple
from datetime import datetime
from shapely.geometry import Point, shape

warnings.filterwarnings('ignore')

RADIUS = 700 # The radius in meters for the GUGiK API request


def dump_filtered_data(data_set: int, 
                       filtered_data: list, 
                       path: str = 'filtered_data') -> None:
    with open(os.path.join(f'{path}',
                           f"filtered_data_{data_set}.json"), 'w') as outfile:
        json.dump(filtered_data, outfile)


def calculate_velocity(pos_1: tuple, pos_2: tuple, time_1, time_2) -> float:
    '''Calculates the velocity of a vehicle between two points in km/h'''

    try:
        distance = geopy.distance.distance(pos_1, pos_2).km
    except BaseException:
        return None

    time_difference = (time_2 - time_1).seconds

    if (time_difference == 0):
        velocity = 0
    else:
        velocity = distance / time_difference * 3600

    return velocity


def determine_district(point_coords: tuple) -> str:
    '''Determines the district of a given point using the geojson file
    warszawa-dzielnice.geojson; returns None if the point is not
    in any district
    '''

    districts = load_geojson('warszawa-dzielnice.geojson')

    for feature in districts['features'][1:]:
        district_shape = shape(feature['geometry'])
        if district_shape.contains(Point(point_coords)):
            return feature['properties']['name']
    return None


def determine_street(pos: tuple) -> str:
    '''Determines the street of a given point using the GUGiK API;
    returns None if the request fails
    '''

    position = "POINT(" + str(pos[0]) + " " + str(pos[1]) + ")"

    try:
        street = requests.post(
            f"http://services.gugik.gov.pl/uug/?request=GetAddressReverse&location={position}&srid=4326&radius={RADIUS}").json()['results']['1']['street']
    except BaseException:
        return None

    return street


def valid_position(pos: tuple) -> bool:
    '''Checks if the position is valid'''

    if (pos[0] < -180 or pos[0] > 180 or pos[1] < -90 or pos[1] > 90):
        return False

    return True


def filter_files(
        file_1_dict: dict,
        file_2_dict: dict,
        filtered_data: list,
        streets: bool) -> Tuple[int, int]:
    '''Filters the data from two files, removing any data that is
    not useful for the analysis or is invalid
    '''

    all = 0
    rejected = 0

    for vehicle_number in file_1_dict:
        all += 1

        if vehicle_number not in file_2_dict:
            rejected += 1
            continue

        pos_1 = (file_1_dict[vehicle_number]['Lon'],
                 file_1_dict[vehicle_number]['Lat'])
        pos_2 = (file_2_dict[vehicle_number]['Lon'],
                 file_2_dict[vehicle_number]['Lat'])

        # If the position is invalid, because the latitude or longitude is out
        # of range, skip the entry
        if not valid_position(pos_1) or not valid_position(pos_2):
            rejected += 1
            continue

        try:
            # Convert the time to a datetime object. If the time is invalid,
            # skip the entry
            time_1 = datetime.strptime(
                file_1_dict[vehicle_number]['Time'], '%Y-%m-%d %H:%M:%S')
            time_2 = datetime.strptime(
                file_2_dict[vehicle_number]['Time'], '%Y-%m-%d %H:%M:%S')
        except BaseException:
            rejected += 1
            continue

        velocity = calculate_velocity(pos_1, pos_2, time_1, time_2)

        # Get the district of the start and end positions
        district_start = determine_district(
            (file_1_dict[vehicle_number]['Lon'],
             file_1_dict[vehicle_number]['Lat']))

        district_end = determine_district(
            (file_2_dict[vehicle_number]['Lon'],
             file_2_dict[vehicle_number]['Lat']))

        if district_start is None or velocity is None or district_end is None:
            rejected += 1
            continue

        if streets:
            # Get the street of the start and end positions
            street_1 = determine_street(pos_1)
            street_2 = determine_street(pos_2)

            # If the request to the GUGiK API fails, skip the entry
            if street_1 is None or street_2 is None:
                rejected += 1
                continue
        else:
            street_1 = ""
            street_2 = ""

        data = {
            "Start_pos": pos_1,
            "End_pos": pos_2,
            "District_Start": district_start,
            "District_End": district_end,
            "Velocity": velocity,
            "Line": file_1_dict[vehicle_number]['Lines'],
            "Street_1": street_1,
            "Street_2": street_2,
            "Vehicle_number": vehicle_number,
            "Brigade": file_1_dict[vehicle_number]['Brigade'],
            "Time_1": file_1_dict[vehicle_number]['Time'],
            "Time_2": file_2_dict[vehicle_number]['Time']
        }

        filtered_data.append(data)

    return (all, rejected)


def filter_data_set(
        data_set: int,
        streets: bool = False,
        radius: int = 700,
        path: str = 'filtered_data',
        src_path: str = 'data_sets') -> None:
    '''Filters the data from the selected data set, by filtering each two
    consecutive files and removing invalid data and removes the invalid
    data from the data set
    '''
    
    global RADIUS
    RADIUS = radius

    all = 0
    rejected = 0

    filtered_data = []

    subfolder_path = os.path.join(f'{src_path}', f"data_set_{data_set}")

    number_of_files = len(os.listdir(subfolder_path))

    file_path_1 = os.path.join(subfolder_path, '0.json')
    open_file_1 = open(file_path_1)
    file_1_dict = json.load(open_file_1)

    for i in range(number_of_files - 1):
        print("\tWORKING ON FILE:", i + 1)

        file_path_2 = os.path.join(subfolder_path, f'{i + 1}.json')
        open_file_2 = open(file_path_2)
        file_2_dict = json.load(open_file_2)

        stats = filter_files(file_1_dict, file_2_dict, filtered_data, streets)

        all += stats[0]
        rejected += stats[1]

        open_file_1.close()
        file_path_1 = file_path_2
        file_1_dict = file_2_dict
        open_file_1 = open_file_2

    open_file_2.close()

    dump_filtered_data(data_set, filtered_data, path)

    print(
        f"Rejected {rejected} out of {all} entries which \
        is {round(rejected / all * 100, 2)}%")
