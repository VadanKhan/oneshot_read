# ESP32-S2 Photosensitive Diode Feedback Loop

This project uses an ESP32-S2 microcontroller to read data from a photosensitive diode and output a response signal to drive a PZ mirror for a feedback loop. The system operates at high frequency to ensure precise control.

## Table of Contents
- Overview
- Hardware Requirements
- Software Requirements
- Installation
- Usage
- Configuration
- License

## Overview
The code reads analog data from a photosensitive diode using the ADC (Analog-to-Digital Converter) of the ESP32-S2. It then processes this data to generate a control signal that drives a PZ mirror using the DAC (Digital-to-Analog Converter). The feedback loop ensures that the system maintains the desired voltage level.

## Hardware Requirements
- A development board with ESP SoC (ESP32-S2 microcontroller)
- A USB cable (USBA - microA) for power supply and programming
- Photosensitive diode
- PZ mirror
- Connecting wires
- Power supply

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

## Software Requirements
- ESP-IDF (Espressif IoT Development Framework) VS code extension.

## Installation
1. **Clone the repository:**
    ```sh
    git clone <repository-url>
    cd <repository-directory>
    ```

2. **Set up the ESP-IDF environment:**
    Follow the setup instructions on the [ESP-IDF VS code extension documentation](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/install.md) to set up the development environment.

    Then *please read* the [Basic Use](https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/basic_use.md) guide to learn how to issue basic commands

3. **Set the device target :**
    You can set the device target using the command palette in VS Code.
    You only need to do this once, unless you change the device you are connecting to:
    - Open the command palette (`Ctrl+Shift+P` or `Cmd+Shift+P` on macOS).
    - Type `ESP-IDF: Set Target` and select it.
    - Choose `esp32s2` from the list of targets.

4. **Select the appropriate COM port:**
    You can select the COM port using the command palette in VS Code.
    You only need to do this once, unless you change the port you are using on your PC:
    - Open the command palette (`Ctrl+Shift+P` or `Cmd+Shift+P` on macOS).
    - Type `ESP-IDF: Select port to use` and select it.
    - Choose the COM port to which your ESP32-S2 is connected.

5. **Build the project:**
    You can build an updated project using the command palette in VS Code:
    - Open the command palette (`Ctrl+Shift+P` or `Cmd+Shift+P` on macOS).
    - Type `ESP-IDF: Build your project` and select it.

6. **Flash the firmware to the ESP32:**
    You can flash any firmware to the ESP using the command palette in VS Code:
    - Open the command palette (`Ctrl+Shift+P` or `Cmd+Shift+P` on macOS).
    - Type `ESP-IDF: Flash (UART)` and select it.

7. **Monitor the output (if expecting any):**
    You can monitor the output from the ESPusing the command palette in VS Code:
    - Open the command palette (`Ctrl+Shift+P` or `Cmd+Shift+P` on macOS).
    - Type `ESP-IDF: Monitor` and select it.
    (To exit the serial monitor, type `Ctrl+]`.)

Alternatively, you can use the following commands in the terminal:
    ```sh
    idf.py build
    idf.py -p PORT flash
    idf.py -p PORT monitor
    ```
    (To exit the serial monitor, type `Ctrl+]`.)

## Usage
1. **Connect the photosensitive diode to the ADC pins (GPIO 1 to 20 for ESP32s2) of the ESP32-S2.**
2. **Connect the PZ mirror to the DAC pins (GPIO 17 and 18 for ESP32s2) of the ESP32-S2.**
3. **Power on the ESP32.**
4. **The system will start reading data from the photosensitive diode and outputting the control signal to the PZ mirror.**

## Configuration
- **ADC Channels:**
  - ADC1 Channel 0: `EXAMPLE_ADC1_CHAN0`
  - ADC1 Channel 1: `EXAMPLE_ADC1_CHAN1`
  - ADC2 Channel 0: `EXAMPLE_ADC2_CHAN0` (if used)

- **DAC Channels:**
  - DAC Channel 0: `DAC_CHAN_0`
  - DAC Channel 1: `DAC_CHAN_1`

- **Calibration:**
  - The code includes calibration functions to ensure accurate ADC readings.

- **Feedback Parameters:**
  - `OFFSET`: Desired voltage level (default: 0.2V)
  - `P_FACTOR`: Proportional gain for the feedback loop (default: 40)

## License
This project began on 1/07/2024 for Tokamak Energy UK, by Vadan Khan and Dr. Cary Colgan.