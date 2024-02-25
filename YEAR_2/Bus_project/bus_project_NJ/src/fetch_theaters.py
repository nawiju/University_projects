import json
import requests
import os


def format_response(response: dict) -> list:
    '''Formats the response from the UM API into a more readable format'''
        
    formatted_response = []

    for feature in response['result']['featureMemberList']:
        theater = {
            'name': feature['properties'][4]['value'],
            'latitude': feature['geometry']['coordinates'][0]['latitude'],
            'longitude': feature['geometry']['coordinates'][0]['longitude'],
            'address': feature['properties'][1]['value'],
            'district': feature['properties'][5]['value']
        }

        formatted_response.append(theater)

    return formatted_response


def fetch_theaters(api_key: str, path: str = 'theaters_data') -> None:
    '''Fetches the data about the theaters in Warsaw and saves 
    it to a JSON file
    '''
        
    # Set the api_key and the url for UM Warszawa
    api_key = api_key
    url = 'https://api.um.warszawa.pl/api/action/wfsstore_get'

    params = {
        'id': 'e26218cb-61ec-4ccb-81cc-fd19a6fee0f8',
        'apikey': api_key
    }

    response = requests.get(url, params=params).json()
        
    response = format_response(response)

    if not os.path.isdir('theaters_data'):
        os.mkdir('theaters_data')

    with open(f'{path}/theaters.json', 'w') as file:
        json.dump(response, file)
