import csv
import sys
import math
import serial
from datetime import datetime
from pathlib import Path

PORT = "/dev/cu.usbserial-10"
BAUD = 115200
OUTPUT_FILE = Path("audio_features.csv")

i=0

FEATURE_NAMES = ( ["peak_hz", "centroid_hz"]
    + [f"mfcc_mean_{i}" for i in range(1, 14)]
    + [f"mfcc_std_{i}" for i in range(1, 14)]
)

label = input("Sound label (noise/clap/speech/music): ").strip().lower()

if not label:
    raise ValueError("Label needed")

write_header = not OUTPUT_FILE.exists() or OUTPUT_FILE.stat().st_size == 0
with serial.Serial(PORT, BAUD, timeout=2) as ser:
    print("Connected. waiting for vectors")

    with OUTPUT_FILE.open("a", newline="") as file:
        writer = csv.writer(file)

        if write_header:
            writer.writerow(["timestamp","label"]+ FEATURE_NAMES)

        try:
            while True:
                line = ser.readline().decode(
                    "utf-8", errors="replace"
                ).strip()

                if not line:
                    continue

                values = line.split(",")

                if len(values)!=28:
                    print(f"Ignored: {line}")
                    continue

                try:
                    features = [float(value) for value in values]
                except ValueError:
                    print(f"Not numeric; ignored {line}")
                    continue

                if not all(math.isfinite(x) for x in features):
                    print("Invalid. Ignored")
                    continue

                timestamp = datetime.now().isoformat(timespec="seconds")
                writer.writerow([timestamp, label]+features)
                file.flush()
                i+=1
                print(f"saved row {i} | label={label} | {features[26]} {features[27]}")

                if i==200:
                    print("\n Collection stopped.")
                    print(f"dataset saved to {OUTPUT_FILE.resolve()}")
                    sys.exit()

        except KeyboardInterrupt:
            print("\n Collection stopped.")

print(f"dataset saved to {OUTPUT_FILE.resolve()}")
        