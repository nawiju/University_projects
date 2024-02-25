import json
import os
import requests


def collect_bus_routes(api_key: str, path: str = 'bus_data') -> None:
    '''Collects the bus routes and saves them in a json file'''

    url = 'https://api.um.warszawa.pl/api/action/public_transport_routes/'

    params = {
        'apikey': api_key,
        'type': '1'
    }

    # The request is repeated until it is successful
    while True:
        try:
            response = requests.get(url, params=params)
            response.raise_for_status()
            break
        except BaseException:
            continue

    file_path = os.path.join(f'{path}', 'bus_routes.json')

    with open(file_path, 'w') as file:
        json.dump(response.json(), file)
