# By Hailey Kaylor for Jeremy Musson at Pinyon Environmental

# Import libraries
import requests
import csv
import os
import sys
import time
import subprocess
import shutil

# Set ESP IP
ESP_IP     = "192.168.106.165"
# URL to read data from ESP immediately
URL        = f"http://{ESP_IP}/data"
# Save data from ESP in a sensor file here temporarily
OUT_FILE   = "/home/hkaylor@mines.edu/esp_data/esp_readings.csv"
# USB file location
USB_MOUNT  = "/media/hkaylor@mines.edu/bootfs"
# Backup temp csv file to USB csv file
USB_BACKUP = os.path.join(USB_MOUNT, "esp_readings.csv")
# Wait 23.5hrs
SLEEP_SEC  = 84600

# Function to turn the WiFi on
def enable_wifi():
    subprocess.run(["sudo", "ifconfig", "wlan0", "up"], check=False)
    time.sleep(10)  # allow association

# Function to turn the WiFi off
def disable_wifi():
    subprocess.run(["sudo", "ifconfig", "wlan0", "down"], check=False)

# Function to get the sensor readings from the ESP to the Pi
def fetch_readings():
    r = requests.get(URL, timeout=10)
    r.raise_for_status()
    return r.json().get("readings", [])

# Function to write data from ESP to the CSV file in the Pi
def write_csv(rows):
    os.makedirs(os.path.dirname(OUT_FILE), exist_ok=True)
    new_file = not os.path.exists(OUT_FILE)
    with open(OUT_FILE, "a", newline="") as f:
        writer = csv.writer(f)
        if new_file:
            writer.writerow(["distance_mm","YYYYMMDD","HHMMSS"])
        writer.writerows(rows)

# Function to append temp CSV file to CSV file in the USB and then clear the temp CSV
def backup_and_clear():
    try:
        # 1) Read the live CSV and skip its header
        with open(OUT_FILE, "r", newline="") as src:
            reader = csv.reader(src)
            headers = next(reader, None)
            rows = list(reader)

        if not rows:
            print("No new rows to back up.")
        else:
            # 2) Ensure the USB file exists with header
            new_usb = not os.path.exists(USB_BACKUP)
            with open(USB_BACKUP, "a", newline="") as dst:
                writer = csv.writer(dst)
                if new_usb:
                    writer.writerow(headers or ["distance_mm","YYYYMMDD","HHMMSS"])
                # 3) Append all live rows
                writer.writerows(rows)
            print(f"Appended {len(rows)} rows to {USB_BACKUP}")

        # 4) Truncate the live CSV and rewrite only its header
        with open(OUT_FILE, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(headers or ["distance_mm","YYYYMMDD","HHMMSS"])
        print(f"Cleared live CSV at {OUT_FILE}")

    except Exception as e:
        print("ERROR during backup/clear:", e, file=sys.stderr)

# Function to enable the WiFi to retrieve and save data from the ESP and then disable the WiFi
def run_once():
    enable_wifi()
    try:
        data = fetch_readings()
    except Exception as e:
        print("ERROR fetching data:", e, file=sys.stderr)
    else:
        if not data:
            print("No readings to write.")
        else:
            write_csv(data)
            print(f"Wrote {len(data)} rows to {OUT_FILE}")
    disable_wifi()

# Function to run constantly
def main():
    while True:
        # if USB stick is mounted, offload and reset live file
        backup_and_clear()
        # fetch today's readings
        run_once()
        # sleep until next cycle (24h)
        time.sleep(SLEEP_SEC)

if __name__ == "__main__":
    main()
