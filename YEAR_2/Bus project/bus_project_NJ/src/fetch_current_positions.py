import json
import os
import time
from .helper_functions import fetch_data

# Set the url for UM Warszawa
url = 'https://api.um.warszawa.pl/api/action/busestrams_get'


def get_next_folder_number(path: str = 'data_sets') -> int:
    return len(os.listdir(f'{path}'))


def fetch_current_positions(
        api_key: str,
        iterations: int,
        spacing: int = 10,
        path: str = 'data_sets') -> None:
    
    '''Fetches data for (iterations * spacing / 60) minutes and places into a
    dict then into a json file
    '''

    # Parameters for the request
    params = {
        'resource_id': 'f2e5503e-927d-4ad3-9500-4ab9e55deb59',
        'apikey': api_key,
        'type': '1'
    }

    folder_number = get_next_folder_number(path)
    folder_path = os.path.join(f'{path}', 'data_set_' + str(folder_number))

    # Always creates a new folder for the data set
    os.makedirs(folder_path)

    i = 0

    while i < iterations:
        data = fetch_data(params, url)

        if data is None:
            continue

        # Saves the data in a dict for easier access
        response_dict = dict()

        for item in data['result']:
            response_dict[item['VehicleNumber']] = item

        file_path = os.path.join(folder_path, f'{i}.json')

        with open(file_path, 'w') as file:
            json.dump(response_dict, file)

        i += 1
        time.sleep(spacing)
