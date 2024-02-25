import json
import os
from .helper_functions import fetch_data

# Set the url for UM Warszawa
url = 'https://api.um.warszawa.pl/api/action/dbtimetable_get'
api_key = ""


def get_bus_lines(path: str = 'bus_data') -> list:
    '''Returns a list of bus lines in Warsaw'''

    with open(f'{path}/bus_routes.json', 'r') as file:
        bus_routes = json.load(file)

    lines = list(bus_routes['result'])

    file.close()

    return lines


def format_response(response, bus_schedule: dict, key: str) -> None:
    ''' Formats response into a friendlier format and saves it in the
    line's schedule dictionary
    '''

    for info in response['result']:
        formatted_data = dict()

        for dic in info["values"]:
            formatted_data[dic["key"]] = dic["value"]

        brigade = formatted_data['brygada']

        if brigade not in bus_schedule:
            bus_schedule[brigade] = dict()

        if key not in bus_schedule[brigade]:
            bus_schedule[brigade][key] = list()

        bus_schedule[brigade][key].append(formatted_data['czas'])


def get_line_schedule(line: str, path: str) -> None:
    '''Collects the bus schedule for a specific bus line and
    saves it in a json file
    '''

    bus_line_dir = os.path.join(f'{path}/bus_lines')

    try:
        with open(f'{path}/bus_routes.json', 'r') as file:
            bus_routes = json.load(file)
    except FileNotFoundError:
        print("ERROR: Bus routes file not found. Please run \
              fetch_bus_routes.py first.")
        return

    bus_schedule = dict()

    for route in bus_routes['result'][line]:
        for stop in bus_routes['result'][line][route]:
            info = bus_routes['result'][line][route][stop]

            # Parameters for the request
            params = {
                'apikey': api_key,
                'id': 'e923fa0e-d96c-43f9-ae6e-60518c9f3238',
                'busstopId': info['nr_zespolu'],
                'busstopNr': info['nr_przystanku'],
                'line': line}

            response = fetch_data(params, url)

            # If the request was unsuccessful, skip to the next stop
            if response is None:
                continue

            key = f"{info['nr_zespolu']}-{info['nr_przystanku']}"

            format_response(response, bus_schedule, key)

    with open(os.path.join(bus_line_dir, 
                           f'bus{line}_schedule.json'), 'w') as file:
        json.dump(bus_schedule, file)


def collect_bus_schedules(apikey: str, path: str = 'bus_data') -> None:
    '''Collects the bus schedules for all the bus lines in 
    Warsaw and saves them in a json file
    '''

    global api_key
    api_key = apikey

    bus_lines_dir = os.path.join(f'{path}', 'bus_lines')

    # Create the directory if it doesn't exist already
    if not os.path.exists(bus_lines_dir):
        os.makedirs(bus_lines_dir)

    bus_lines = get_bus_lines(path)
    
    for bus in bus_lines:
        print(f'Collecting data for line {bus}')
        get_line_schedule(bus, path)

