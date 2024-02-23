import os
import json
import matplotlib.pyplot as plt
import numpy as np
from typing import Tuple
from .helper_functions import create_directory, load_geojson

# Set the font styles for the plots to ensure consistency and cohesion
fontstyle_annotation = {'fontsize': 9,
                        'fontweight': 'bold', 'color': '#14333D'}

fontstyle_small = {'fontsize': 8, 'color': '#14333D'}

fontstyle_title = {'fontsize': 13, 'fontweight': 'bold', 'color': '#14333D'}

# Colours for each of the districts
colours = ['#3943B7', '#83C5BE', '#0CF574', '#617073', '#E29578', '#BF9ACA',
           '#EB5160', '#FF7F11', '#2AB7CA', '#7BD389', '#521945', '#9B7EDE',
           '#76E7CD', '#29335C', '#AF125A', '#C6BF06', '#35A7FF', '#706C61']

SPEED_LIMIT = 50


def convert_data(data: list) -> list:
    '''Converts the data from seconds to minutes and
    puts it into a dictionary
    '''

    new_data = []

    for info in data:
        info = info // 60

        new_data.append(info)

    return new_data


def get_percentage(data: list) -> float:
    '''Calculates the percentage of busses that are late in a district'''

    late = 0

    for info in data:
        if info > 5:
            late += 1

    return (late / len(data)) * 100


def plot_district(district: str, data: list, plot: plt.Axes,
                  colour: str, order: str) -> None:
    '''Plots the number of minutes late for a specific district'''

    frequencies = {}
    data = convert_data(data)

    for number in data:
        if number in frequencies:
            frequencies[number] += 1
        else:
            frequencies[number] = 1

    x = list(frequencies.keys())
    y = list(frequencies.values())

    plot.set_title(f'{district} #{order}', fontstyle_annotation)
    plot.set_xlabel('Number of minutes late', fontstyle_small)
    plot.set_ylabel('Frequency', fontstyle_small)

    # Each district has a different colour from the colours array
    plot.bar(x, y, color=colour)


def plot_warsaw(data, plot: plt.Axes) -> None:
    '''Plots the number of minutes late for all of Warsaw'''

    frequencies = {}

    for district in data:
        for number in convert_data(data[district]):
            if number in frequencies:
                frequencies[number] += 1
            else:
                frequencies[number] = 1

    x = list(frequencies.keys())
    y = list(frequencies.values())

    plot.set_title('Warsaw', fontstyle_annotation)
    plot.set_xlabel('Number of minutes late', fontstyle_small)
    plot.set_ylabel('Frequency', fontstyle_small)

    plot.bar(x, y)


def plot_district_percentage(districts, data, plot: plt.Axes) -> None:
    '''Plots the percentage of busses late in each district in one plot'''

    percentages = {}
    district_number = 0

    for district in districts:
        if district not in data:
            continue

        if len(data[district]) == 0:  # Avoids division by zero
            continue

        percentages[district_number + 1] = get_percentage(convert_data(data[district]))

        # Each district is assigned a number from 1 to 18
        district_number += 1

    x = list(percentages.keys())
    y = list(percentages.values())

    plot.set_title(
        'The percentage of busses\nlate in each district',
        fontstyle_annotation)
    plot.set_xlabel('District number', fontstyle_small)
    plot.set_ylabel('Percentage', fontstyle_small)

    plot.bar(x, y, color=colours)


def configure_plot() -> Tuple[plt.Figure, plt.Axes]:
    '''Creates the appropriate figure and plot to plot the
    punctuality of Warsaw's busses
    '''

    figure, plot = plt.subplots(4, 5, figsize=(30, 20))
    figure.subplots_adjust(wspace=0.6, hspace=0.6)

    figure.suptitle(
        'The number of minutes late in Warsaw and all of Warsaw\'s districts',
        fontsize=16,
        fontweight='bold',
        fontdict=fontstyle_title,
        y=0.96)

    return figure, plot


def plot_lateness(data_set: int, 
                  src_path: str = 'lateness_data',
                  dst_path: str = 'plots') -> None:
    '''Plots the number of minutes late for all of Warsaw and
    all of Warsaw's districts
    '''

    # Creates a directory for the plots if it does not already exist
    create_directory(data_set)

    try:
        with open(f'{src_path}/lateness_{data_set}.json', 'r') as file:
            lateness_data = json.load(file)
    except FileNotFoundError:
        print(f"ERROR: File {src_path}/lateness_{data_set}.json \
              does not exist")
        return

    districts_file = load_geojson('warszawa-dzielnice.geojson')

    districts = sorted([district['properties']['name']
                       for district in districts_file['features'][1:]])

    figure, plot = configure_plot()

    iter = 0

    for i in range(20):
        row = i // 5
        col = i % 5

        if i == 7:
            plot_warsaw(lateness_data, plot[row][col])
            continue

        if i == 12:
            plot_district_percentage(districts, lateness_data, plot[row][col])
            continue

        if districts[iter] in lateness_data:
            plot_district(districts[iter],
                          lateness_data[districts[iter]],
                          plot[row][col],
                          colours[iter],
                          f'{iter + 1}')

        iter += 1

    plt.get_current_fig_manager().full_screen_toggle()
    plt.savefig(f'{dst_path}/plots_{data_set}/Bus_punctuality_plot_{data_set}.png')


def graph_speed_versus_delays(data: list, data_set: int, path) -> None:
    '''Graphs the relationship between the percentage of busses late and the
    percentage of busses speeding for the 10 busses with the highest
    delays to speeding ratio
    '''
    
    x = [f'{entry[0][0]}-{entry[0][1]}-{entry[0][2]}' for entry in data[:10]]
    y1 = [entry[1] for entry in data[:10]]
    y2 = [entry[2] for entry in data[:10]]

    fig, ax = plt.subplots(figsize=(10, 6))
    width = 0.4
    ax.bar(x, y1, label='Percentage Late', width=width)
    ax.bar([i + width for i in range(len(x))], y2,
           label='Speeding Percentage', width=width)

    ax.set_xlabel('Line-Brigade-Vehicle Number')
    ax.set_ylabel('Percentage')
    ax.set_title(
        'Percentage Late and Speeding Percentage for the 10 Busses Sorted by their Delays to Speeding Ratio')
    ax.legend()

    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.savefig(
        f'{path}/plots_{data_set}/Speeding_versus_delays_plot_{data_set}.png')


def plot_speed_versus_delays(data_set: int, 
                             path: str = 'plots', 
                             filtered_path: str = 'filtered_data',
                             lateness_path: str = 'lateness_data') -> None:
    '''Plots the relationship between a vehicle going over the speed limit and 
    them being on time
    '''

    try:
        with open(f'{lateness_path}/lateness_lines_{data_set}.json', 'r') as f:
            lateness_data = json.load(f)
    except FileNotFoundError:
        print(f"ERROR: File {lateness_path}/lateness_lines_{data_set}.json \
              does not exist")
        return

    try:
        with open(f'{filtered_path}/filtered_data_{data_set}.json', 'r') as f:
            filtered_data = json.load(f)
    except FileNotFoundError:
        print(f"ERROR: File {filtered_path}/filtered_data_{data_set}.json \
              does not exist")
        return

    data = []

    for line in lateness_data:
        if lateness_data[line][1] == []:
            continue

        percentage_late = get_percentage(convert_data(lateness_data[line][1]))

        speeding = 0
        all_measurements = 0

        for info in filtered_data:
            if info['Line'] == lateness_data[line][0][0] and info['Brigade'] == lateness_data[
                    line][0][1] and info['Vehicle_number'] == lateness_data[line][0][2]:
                all_measurements += 1

                if info['Velocity'] > SPEED_LIMIT:
                    speeding += 1

        if all_measurements == 0:
            continue

        speeding_percentage = round(speeding / all_measurements * 100, 2)

        if speeding_percentage == 0:
            continue

        data.append(
            (lateness_data[line][0],
             percentage_late,
             speeding_percentage,
             percentage_late /
             speeding_percentage))

    data.sort(key=lambda x: x[3], reverse=True)

    graph_speed_versus_delays(data, data_set, path)
