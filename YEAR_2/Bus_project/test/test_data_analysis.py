import pytest
from bus_project_NJ.src.data_processing_speed import prepare_data
from bus_project_NJ.src.data_processing_theaters import plot_theaters_map
from unittest.mock import MagicMock, patch
import sys
import json
import shutil
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

# Test 1: tests the data preparation for the speed analysis.
# All other functions are based upon this data directly


def test_speed_analysis():
    expected_data = [[[[21.041642,
                        52.24989],
                       [21.033248,
                        52.260526]],
                      [[21.034853,
                        52.253721],
                       [21.033248,
                        52.274809]]],
                     {"Ochota_total": 0,
                      "Rembert\u00f3w_total": 0,
                      "Bemowo_total": 0,
                      "Bia\u0142o\u0142\u0119ka_total": 0,
                      "Weso\u0142a_total": 0,
                      "Bielany_total": 0,
                      "Mokot\u00f3w_total": 0,
                      "Praga Po\u0142udnie_total": 0,
                      "Praga P\u00f3\u0142noc_total": 3,
                      "\u015ar\u00f3dmie\u015bcie_total": 1,
                      "Targ\u00f3wek_total": 1,
                      "Ursus_total": 0,
                      "Wola_total": 0,
                      "Ursyn\u00f3w_total": 0,
                      "Wawer_total": 0,
                      "Wilan\u00f3w_total": 0,
                      "W\u0142ochy_total": 0,
                      "\u017boliborz_total": 0,
                      "Praga P\u00f3\u0142noc": 2,
                      "Targ\u00f3wek": 1},
                     [{},
                      5]]

    prepare_data(6, 'test', dump=True, path='test')

    with open('test/data_dump_6.json', 'r') as f:
        data = json.load(f)

    assert data == expected_data

# Test 2: tests the data for the theater analysis


def test_theater_data():
    expected_data = [[],
                     [[[21.041642, 52.24989], [21.033248, 52.260526]],
                      [[21.034853, 52.253721], [21.033248, 52.274809]]]]

    plot_theaters_map(
        6,
        'test',
        'test',
        dump=True,
        dump_path='test',
        filtered_path='test')

    with open('test/theater_lines_6.json', 'r') as f:
        data = json.load(f)

    assert data == expected_data
