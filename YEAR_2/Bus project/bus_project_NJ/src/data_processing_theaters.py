import matplotlib.pyplot as plt
import json
import geopandas as gpd
import geopy.distance
from .helper_functions import create_directory, read_geojson
from typing import Tuple
from shapely.geometry import LineString, Point

# Set the font styles for the plots to ensure consistency and cohesion
fontstyle_annotation = {
    'fontsize': 10,
    'fontweight': 'bold',
    'color': '#235A6C'}

fontstyle_title = {'fontsize': 13, 'fontweight': 'bold', 'color': '#14333D'}

DISTANCE = 0.3
SPEED_LIMIT = 50


def calculate_distance(pos_1: tuple, pos_2: tuple) -> float:
    return geopy.distance.distance(pos_1, pos_2).km


def get_lines(data_set: int, theater_coords: tuple, path: str) -> Tuple[list, list]:
    '''Returns the intervals in which the busses were going over 
    50km/h around theaters and in general
    '''

    with open(f'{path}/filtered_data_{data_set}.json', 'r') as file:
        data = json.load(file)

    lines_theater = []
    lines = []

    for info in data:
        if (
            info['Velocity'] > 50 and calculate_distance(
                (info['Start_pos'][0],
                 info['Start_pos'][1]),
                theater_coords) < DISTANCE or calculate_distance(
                (info['End_pos'][0],
                 info['End_pos'][1]),
                theater_coords) < DISTANCE):
            lines_theater.append((info["Start_pos"], info["End_pos"]))
        elif info['Velocity'] > SPEED_LIMIT:
            lines.append((info["Start_pos"], info["End_pos"]))

    return lines_theater, lines


def configure_plot(map: plt.Axes, percentage: float) -> None:
    '''Sets the title, legend and annotation for the map of Warsaw with the 
    speeding busses and the percentage of speeding around theaters
    '''

    map.set_title(
        "Map of speeding busses in Warsaw and speeding around theaters",
        fontdict=fontstyle_title,
        y=1.05)

    map.legend(["Speeding busses", "Speeding busses around theaters"],
               facecolor='white', edgecolor='black')
    legend = map.get_legend()

    # Set color for "Speeding busses"
    legend.legendHandles[0].set_color('#96C9DC')

    # Set color for "Speeding around theaters"
    legend.legendHandles[1].set_color('red')

    map.text(
        0.96,
        0.85,
        f'{percentage}% of speeding\nhappened around theaters',
        horizontalalignment='right',
        verticalalignment='center',
        transform=map.transAxes,
        fontsize=12,
        fontdict=fontstyle_annotation)


def plot_theaters_map(data_set: int, 
                      theaters_path: str = 'theaters_data', 
                      destination: str = 'plots',
                      dump: bool = False,
                      dump_path: str = '',
                      filtered_path: str = 'filtered_data') -> None:
    '''Plots the map of Warsaw with the speeding busses 
    and the theaters marked on it
    '''

    create_directory(data_set)

    try: 
        with open(f'{theaters_path}/theaters.json', 'r') as file:
            theaters_data = json.load(file)
    except FileNotFoundError:
        print("The file theaters.json does not exist. "
              "Please fetch the theater data first.")
        return

    lines_theater = []
    lines = []

    # Plot the map of Warsaw using the GeoPandas library and the
    # warszawa-dzielnice.geojson file
    gdf = read_geojson('warszawa-dzielnice.geojson')

    map = gdf.plot(figsize=(18, 10), edgecolor="black",
                   color="#FFDDD2", legend=True, linewidth=1)

    for theater in theaters_data:
        longitude = theater['longitude']
        latitude = theater['latitude']

        point = Point(longitude, latitude)
        gdf_point = gpd.GeoDataFrame(geometry=[point], crs="EPSG:4326")
        gdf_point.plot(ax=map, color="blue", markersize=50)

        lines_theater.extend(get_lines(data_set, (longitude, latitude), f'{filtered_path}')[0])
        lines.extend(get_lines(data_set, (longitude, latitude), f'{filtered_path}')[1])
        
    if dump:
        try:
            with open(f'{dump_path}/theater_lines_{data_set}.json', 'w') as file:
                json.dump((lines_theater, lines), file)
        except FileNotFoundError:
            print(f"ERROR: File {dump_path}/theater_lines_{data_set}.json does not exist")
        return

    # Plot the lines of the speeding busses
    lines = gpd.GeoDataFrame(
        geometry=[LineString([line[0], line[1]]) for line in lines])
    lines = lines.set_crs(epsg=4326)

    lines.plot(ax=map, color="#96C9DC", linewidth=1.5, alpha=0.7, legend=True)

    lines_theater = gpd.GeoDataFrame(
        geometry=[LineString([line[0], line[1]]) for line in lines_theater])
    lines_theater = lines_theater.set_crs(epsg=4326)

    lines_theater.plot(ax=map, color="red", linewidth=1.5, legend=True)

    percentage = round(len(lines_theater) / len(lines) * 100, 2)

    configure_plot(map, percentage)

    plt.get_current_fig_manager().full_screen_toggle()
    plt.savefig(f'{destination}/plots_{data_set}/Theater_plot_{data_set}.png')