# ESP32 WiFi Health Monitor Firmware

This project is the embedded side of a full ESP32 monitoring system. It runs on an ESP32-S3 board, connects to Wi-Fi, monitors connectivity and device health, and publishes telemetry to Firebase Realtime Database for a browser-based dashboard to visualize.

The firmware is designed to be practical, readable, and easy to extend for makers, students, and developers building connected IoT systems.

## Overview

The firmware does the following:

- Connects to Wi-Fi and keeps a resilient connection loop
- Starts a local provisioning flow if no saved credentials are available
- Tracks device information and firmware metadata
- Measures Wi-Fi quality, RSSI, temperature, and heap usage
- Sends periodic telemetry to Firebase
- Exposes a lightweight local web interface for setup and diagnostics
- Drives status LEDs for system feedback

This project is intended to work alongside the dashboard in this repository, creating an end-to-end ESP32 monitoring workflow.

## Features

- ESP32-S3 support through PlatformIO
- Automatic Wi-Fi provisioning with saved credentials
- Firebase Authentication and Realtime Database integration
- Real-time device registration and telemetry uploads
- LED state feedback for boot, Wi-Fi, Firebase, and health status
- Local web server support for setup and debugging
- LittleFS filesystem support for optional web assets

## Hardware

This firmware targets:

- ESP32-S3 development board
- RGB LED support
- 16MB flash configuration
- Wi-Fi enabled operation

The default board configuration in the project is:

- Board: esp32-s3-devkitc-1
- Framework: Arduino
- Flash: 16MB

## Project Structure

```text
firmware/
├── include/               # Public headers for each module
│   ├── device_config.h
│   ├── device_manager.h
│   ├── firebase_manager.h
│   ├── led_manager.h
│   ├── logger.h
│   ├── time_manager.h
│   ├── web_server.h
│   └── wifi_manager.h
├── src/                   # Firmware implementation files
│   ├── device_manager.cpp
│   ├── firebase_manager.cpp
│   ├── led_manager.cpp
│   ├── logger.cpp
│   ├── main.cpp
│   ├── time_manager.cpp
│   ├── web_server.cpp
│   └── wifi_manager.cpp
├── data/                  # Files served by the local web server
├── dummy.env              # Example environment template
├── extra_script.py        # Loads .env values into compile-time defines
├── platformio.ini         # PlatformIO configuration
├── README.md              # This file
└── .env                   # Local environment file (not committed)
```

## Prerequisites

Before building or flashing the firmware, make sure you have:

- PlatformIO installed
- A compatible ESP32-S3 board connected over USB
- A Firebase project configured for Authentication and Realtime Database
- A Wi-Fi network available for the device to join

## Configuration

Create a local .env file in the firmware folder before building.

A template is included in [firmware/dummy.env](dummy.env).

Example:

```env
# Firebase configuration
FIREBASE_API_KEY=your_firebase_api_key
FIREBASE_PROJECT_ID=your_project_id
FIREBASE_DATABASE_URL=https://your-project-default-rtdb.firebaseio.com
FIREBASE_USER_EMAIL=your-firebase-user@example.com
FIREBASE_USER_PASSWORD=your-firebase-password

# Device configuration
EDGE_NAME=ESP32-001
DEVICE_MODEL=ESP32-S3-N16R8
FIRMWARE_VERSION=1.0.0
```

The project reads this file through [firmware/extra_script.py](extra_script.py), validates the required variables, and injects them into the C++ build as compile-time definitions.

> Important: never commit a real .env file to a public repository. Keep credentials out of version control.

## Quick Start

1. Open a terminal in the firmware folder:

```bash
cd firmware
```

2. Create your local environment file:

```bash
cp dummy.env .env
```

3. Fill in your Firebase and device values.

4. Build and upload the firmware:

```bash
pio run -t upload
```

5. If you want to upload the filesystem assets as well:

```bash
pio run -t uploadfs
```

6. Open the serial monitor:

```bash
pio device monitor
```

## Build and Flash Commands

Common PlatformIO commands:

```bash
pio run
pio run -t upload
pio run -t uploadfs
pio run -t monitor
pio run -t clean
```

## Firebase Integration

The device authenticates with Firebase using the configured user email and password, then writes telemetry and metadata to Firebase Realtime Database.

The runtime logic registers the device under a path like:

```text
/devices/<device-id>/info
```

and then publishes periodic metrics under telemetry paths such as:

```text
/devices/<device-id>/telemetry
```

This structure is designed to be consumed by the dashboard frontend in this repository.

## Wi-Fi Behavior

The firmware is designed to be resilient in the field:

- It attempts to connect using previously saved Wi-Fi credentials
- If no credentials are present, it enters provisioning mode
- It can accept new credentials and save them only after a successful connection
- It reports connection and health status through LEDs and the serial console

## Local Web Server

The project includes a web server module for local setup and diagnostic tasks. This helps with device configuration and operational visibility during development and testing.

## Serial Logging

The firmware uses a logger module for structured console output. During boot and operation, it prints status details such as:

- Wi-Fi state
- Firebase connection state
- Device registration status
- Telemetry collection progress
- Errors and warnings

## Troubleshooting

### Build fails because .env is missing

Make sure you created a .env file in the firmware folder before running PlatformIO.

### Firebase authentication fails

Check the following:

- FIREBASE_API_KEY is correct
- FIREBASE_USER_EMAIL exists in your Firebase project
- FIREBASE_USER_PASSWORD is valid
- Authentication is enabled for the project
- Realtime Database is initialized

### Wi-Fi never connects

Verify:

- SSID and password are correct
- The board is within range of the access point
- The network is not using unsupported security settings
- The device is not stuck in provisioning mode unexpectedly

### Telemetry not appearing in Firebase

Check:

- The board is connected to Wi-Fi
- Firebase auth succeeds
- DATABASE_URL is correct
- The device path is valid and the database rules allow writes

## Security Notes

This project is intended for educational, maker, and internal use. For production deployments, consider:

- Restricting Firebase database rules
- Using a dedicated service account or secure user model
- Avoiding hardcoded secrets in source files
- Validating the telemetry data received by the backend

## Contributing

Contributions are welcome. If you want to improve the project:

1. Fork the repository
2. Create a feature or fix branch
3. Improve the firmware, documentation, or integration flow
4. Test on hardware where possible
5. Open a pull request with a clear description

## Community Use

This firmware is a good foundation for:

- ESP32 based monitoring systems
- Wi-Fi health dashboards
- IoT learning projects
- Smart device telemetry collectors
- Embedded systems experiments with Firebase

If you build on this project, consider sharing your improvements with the community so others can learn from your work.

## License

This project is intended for open learning and community-driven development. If you are publishing a fork, please keep attribution clear and respect the repository license if one is added later.
