# Sensor Recordings

This repository contains code, and documentation collected from development and testing. The materials are intended to support Pinyon Environmental’s Parshall flume flow monitoring efforts, balancing accuracy and budget requirements.

## Contact Information

For any comment, questions, or concerns, please contact Hailey Kaylor at HKaylor@Mines.edu.

## Repository Structure

```
/Sensor-Recordings
├── Code/               # Scripts for data processing, sensor reading, and communication
├── Documentation/      # Wiring diagrams, setup instructions, and hardware notes
└── README.md           # This file
```

## Overview

The primary purpose of this repository is to store and organize microcontroller code and supporting documentation for a low-cost, accurate water level sensing system. This system uses an Arduino for data aggregation and an ESP8266 for accurate level sensing and wireless communication.

## Usage

- Use the code in the `Code/` folder to upload to an Arduino and ESP8266 for data collection and wireless transmission.
- The `Documentation/` folder contains setup instructions and wiring guides to ensure proper integration and deployment of the hardware.

## How to Use

1. Clone the repository:

   ```bash
   git clone https://github.com/Hekaylor/Sensor-Recordings.git
   cd Sensor-Recordings
   ```

2. Read the relevant files in the `Documentation/` directory for guidance on wiring, uploading code, and starting up the system.

3. Upload the relevant sketches in `Code/` to your Arduino and ESP8266. For example, to run the Raspberry Pi data logging script:

   ```bash
   python scripts/RaspberryPi_DataStorage.py
   ```

## Requirements

- Python 3.x
- Recommended libraries:
  - pandas
  - numpy
  - matplotlib

Install dependencies with:

```bash
pip install pandas numpy matplotlib
```

## License

This project is currently private and intended for internal use by Pinyon Environmental. Contact the repository owner for access or collaboration.
