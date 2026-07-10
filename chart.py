"""Stage 3: live chart of the CSV ledger. Run alongside poller.py."""

import csv
from datetime import datetime
from pathlib import Path

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

CSV_PATH = Path(__file__).parent / "readings.csv"
REDRAW_INTERVAL_MS = 5000  # re-read the ledger every 5 seconds


def load_readings():
    """Read the whole CSV. Returns (times, temps, hums) as three lists."""
    times, temps, hums = [], [], []
    if not CSV_PATH.exists():
        return times, temps, hums

    with open(CSV_PATH, newline="") as f:
        for row in csv.DictReader(f):
            times.append(datetime.fromisoformat(row["timestamp"]))
            temps.append(float(row["temperature_c"]))
            hums.append(float(row["humidity_pct"]))
    return times, temps, hums


fig, (ax_temp, ax_hum) = plt.subplots(2, 1, sharex=True, figsize=(10, 6))
fig.suptitle("ESP32 room climate (live)")


def redraw(frame):
    """Called by matplotlib every REDRAW_INTERVAL_MS: reload the CSV and replot."""
    times, temps, hums = load_readings()

    ax_temp.clear()
    ax_hum.clear()

    if not times:
        ax_temp.set_title("Waiting for data... is poller.py running?")
        return

    ax_temp.plot(times, temps, color="tab:red", marker=".")
    ax_temp.set_ylabel("Temperature (C)")
    ax_temp.grid(True, alpha=0.3)

    ax_hum.plot(times, hums, color="tab:blue", marker=".")
    ax_hum.set_ylabel("Humidity (%)")
    ax_hum.grid(True, alpha=0.3)

    fig.autofmt_xdate()  # tilt the time labels so they don't overlap


if __name__ == "__main__":
    animation = FuncAnimation(fig, redraw, interval=REDRAW_INTERVAL_MS,
                              cache_frame_data=False)
    plt.show()
