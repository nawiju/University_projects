from bus_project_NJ.src.fetch_theaters import fetch_theaters
from bus_project_NJ.src.fetch_current_positions import fetch_current_positions
from bus_project_NJ.src.fetch_bus_stops import fetch_bus_stops
from bus_project_NJ.src.fetch_bus_routes import collect_bus_routes
from bus_project_NJ.src.fetch_bus_schedules import collect_bus_schedules
import pytest
from unittest.mock import MagicMock, patch
import sys
import json
import os
import shutil
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))


# Test 1: tests fetching data regarding theaters from API
@pytest.fixture
def mock_theater_response():
    response = {
        "result": {
            "featureMemberPropertyKey": [
                "OBJECTID",
                "ULICA",
                "NUMER",
                "KOD",
                "OPIS",
                "DZIELNICA",
                "JEDN_ADM",
                "TEL_FAX",
                "WWW",
                "MAIL",
                "AKTU_DAN"
            ],
            "featureMemberList": [
                {
                    "geometry": {
                        "type": "ShapePoint",
                        "coordinates": [
                            {
                                "latitude": "52.217160",
                                "longitude": "20.989056"
                            }
                        ]
                    },
                    "properties": [
                        {
                            "value": "13",
                            "key": "OBJECTID"
                        },
                        {
                            "value": "M. Reja",
                            "key": "ULICA"
                        },
                        {
                            "value": "9",
                            "key": "NUMER"
                        },
                        {
                            "value": "02-104",
                            "key": "KOD"
                        },
                        {
                            "value": "Teatr Ochoty",
                            "key": "OPIS"
                        },
                        {
                            "value": "Ochota",
                            "key": "DZIELNICA"
                        },
                        {
                            "value": "Warszawa",
                            "key": "JEDN_ADM"
                        },
                        {
                            "value": "535 395 513",
                            "key": "TEL_FAX"
                        },
                        {
                            "value": "http:\\/\\/www.teatrochoty.pl\\/",
                            "key": "WWW"
                        },
                        {
                            "value": "mailto:sekretariat@teatrochoty.pl",
                            "key": "MAIL"
                        },
                        {
                            "value": "czerwiec 2014",
                            "key": "AKTU_DAN"
                        }
                    ]
                }
            ]
        }
    }

    return response


@pytest.fixture
def expected_theater_response():
    return [
        {
            "name": "Teatr Ochoty",
            "latitude": "52.217160",
            "longitude": "20.989056",
            "address": "M. Reja",
            "district": "Ochota"
        }
    ]


def test_fetching_theaters(mock_theater_response, expected_theater_response):
    with patch('bus_project_NJ.src.fetch_theaters.requests.get') as mock_get:
        mock_get.return_value = MagicMock(status_code=200)
        mock_get.return_value.json.return_value = mock_theater_response

        folder_path = 'test'

        fetch_theaters("dummy_key", folder_path)

        with open('test/theaters.json', 'r') as file:
            data = json.load(file)
            assert data == expected_theater_response


# Test 2.1: tests fetching data regarding the current location of busses
# from API


@pytest.fixture
def mock_current_positions_1():
    return {
        "result": [
            {
                "Lines": "test1",
                "Lon": 21.041642,
                "VehicleNumber": "1000",
                "Time": "2024-02-19 19:29:29",
                "Lat": 52.24989,
                "Brigade": "1"
            },
            {
                "Lines": "test2",
                "Lon": 21.035853,
                "VehicleNumber": "1001",
                "Time": "2024-02-19 19:29:36",
                "Lat": 52.2227791,
                "Brigade": "3"
            },
            {
                "Lines": "test3",
                "Lon": 21.034853,
                "VehicleNumber": "1002",
                "Time": "2024-02-19 18:05:48",
                "Lat": 52.253721,
                "Brigade": "4"
            }
        ]
    }


@pytest.fixture
def expected_current_positions_1():
    return {
        "1000": {
            "Lines": "test1",
            "Lon": 21.041642,
            "VehicleNumber": "1000",
            "Time": "2024-02-19 19:29:29",
            "Lat": 52.24989,
            "Brigade": "1"
        },
        "1001": {
            "Lines": "test2",
            "Lon": 21.035853,
            "VehicleNumber": "1001",
            "Time": "2024-02-19 19:29:36",
            "Lat": 52.2227791,
            "Brigade": "3"
        },
        "1002": {
            "Lines": "test3",
            "Lon": 21.034853,
            "VehicleNumber": "1002",
            "Time": "2024-02-19 18:05:48",
            "Lat": 52.253721,
            "Brigade": "4"
        }
    }


def test_fetching_current_positions(
        mock_current_positions_1,
        expected_current_positions_1):

    with patch('bus_project_NJ.src.helper_functions.requests.get') as mock_get:
        mock_get.return_value = MagicMock(status_code=200)
        mock_get.return_value.json.return_value = mock_current_positions_1

        file_number = len(os.listdir('test'))

        fetch_current_positions("dummy_key", 1, 10, 'test')

        with open(f'test/data_set_{file_number}/0.json', 'r') as file:
            data = json.load(file)
            assert data == expected_current_positions_1

# Test 2.2: tests fetching data regarding the current location of busses
# from API


@pytest.fixture
def mock_current_positions_2():
    return {
        "result": [
            {
                "Lines": "test1",
                "Lon": 21.033248,
                "VehicleNumber": "1000",
                "Time": "2024-02-19 19:30:36",
                "Lat": 52.260526,
                "Brigade": "1"
            },
            {
                "Lines": "test2",
                "Lon": 21.033248,
                "VehicleNumber": "1001",
                "Time": "2024-02-19 19:36:36",
                "Lat": 52.267934,
                "Brigade": "3"
            },
            {
                "Lines": "test3",
                "Lon": 21.033248,
                "VehicleNumber": "1002",
                "Time": "2024-02-19 18:07:58",
                "Lat": 52.274809,
                "Brigade": "4"
            }
        ]
    }


@pytest.fixture
def expected_current_positions_2():
    return {
        "1000": {
            "Lines": "test1",
            "Lon": 21.033248,
            "VehicleNumber": "1000",
            "Time": "2024-02-19 19:30:36",
            "Lat": 52.260526,
            "Brigade": "1"
        },
        "1001": {
            "Lines": "test2",
            "Lon": 21.033248,
            "VehicleNumber": "1001",
            "Time": "2024-02-19 19:36:36",
            "Lat": 52.267934,
            "Brigade": "3"
        },
        "1002": {
            "Lines": "test3",
            "Lon": 21.033248,
            "VehicleNumber": "1002",
            "Time": "2024-02-19 18:07:58",
            "Lat": 52.274809,
            "Brigade": "4"
        }
    }


def test_fetching_current_positions_2(
        mock_current_positions_2,
        expected_current_positions_2):

    with patch('bus_project_NJ.src.helper_functions.requests.get') as mock_get:
        mock_get.return_value = MagicMock(status_code=200)
        mock_get.return_value.json.return_value = mock_current_positions_2

        file_number = len(os.listdir('test'))

        fetch_current_positions("dummy_key", 1, 10, 'test')

        with open(f'test/data_set_{file_number}/0.json', 'r') as file:
            data = json.load(file)
            assert data == expected_current_positions_2

            existing_file_path = f'test/data_set_{file_number}/0.json'

            new_file_path = f'test/data_set_{file_number - 1}/1.json'

            with open(existing_file_path, 'r') as existing_file:
                data = json.load(existing_file)

            with open(new_file_path, 'w') as new_file:
                json.dump(data, new_file)

            os.remove(existing_file_path)

            folder_path = f'test/data_set_{file_number}'
            shutil.rmtree(folder_path)

# Test 3: tests fetching data regarding bus stops from API


@pytest.fixture
def mock_bus_stops():
    return {
        "result": [
            {
                "values": [
                    {"value": "1001", "key": "zespol"},
                    {"value": "01", "key": "slupek"},
                    {"value": "Kijowska", "key": "nazwa_zespolu"},
                    {"value": "2201", "key": "id_ulicy"},
                    {"value": "52.248455", "key": "szer_geo"},
                    {"value": "21.044827", "key": "dlug_geo"},
                    {"value": "al.Zieleniecka", "key": "kierunek"},
                    {"value": "2023-10-14 00:00:00.0", "key": "obowiazuje_od"}
                ]
            },
            {
                "values": [
                    {"value": "1002", "key": "zespol"},
                    {"value": "01", "key": "slupek"},
                    {"value": "Z\u0105bkowska", "key": "nazwa_zespolu"},
                    {"value": "2201", "key": "id_ulicy"},
                    {"value": "52.251325", "key": "szer_geo"},
                    {"value": "21.038457", "key": "dlug_geo"},
                    {"value": "Kijowska", "key": "kierunek"},
                    {"value": "2023-10-14 00:00:00.0", "key": "obowiazuje_od"}
                ]
            },
            {
                "values": [
                    {"value": "1003", "key": "zespol"},
                    {"value": "03", "key": "slupek"},
                    {"value": "Dw.Wile\u0144ski", "key": "nazwa_zespolu"},
                    {"value": "0123", "key": "id_ulicy"},
                    {"value": "52.254118", "key": "szer_geo"},
                    {"value": "21.033248", "key": "dlug_geo"},
                    {"value": "Park Praski", "key": "kierunek"},
                    {"value": "2023-10-14 00:00:00.0", "key": "obowiazuje_od"}
                ]
            }
        ]
    }


@pytest.fixture
def expected_bus_stops_response():
    return {
        "1001-01": {
            "zespol": "1001",
            "slupek": "01",
            "nazwa_zespolu": "Kijowska",
            "id_ulicy": "2201",
            "szer_geo": "52.248455",
            "dlug_geo": "21.044827",
            "kierunek": "al.Zieleniecka",
            "obowiazuje_od": "2023-10-14 00:00:00.0"
        },
        "1002-01": {
            "zespol": "1002",
            "slupek": "01",
            "nazwa_zespolu": "Ząbkowska",
            "id_ulicy": "2201",
            "szer_geo": "52.251325",
            "dlug_geo": "21.038457",
            "kierunek": "Kijowska",
            "obowiazuje_od": "2023-10-14 00:00:00.0"
        },
        "1003-03": {
            "zespol": "1003",
            "slupek": "03",
            "nazwa_zespolu": "Dw.Wileński",
            "id_ulicy": "0123",
            "szer_geo": "52.254118",
            "dlug_geo": "21.033248",
            "kierunek": "Park Praski",
            "obowiazuje_od": "2023-10-14 00:00:00.0"
        }
    }


def test_fetching_bus_stops(mock_bus_stops, expected_bus_stops_response):
    with patch('bus_project_NJ.src.helper_functions.requests.get') as mock_get:
        mock_get.return_value = MagicMock(status_code=200)
        mock_get.return_value.json.return_value = mock_bus_stops

        folder_path = 'test'

        fetch_bus_stops("dummy_key", folder_path)

        with open('test/bus_stops.json', 'r') as file:
            data = json.load(file)
            assert data == expected_bus_stops_response

# Test 4: tests fetching data regarding bus routes from API


@pytest.fixture
def mock_bus_routes():
    return {
        "result": {
            "test1": {
                "test_route_1": {
                    "11": {
                        "odleglosc": 7033,
                        "ulica_id": "2201",
                        "nr_zespolu": "1001",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "10": {
                        "odleglosc": 6393,
                        "ulica_id": "2201",
                        "nr_zespolu": "1002",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "12": {
                        "odleglosc": 7613,
                        "ulica_id": "0123",
                        "nr_zespolu": "1003",
                        "typ": "1",
                        "nr_przystanku": "03"
                    }
                }
            },
            "test2": {
                "test_route_2": {
                    "11": {
                        "odleglosc": 7033,
                        "ulica_id": "2201",
                        "nr_zespolu": "1001",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "10": {
                        "odleglosc": 6393,
                        "ulica_id": "2201",
                        "nr_zespolu": "1002",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "12": {
                        "odleglosc": 7613,
                        "ulica_id": "0123",
                        "nr_zespolu": "1003",
                        "typ": "1",
                        "nr_przystanku": "03"
                    }
                }
            },
            "test3": {
                "test_route_3": {
                    "11": {
                        "odleglosc": 7033,
                        "ulica_id": "2201",
                        "nr_zespolu": "1001",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "10": {
                        "odleglosc": 6393,
                        "ulica_id": "2201",
                        "nr_zespolu": "1002",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "12": {
                        "odleglosc": 7613,
                        "ulica_id": "0123",
                        "nr_zespolu": "1003",
                        "typ": "1",
                        "nr_przystanku": "03"
                    }
                }
            }
        }
    }


@pytest.fixture
def expected_bus_routes_response():
    return {
        "result": {
            "test1": {
                "test_route_1": {
                    "11": {
                        "odleglosc": 7033,
                        "ulica_id": "2201",
                        "nr_zespolu": "1001",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "10": {
                        "odleglosc": 6393,
                        "ulica_id": "2201",
                        "nr_zespolu": "1002",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "12": {
                        "odleglosc": 7613,
                        "ulica_id": "0123",
                        "nr_zespolu": "1003",
                        "typ": "1",
                        "nr_przystanku": "03"
                    }
                }
            },
            "test2": {
                "test_route_2": {
                    "11": {
                        "odleglosc": 7033,
                        "ulica_id": "2201",
                        "nr_zespolu": "1001",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "10": {
                        "odleglosc": 6393,
                        "ulica_id": "2201",
                        "nr_zespolu": "1002",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "12": {
                        "odleglosc": 7613,
                        "ulica_id": "0123",
                        "nr_zespolu": "1003",
                        "typ": "1",
                        "nr_przystanku": "03"
                    }
                }
            },
            "test3": {
                "test_route_3": {
                    "11": {
                        "odleglosc": 7033,
                        "ulica_id": "2201",
                        "nr_zespolu": "1001",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "10": {
                        "odleglosc": 6393,
                        "ulica_id": "2201",
                        "nr_zespolu": "1002",
                        "typ": "1",
                        "nr_przystanku": "01"
                    },
                    "12": {
                        "odleglosc": 7613,
                        "ulica_id": "0123",
                        "nr_zespolu": "1003",
                        "typ": "1",
                        "nr_przystanku": "03"
                    }
                }
            }
        }
    }


def test_fetching_bus_routes(mock_bus_routes, expected_bus_routes_response):
    with patch('bus_project_NJ.src.helper_functions.requests.get') as mock_get:
        mock_get.return_value = MagicMock(status_code=200)
        mock_get.return_value.json.return_value = mock_bus_routes

        folder_path = 'test'

        collect_bus_routes("dummy_key", folder_path)

        with open('test/bus_routes.json', 'r') as file:
            data = json.load(file)
            assert data == expected_bus_routes_response