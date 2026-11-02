#!/usr/bin/env python3
import subprocess
import statistics
import re
from pathlib import Path

BIN = Path('/workspaces/HASLab/shio/paddl_simulator/build/paddl_simulator')
BASE = Path('/workspaces/HASLab/shio/paddl_simulator/bandwidth_traces')
CAPACITIES = [10000, 15000, 20000, 25000, 30000, 35000, 40000, 45000, 50000]
FILES = [
    ('p15', BASE / 'p15_20_20_45_half_log_all_zero_data.csv'),
    ('p25', BASE / 'p25_25_25_25_half_log_all_zero_data.csv'),
]
ALGOS = ['static', 'dynamic_max_min']


def run_case(trace_path: Path, algo: str, total_bw: int):
    p = subprocess.run(
        [str(BIN), str(trace_path), algo, '0', str(total_bw)],
        text=True,
        capture_output=True,
        timeout=120,
    )
    text = (p.stdout or '') + (p.stderr or '')
    values = [float(v) for _, v in re.findall(r'Node\s+\d+: final time: (\d+), overhead: ([0-9]+(?:\.[0-9]+)?)%', text)]
    return values


def summarize(values):
    if not values:
        return {'min': None, 'median': None, 'max': None, 'avg': None}
    return {
        'min': min(values),
        'median': statistics.median(values),
        'max': max(values),
        'avg': statistics.fmean(values),
    }


def print_table():
    header = (
        'trace | capacity | static avg | static min | static median | static max | '
        'dynamic avg | dynamic min | dynamic median | dynamic max'
    )
    print(header)
    print('-' * len(header))

    for trace_name, trace_path in FILES:
        for cap in CAPACITIES:
            static_vals = run_case(trace_path, 'static', cap)
            dynamic_vals = run_case(trace_path, 'dynamic_max_min', cap)
            s = summarize(static_vals)
            d = summarize(dynamic_vals)
            print(
                f'{trace_name} | {cap} | '
                f'{s["avg"] if s["avg"] is not None else "N/A"} | '
                f'{s["min"] if s["min"] is not None else "N/A"} | '
                f'{s["median"] if s["median"] is not None else "N/A"} | '
                f'{s["max"] if s["max"] is not None else "N/A"} | '
                f'{d["avg"] if d["avg"] is not None else "N/A"} | '
                f'{d["min"] if d["min"] is not None else "N/A"} | '
                f'{d["median"] if d["median"] is not None else "N/A"} | '
                f'{d["max"] if d["max"] is not None else "N/A"}'
            )
        print()


if __name__ == '__main__':
    print_table()
