import json
import os
from .helper_functions import fetch_data

# Set the url for UM Warszawa
api_key = ''
url = 'https://api.um.warszawa.pl/api/action/dbstore_get'

# Parameters for the request
params = {
    'apikey': api_key,
    'id': 'ab75c33d-3a26-4342-b36a-6e5fef0a3ac3'
}


def format_response(data: dict) -> tuple:
    '''Formats response into a more programmer-friendly format'''
    formatted_data = dict()

    for element in data['values']:
        formatted_data[element['key']] = element['value']

    key = str(formatted_data['zespol']) + "-" + str(formatted_data['slupek'])

    return (formatted_data, key)


def fetch_bus_stops(apikey: str, path: str = 'bus_data') -> None:
    '''Fetches bus stops from UM Warszawa and saves them in a json file'''
    global api_key
    api_key = apikey

    response = fetch_data(params, url, 10000)

    bus_stops_dict = dict()

    for data in response['result']:
        formatted_data = format_response(data)
        bus_stops_dict[formatted_data[1]] = formatted_data[0]

    file_path = os.path.join(f'{path}', 'bus_stops.json')

    with open(file_path, 'w') as file:
        json.dump(bus_stops_dict, file)
