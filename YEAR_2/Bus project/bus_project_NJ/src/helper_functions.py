import time
import warnings
import requests
import time
import json
import pkg_resources
import geopandas as gpd
import os

warnings.filterwarnings('ignore')


def fetch_data(params, url, attempts=100) -> dict:
    '''Forces the Warsaw UM API to return the data in a number of
    attempts and retries if the request fails.
    '''

    attempted = 0

    while True:
        if attempted >= attempts:
            return None

        try:
            response = requests.get(url, params=params, timeout=10)
            response.raise_for_status()
            response = response.json()

            if response['result'][0] == 'B':
                attempted += 1
                time.sleep(0.1)
                continue
            else:
                break
        except (requests.exceptions.RequestException, ValueError) as err:
            attempted += 1
            time.sleep(0.1)
        except BaseException:
            attempted += 1
            time.sleep(0.1)
            continue

    print(f'{attempted} attempts were made to fetch the data.')
    return response


def load_geojson(file_path: str) -> dict:
    '''Loads a geojson file from the package resources 
    and returns a dictionary
    '''

    with pkg_resources.resource_stream(__name__, file_path) as file:
        gdf = json.load(file)

    return gdf


def read_geojson(file_path: str) -> gpd.GeoDataFrame:
    '''Reads a geojson file from the package resources 
    and returns a GeoDataFrame
    '''

    with pkg_resources.resource_stream(__name__, file_path) as file:
        gdf = gpd.read_file(file)

    return gdf


def check_validity_of_function_call(
        data_set: int,
        streets: bool = False,
        lateness: bool = False) -> bool:
    '''Verifies if the function call is valid. If the data set does not exist,
    or if the data set does not have street data or lateness data, the 
    function returns False. Otherwise, it returns True.
    '''

    data_set_dir = f"data_sets/data_set_{data_set}"

    if not os.path.isdir(data_set_dir):
        print(f"Data set {data_set} does not exist.")
        return False

    if not os.path.isfile(f'filtered_data/filtered_data_{data_set}.json'):
        print(f"Data set {data_set} does not have street data.")
        return False

    if streets:
        with open(f'filtered_data/filtered_data_{data_set}.json') as file:
            data = json.load(file)
            street_1 = data[0]['Street_1']
            street_2 = data[0]['Street_2']

        if street_1 == '' or street_2 == '':
            print(f"Data set {data_set} does not have street data.")
            return False

    if lateness and not os.path.isfile(
            f'lateness_data/lateness_{data_set}.json'):
        return False

    return True


def create_directory(data_set: int) -> None:
    '''Creates a directory for the plots of a specific data 
    set if it does not exist already.
    '''

    directory = f'plots/plots_{data_set}'

    if not os.path.exists(directory):
        os.makedirs(directory)
