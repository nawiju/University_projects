import matplotlib.pyplot as plt
import os
import json
import geopandas as gpd
from .helper_functions import (
    create_directory, 
    read_geojson, 
    check_validity_of_function_call, 
    load_geojson
)
from typing import Tuple
from shapely.geometry import LineString

SPEED_LIMIT = 50

# Set the font styles for the plots to ensure consistency and cohesion
fontstyle_annotation = {
    'fontsize': 10,
    'fontweight': 'bold',
    'color': '#14333D'}

fontstyle_title = {'fontsize': 13, 'fontweight': 'bold', 'color': '#14333D'}


def get_district_percentage(districts) -> dict:
    '''Returns the percentage of infractions in each district'''
    district_percentages = dict()

    for district in districts:
        if not district.endswith("_total"):
            district_percentages[district] = districts[district] / \
                districts[district + "_total"]

    return district_percentages


def draw_map(data_set: int, path: str = 'plots') -> None:
    '''Draws a map of Warsaw with the speeding busses and alongside 
    the percentage of infractions in each district. Red lines 
    represent the places where the busses were speeding
    '''
    check_validity_of_function_call(data_set)
    create_directory(data_set)

    data = prepare_data(data_set)
    lines = data[0]
    districts = data[1]

    district_percentages = get_district_percentage(districts)

    # Plot the map of Warsaw using the GeoPandas library and the
    # warszawa-dzielnice.geojson file
    gdf = read_geojson('warszawa-dzielnice.geojson')

    map = gdf.plot(
        figsize=(18, 10),
        edgecolor="black",
        color="#FFDDD2",
        legend=True,
        linewidth=1)

    map.set_title("Map of speeding busses in Warsaw", fontdict=fontstyle_title)
    map.set_xlabel("Longitude", fontsize=15, color="black")
    map.set_ylabel("Latitude", fontsize=15, color="black")

    # Plot the lines of the speeding busses
    lines = gpd.GeoDataFrame(
        geometry=[LineString([line[0], line[1]]) for line in lines])
    lines = lines.set_crs(epsg=4326)

    lines.plot(ax=map, color="red", linewidth=1.5, legend=True)

    # Plot the legend for the percentage of infractions in each district
    for district in district_percentages:
        map.scatter(
            [],
            [],
            label=f'{district} - {round(district_percentages[district] * 100, 2)}%',
            color='white')

    map.legend(title="Percentage of\nspeeding busses in each district",
               loc='center left',
               bbox_to_anchor=(1, 0.5),
               fontsize=10,
               title_fontsize=10,
               labelspacing=0.5,
               edgecolor='#14333D',
               facecolor='white',
               frameon=True,
               handlelength=0)

    plt.get_current_fig_manager().full_screen_toggle()

    plt.savefig(
        os.path.join(
            f'{path}/plots_{data_set}',
            f'Warsaw_map_{data_set}.png'))


def plot_number_of_infractions(data_set: int, path: str = 'plots') -> None:
    '''Plots the number of infractions that happened in each district'''
    check_validity_of_function_call(data_set)
    
    create_directory(data_set)

    districts = prepare_data(data_set)[1]
    districts = {district: count for district,
                 count in districts.items() if not district.endswith("_total")}

    fig, bar_chart = plt.subplots(figsize=(20, 12))

    sorted_districts = dict(
        sorted(
            districts.items(),
            key=lambda x: x[1],
            reverse=True))

    a = bar_chart.bar(
        list(
            sorted_districts.keys()), list(
            sorted_districts.values()), color='#00A8B8')

    plt.xticks(rotation=45, fontsize=10)
    plt.xlabel("District", fontdict=fontstyle_annotation)

    plt.yticks(fontsize=8)
    plt.ylabel("Number of infractions", fontdict=fontstyle_annotation)

    plt.title("Number of infractions in each district",
              fontdict=fontstyle_title)

    plt.savefig(
        os.path.join(
            f'{path}/plots_{data_set}',
            f'Number_of_infractions_{data_set}.png'))


def plot_percentage_per_district(data_set: int, path: str = 'plots') -> None:
    '''Plots the percentage of infractions that happened in each 
    district as a part of the total number of infractions in a pie chart
    '''
    check_validity_of_function_call(data_set)
    
    create_directory(data_set)

    districts = prepare_data(data_set)[1]

    total_infractions = sum([count for district, count in districts.items()
                            if not district.endswith("_total")])

    district_percentages = {
        district: (
            count
            / total_infractions)
        * 100 for district,
        count in districts.items() if not district.endswith("_total")}

    fig, pie_chart = plt.subplots(figsize=(10, 10))

    pie_chart.pie(
        list(
            district_percentages.values()),
        labels=list(
            district_percentages.keys()),
        autopct='%1.1f%%',
        pctdistance=0.85,
        colors=['#83C5BE', '#006D77', '#FFDDD2', '#E29578', '#24596B'])

    pie_chart.set_title(
        "Percentage of infractions committed in each district\n \
        as a part of the total number of infractions",
        fontdict=fontstyle_title)

    pie_chart.text(0, -1.3, "Total Infractions: {}".format(total_infractions),
                   fontsize=12, ha='center')

    plt.savefig(
        os.path.join(
            f'{path}/plots_{data_set}',
            f'Percentage_per_district_{data_set}.png'))


def plot_district_percentage(data_set: int, 
                             dst_path: str = 'plots') -> None:
    '''Plots the percentage of infractions that happened in each 
    district in a bar chart
    '''
    check_validity_of_function_call(data_set)
    
    create_directory(data_set)

    districts = prepare_data(data_set)[1]

    district_percentages = get_district_percentage(districts)

    fig, bar_chart = plt.subplots(figsize=(20, 8))

    plt.subplots_adjust(left=0.1, bottom=0.2, right=0.9,
                        top=0.85)  # Add padding

    plt.xticks(rotation=45, fontsize=10)
    plt.xlabel("District", fontdict=fontstyle_annotation)
    plt.yticks(fontsize=8)
    plt.ylabel(
        "Percentage of infractions in each district",
        fontdict=fontstyle_annotation)
    
    plt.title(
        "The percentage of speeding busses in each district",
        fontdict=fontstyle_title)

    a = bar_chart.bar(
        list(district_percentages.keys()),
        sorted(list(district_percentages.values()),reverse=True),
        color='#00A8B8')

    plt.savefig(
        os.path.join(
            f'{dst_path}/plots_{data_set}',
            f'District_percentage_{data_set}.png'))


def plot_street_percentage(data_set: int, 
                           number: int = 10, 
                           src_path: str = 'filtered_data', 
                           dst_path: str = 'plots') -> None:
    '''Plots the percentage of infractions that happened on the top
    {number} streets in a bar chart
    '''
    check_validity_of_function_call(data_set, True)
    
    create_directory(data_set)

    streets_tuple = prepare_data(data_set, src_path, streets_on=True)[2]

    if streets_tuple is None:
        return

    streets = streets_tuple[0]
    total = streets_tuple[1]

    street_percentages = {
        street: (
            count 
            / total) 
        * 100 for street, count in streets.items()}

    top_10_streets = dict(
        sorted(
            street_percentages.items(),
            key=lambda x: x[1],
            reverse=True)[:number])

    fig, bar_chart = plt.subplots(figsize=(20, 10))

    plt.xticks(rotation=45, fontsize=10, ha='right')
    plt.xlabel("Street name", fontdict=fontstyle_annotation)

    plt.yticks(fontsize=8)
    plt.ylabel(
        "Percentage of infractions on each street",
        fontdict=fontstyle_annotation)

    plt.title(
        f"The top {number} streets with the highest percentage "
        f"of speeding busses",
        fontdict=fontstyle_title)

    a = bar_chart.bar(
        list(top_10_streets.keys()), 
        list(top_10_streets.values()), color='#00A8B8')

    # Add padding
    plt.subplots_adjust(left=0.2, bottom=0.25, right=0.8, top=0.9)

    plt.savefig(
        os.path.join(
            f'{dst_path}/plots_{data_set}',
            f'Street_percentage_{data_set}.png'))


def increase_dict_values(dictionary: dict, value_1: str, value_2: str) -> None:
    '''Increases the value of the keys in the dictionary by 1. 
    If the key does not exist, it is created and set to 1. If 
    value_2 is different from value_1, it is also increased by 1.
    '''
    if value_1 not in dictionary:
        dictionary[value_1] = 0

    dictionary[value_1] += 1

    # Prevents the double counting of the same street or
    # district in the preparation of the data
    if value_2 != value_1:
        if value_2 not in dictionary:
            dictionary[value_2] = 0

        dictionary[value_2] += 1


def prepare_data(data_set: int, 
                 source: str = 'filtered_data',
                 streets_on: bool = False,
                 dump: bool = False,
                 path: str = '') -> Tuple[list, dict, Tuple[dict, int]]:
    '''Prepares the data for the plots by reading the filtered data 
    from a json file and returning the lines, districts and streets
    '''

    file_path = os.path.join(f'{source}', f'filtered_data_{data_set}.json')
    file = open(file_path)
    file_data = json.load(file)

    lines = []
    districts = {}
    streets = {}

    district_json = load_geojson('warszawa-dzielnice.geojson')

    for feature in district_json['features'][1:]:
        if feature['properties']['name'] not in districts:
            districts[feature['properties']['name'] + "_total"] = 0

    for info in file_data:
        if info['Velocity'] > SPEED_LIMIT:
            lines.append((info["Start_pos"], info["End_pos"]))

            increase_dict_values(
                districts,
                info["District_Start"],
                info["District_End"])

            if streets_on:
                if info["Street_1"] == "" or info["Street_2"] == "":
                    print("This data set is not suitable for street analysis")
                    return (lines, districts, None)

                increase_dict_values(
                    streets, info["Street_1"], info["Street_2"])

        # We keep the total number of data collected in each district
        if info["District_Start"] != info["District_End"]:
            districts[info["District_End"] + "_total"] += 1

        districts[info["District_Start"] + "_total"] += 1

    total = sum([count for district, count in districts.items()
                if district.endswith("_total")])

    file.close()
    
    if dump and path != '':
        try:
            with open(os.path.join(path, f'data_dump_{data_set}.json'), 'w') as file:
                json.dump((lines, districts, (streets, total)), file)
        except FileNotFoundError:
            print(f"ERROR: File {path}/data_dump_{data_set}.json does not exist")

    return (lines, districts, (streets, total))
