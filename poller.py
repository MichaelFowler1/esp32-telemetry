"""Stage 2: poll the ESP32 on an interval and append readings to a CSV."""

import csv
import time
from datetime import datetime
from pathlib import Path

import requests

ESP32_URL = "http://10.0.0.58/data"
TIMEOUT_S = 5           # give up if the ESP32 doesn't answer within 5 seconds
POLL_INTERVAL_S = 30    # seconds between readings
CSV_PATH = Path(__file__).parent / "readings.csv"

CSV_COLUMNS = ["timestamp", "temperature_c", "humidity_pct", "uptime_ms"]


def fetch_reading():
    """Ask the ESP32 for one sensor reading. Returns a dict, or None on failure."""
    try:
        response = requests.get(ESP32_URL, timeout=TIMEOUT_S)
    except requests.exceptions.RequestException as err:
        print(f"Request failed: {err}")
        return None

    if response.status_code != 200:
        print(f"ESP32 answered but reported a problem (HTTP {response.status_code}): {response.text}")
        return None

    return response.json()


def append_reading(reading):
    """Add one row to the CSV ledger, writing the header first if the file is new."""
    is_new_file = not CSV_PATH.exists()

    with open(CSV_PATH, "a", newline="") as f:
        writer = csv.writer(f)
        if is_new_file:
            writer.writerow(CSV_COLUMNS)
        writer.writerow([
            datetime.now().isoformat(timespec="seconds"),
            reading["temperature_c"],
            reading["humidity_pct"],
            reading["uptime_ms"],
        ])


def run(poll_interval_s, max_polls=None):
    """Poll forever (or max_polls times), logging each good reading to the CSV."""
    polls_done = 0
    while max_polls is None or polls_done < max_polls:
        reading = fetch_reading()
        if reading is not None:
            append_reading(reading)
            print(f"{datetime.now():%H:%M:%S}  "
                  f"{reading['temperature_c']} C  {reading['humidity_pct']} %  -> logged")
        polls_done += 1
        if max_polls is None or polls_done < max_polls:
            time.sleep(poll_interval_s)


if __name__ == "__main__":
    print(f"Polling {ESP32_URL} every {POLL_INTERVAL_S}s. Ledger: {CSV_PATH}")
    print("Press Ctrl+C to stop.")
    try:
        run(POLL_INTERVAL_S)
    except KeyboardInterrupt:
        print("\nStopped. The CSV keeps everything logged so far.")
