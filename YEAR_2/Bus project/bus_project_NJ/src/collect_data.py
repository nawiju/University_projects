from .fetch_bus_routes import collect_bus_routes
from .fetch_current_positions import fetch_current_positions
from .fetch_bus_schedules import collect_bus_schedules
from .fetch_theaters import fetch_theaters
from .fetch_bus_stops import fetch_bus_stops


def collect_all_data(api_key, data_set_size: int = 60, spacing: int = 60) -> None:
    '''Collects all the data for the project'''

    collect_bus_schedules(api_key)
    collect_bus_routes(api_key)
    fetch_bus_stops(api_key)
    fetch_theaters(api_key)
    fetch_current_positions(api_key, data_set_size, spacing)
