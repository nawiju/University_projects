from bus_project_NJ import (
    plot_theaters_map, 
    draw_map, 
    plot_number_of_infractions,
    plot_percentage_per_district,
    plot_district_percentage,
    plot_street_percentage,
    plot_lateness,
    plot_speed_versus_delays,
    fetch_current_positions,
    filter_data_set,
    calculate_total_lateness
)

api_key = 'd97eadec-1b09-4430-92b4-fc70e36b5044'

def fetch_data():
    fetch_current_positions(api_key, 5, 10)
    filter_data_set(11)
    calculate_total_lateness(11)

def plot_data():
    plot_theaters_map(11)
    draw_map(11)
    plot_number_of_infractions(11)
    plot_percentage_per_district(11)
    plot_district_percentage(11)
    plot_street_percentage(11)
    plot_lateness(11)
    plot_speed_versus_delays(11)
    
def plot_data_set(data_set: int, street: bool):
    # plot_theaters_map(data_set)
    draw_map(data_set)
    plot_number_of_infractions(data_set)
    plot_percentage_per_district(data_set)
    plot_district_percentage(data_set)
    plot_lateness(data_set)
    plot_speed_versus_delays(data_set)
    
    if street:
        plot_street_percentage(data_set)

# fetch_data()
# plot_data()

# plot_data_set(10, False)
calculate_total_lateness(11)

# plot_data_set(2, True)
