# Python module for the analysis of public busses in Warsaw, Poland

A Python module that analyzes publicly available data on public busses in Warsaw, Poland. The data is available at the [Warsaw Public Transport Authority](https://api.um.warszawa.pl/). The project's goals are to: (1) visualize where the busses exceed the speed of 50 km/h, (2) analyze the punctuality of the busses as well as evaluate in which districts do the busses tends to be late, and (3) analyze whether there is a correlation between busses exceeding the speed of 50 km/h and them speeding in the vicinity of theaters.

## Installation

`pip install bus_project_NJ`

## Documentation

`collect_all_data(api_key, data_set_size: int = 60, spacing: int = 60) -> None` - Collects all the data from the API (ie. the bus schedules, bus routes, bus stops, the positions of theaters in Warsaw and the current positions of the busses) and saves it in appropriate directories. 

`collect_bus_routes(api_key: str, path: str = 'bus_data') -> None` - Collects the bus routes and saves them in the "bus_routes" directory.

`collect_bus_schedules(apikey: str, path: str = 'bus_data') -> None` - Collects the bus schedules and saves them in the "bus_data/bus_lines" directory. The `path` parameter is used to determine the directory in which to save the data.

`fetch_bus_stops(apikey: str, path: str = 'bus_data') -> None` - Fetches the bus stops and saves them in the "bus_data/bus_stops" directory. The `path` parameter is used to determine the directory in which to save the data.

`fetch_current_positions(api_key: str, iterations: int, spacing: int = 10, path: str = 'data_sets') -> None` - Fetches the current positions of the busses and saves them in the "data_sets" directory. The `path` parameter is used to determine the directory in which to save the data.

`fetch_theaters(api_key: str, path: str = 'theaters_data') -> None` - Fetches the positions of theaters in Warsaw and saves them in the "theaters_data" directory.

`analyze_all(data_set: int, streets: bool = False) -> None` - analyzes all the data collected and saves the plots in the "plots" directory. The `streets` parameter is used to determine whether to analyze the data in the context of streets as well as districts (by default only districts are analyzed).

`calculate_total_lateness(data_set: int, path: str = 'lateness_data') -> None` - transforms the data into a format that allows for the calculation of the total delays of the busses in Warsaw. The `path` parameter is used to determine the directory in which to save the data.

`filter_data_set(data_set: int, streets: bool = False radius: int = 700, path: str = 'filtered_data', src_path: str = 'data_sets') -> None` - filters the data set into a more friendly format for the analysis. If the `streets` parameter is set to `True`, the data is filtered to include the streets at which the busses sent out data by sending a request to the GUGiK API. The `radius` parameter is used to determine the radius around the positions in which to look for streets. The `path` parameter is used to determine the directory in which to save the data.

`plot_theaters_map(data_set: int, theaters_path: str = 'theaters_data', destination: str = 'plots',dump: bool = False, dump_path: str = '', filtered_path: str = 'filtered_data') -> None` - plots the positions of the theaters in Warsaw and the positions of the busses that exceed the speed of 50 km/h. The `theaters_path` parameter is used to determine the directory in which the theaters data is saved. The `destination` parameter is used to determine the directory in which to save the plots. The `dump` parameter is used to determine whether to save the data in a file. The `dump_path` parameter is used to determine the directory in which to save the dumped data. The `filtered_path` parameter is used to determine the directory in which the filtered data is located.

`plot_street_percentage(data_set: int, number: int = 10, src_path: str = 'filtered_data', dst_path: str = 'plots') -> None` - plots the percentage of infractions that happened on the top {number} streets in a bar chart.

`plot_district_percentage(data_set: int, dst_path: = 'plots') -> None` - plots the percentage of infractions that happened in each district in a bar chart.
 
`plot_percentage_per_district(data_set: int, path: str = 'plots')` - plots the percentage of infractions that happened in each district as a part of the total number of infractions in a pie chart.
  
`plot_number_of_infractions(data_set: int, path: str = 'plots') -> None` - plots the number of infractions that happened in each district.

`draw_map(data_set: int, path: str = 'plots') -> None` - draws a map of Warsaw with the speeding busses and alongside the percentage of infractions in each district. Red lines represent the places where the busses were speeding.

`plot_lateness(data_set: int, src_path: str = 'lateness_data', dst_path: str = 'plots')` - Plots the number of minutes late for all of Warsaw and all of Warsaw's districts.

`plot_speed_versus_delays(data_set: int, path: str = 'plots') -> None` - Plots the relationship between a vehicle going over the speed limit and 
them being on time. 
