from bus_project_NJ.src.filter_data import filter_data_set
import sys
import json
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

# Test 1: test the filtering of data (their velocity and other data)
def test_filter_data_set():
    expected_filtered_data = [
        {
            "Start_pos": [21.041642, 52.24989],
            "End_pos": [21.033248, 52.260526],
            "District_Start": "Praga Północ",
            "District_End": "Praga Północ",
            "Velocity": 77.60344478474863,
            "Line": "test1",
            "Street_1": "",
            "Street_2": "",
            "Vehicle_number": "1000",
            "Brigade": "1",
            "Time_1": "2024-02-19 19:29:29",
            "Time_2": "2024-02-19 19:30:36"
        },
        {
            "Start_pos": [21.035853, 52.2227791],
            "End_pos": [21.033248, 52.267934],
            "District_Start": "Śródmieście",
            "District_End": "Praga Północ",
            "Velocity": 40.30752757142837,
            "Line": "test2",
            "Street_1": "",
            "Street_2": "",
            "Vehicle_number": "1001",
            "Brigade": "3",
            "Time_1": "2024-02-19 19:29:36",
            "Time_2": "2024-02-19 19:36:36"
        },
        {
            "Start_pos": [21.034853, 52.253721],
            "End_pos": [21.033248, 52.274809],
            "District_Start": "Praga Północ",
            "District_End": "Targówek",
            "Velocity": 60.90151172737643,
            "Line": "test3",
            "Street_1": "",
            "Street_2": "",
            "Vehicle_number": "1002",
            "Brigade": "4",
            "Time_1": "2024-02-19 18:05:48",
            "Time_2": "2024-02-19 18:07:58"
        }
    ]
    
    filter_data_set(6, False, 700, 'test', 'test')
    
    with open('test/filtered_data_6.json', 'r') as f:
        data = json.load(f)
        
    assert data == expected_filtered_data
    