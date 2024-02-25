import matplotlib.pyplot as plt
import json
import os
import geopandas as gpd
import geopy.distance
from .helper_functions import load_geojson
from typing import Tuple
from shapely.geometry import Point, shape
from datetime import datetime

# 50 meters - the maximum distance from the bus stop to the
# bus route to be considered in the analysis
DISTANCE = 0.05


def calculate_distance(pos_1: tuple, pos_2: tuple) -> float:
    return geopy.distance.distance(pos_1, pos_2).km


def determine_district(point_coords: tuple) -> str:
    '''Determines the district of a given point using the
    warszawa-dzielnice.geojson geojson file
    '''

    districts = load_geojson('warszawa-dzielnice.geojson')

    for feature in districts['features'][1:]:
        district_shape = shape(feature['geometry'])
        if district_shape.contains(Point(point_coords)):
            return feature['properties']['name']
    return None


def get_appropriate_time(schedule: list, time: str) -> str:
    '''Returns the closest scheduled time to the given time based on the
    bus schedule and the time at which the bus sent out its location
    '''

    if int(time[11:13]) >= 24:  # Accounts for faulty data
        time = '00' + time[13:]
    else:
        time = time[11:19]

    if int(schedule[0][:2]) >= 24:
        schedule[0] = '00' + schedule[0][2:]

    # Initializes the minimum difference to the first scheduled time
    min_diff = abs(
        datetime.strptime(
            time,
            '%H:%M:%S') -
        datetime.strptime(
            schedule[0],
            '%H:%M:%S'))

    min_time = schedule[0]

    for scheduled_time in schedule:
        if int(scheduled_time[:2]) >= 24:
            scheduled_time = '00' + scheduled_time[2:]

        diff = abs(
            datetime.strptime(
                time,
                '%H:%M:%S') -
            datetime.strptime(
                scheduled_time,
                '%H:%M:%S'))

        if diff < min_diff:
            min_diff = diff
            min_time = scheduled_time

    return min_time


def get_lateness(time_1: str, time_2: str, schedule: list) -> int:
    '''Returns the lateness of the bus in seconds'''

    appropriate_time = get_appropriate_time(schedule, time_1)

    time_1 = datetime.strptime(time_1[11:19], '%H:%M:%S')
    time_2 = datetime.strptime(time_2[11:19], '%H:%M:%S')

    appropriate_time = datetime.strptime(appropriate_time, '%H:%M:%S')

    # Calculates the lowest number of seconds the bus was late to the stop
    return min((abs(appropriate_time - time_1)).seconds,
               (abs(appropriate_time - time_2)).seconds)


def open_files(data_set: int) -> Tuple[list, dict, dict]:
    '''Opens the necessary files for the analysis'''

    try:
        with open('bus_data/bus_routes.json', 'r') as file:
            bus_routes = json.load(file)
    except FileNotFoundError:
        print("ERROR: Bus routes file not found. "
              "Please run fetch_bus_routes.py first.")
        return None, None, None

    try:
        with open(f'filtered_data/filtered_data_{data_set}.json', 'r') as f:
            filtered_data = json.load(f)
    except FileNotFoundError:
        print(f"ERROR: Filtered data set {data_set} not found. "
              "Please run filter_data.py first.")
        return None, None, None

    try:
        with open('bus_data/bus_stops.json', 'r') as file:
            bus_stops_coordinates = json.load(file)
    except FileNotFoundError:
        print("ERROR: Bus stops file not found. \
                Please run fetch_bus_stops.py first.")
        return None, None, None

    return filtered_data, bus_routes, bus_stops_coordinates


def filter_route(
        my_bus_routes: dict,
        my_schedule: dict,
        brigade: str,
        info: dict,
        lateness: list,
        lateness_per_district: dict,
        bus_stops_coordinates: dict,
        route: list) -> None:
    '''Filters the route of the bus and adds the lateness of the bus
    to each stop and district to the respective lists and dictionaries
    '''

    for stop in my_bus_routes[route]:
        stop_id = my_bus_routes[route][stop]['nr_zespolu'] + \
            '-' + my_bus_routes[route][stop]['nr_przystanku']

        if stop_id not in bus_stops_coordinates:
            continue

        stop_coordinates = (
            bus_stops_coordinates[stop_id]['dlug_geo'],
            bus_stops_coordinates[stop_id]['szer_geo'])

        # If the bus was not close enough to the stop, it is not considered in
        # the analysis
        if (
            calculate_distance(
                info['Start_pos'],
                stop_coordinates) > DISTANCE or calculate_distance(
                info['End_pos'],
                stop_coordinates) > DISTANCE):
            continue

        # If the brigade is not in the schedule or the stop is not in the
        # schedule, it is not considered in the analysis as it is faulty data
        if brigade not in my_schedule or stop_id not in my_schedule[brigade]:
            continue

        time_late = get_lateness(
            info['Time_1'],
            info['Time_2'],
            my_schedule[brigade][stop_id])

        # If the bus was late for more than 40 minutes, it is not considered
        # in the analysis as this implies a mistake in the calculations
        if time_late >= 2400:
            continue

        lateness.append(time_late)

        district = determine_district(stop_coordinates)

        if district not in lateness_per_district:
            lateness_per_district[district] = []

        lateness_per_district[district].append(time_late)


def calculate_bus_lateness(
        data_set: int,
        bus_number: str,
        brigade: str,
        line: str) -> Tuple[list, dict]:
    '''Returns the total number of seconds that the bus line was late to each
    stop in a list and the total number of seconds that the bus line was late
    to each district in a dictionary
    '''

    filtered_data, bus_routes, bus_stops_coordinates = open_files(data_set)

    if (filtered_data is None
        or bus_routes is None
            or bus_stops_coordinates is None):
        return [], {}

    schedule_file_path = f'bus_data/bus_lines/bus{line}_schedule.json'

    if os.path.exists(schedule_file_path):
        with open(f'bus_data/bus_lines/bus{line}_schedule.json', 'r') as file:
            my_schedule = json.load(file)
    else:
        return [], {}

    my_bus_routes = bus_routes['result'][line]

    lateness = []
    lateness_per_district = {}

    for info in filtered_data:
        if info['Vehicle_number'] != bus_number or info['Brigade'] != brigade:
            continue

        for route in my_bus_routes:
            filter_route(
                my_bus_routes,
                my_schedule,
                brigade,
                info,
                lateness,
                lateness_per_district,
                bus_stops_coordinates,
                route)

    # Returns all the minutes the busses were late to each stop and
    # the total number of minutes the busses were late to each district
    return lateness, lateness_per_district


def get_lines(data_set: int) -> set:
    '''Returns the set of intervals (lines) that are present in the
    filtered data
    '''

    try:
        with open(f'filtered_data/filtered_data_{data_set}.json', 'r') as file:
            filtered_data = json.load(file)
    except FileNotFoundError:
        print(
            f"ERROR: Filtered data set {data_set} not found. "
            f"Please run filter_data.py first.")
        return None

    lines = set()

    for info in filtered_data:
        lines.add((info['Line'], info['Brigade'], info['Vehicle_number']))

    return lines


def calculate_total_lateness(data_set: int, path: str = 'lateness_data') -> None:
    '''Filters all the data in the already filtered data set and saves the
    statistics regarding how late the busses were to each stop in a json file
    '''

    total_lateness = []
    lateness_per_district = {}
    lines_lateness = {}

    lines = get_lines(data_set)

    if lines is None:
        return
    
    iter = 0

    for line in lines:
        lateness = calculate_bus_lateness(data_set, line[2], line[1], line[0])

        total_lateness.extend(lateness)
        lines_lateness[iter] = (line, lateness[0])
        
        iter += 1   

        for district in lateness[1]:
            if district not in lateness_per_district:
                lateness_per_district[district] = []

            lateness_per_district[district].extend(lateness[1][district])

    with open(f'{path}/lateness_{data_set}.json', 'w') as file:
        json.dump(lateness_per_district, file)
        
    with open(f'{path}/lateness_lines_{data_set}.json', 'w') as file:
        json.dump(lines_lateness, file)

