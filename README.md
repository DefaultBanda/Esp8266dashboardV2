# ESP8266 IoT Dashboard

A powerful and feature-rich IoT dashboard built using ESP8266, providing real-time weather, stock prices, 3D printer monitoring, and control capabilities. This project is ideal for people who wan to integrate multiple IoT functionalities in a single web-based interface.

This V2 fixes a few bugs which caused to esp8266 to heat up alot and crash and made the program footprint smaller so it runs alot smoother
![IMG_4189 (1)](https://github.com/user-attachments/assets/c49e404c-37e2-4438-b34f-66f13a6b6519)
![SS8266](https://github.com/user-attachments/assets/82863979-3a43-4d00-a298-62c52ce63d28)

https://github.com/user-attachments/assets/4d789f48-f256-43a6-845e-4885a30e075d


## Features

- **Real-time Weather Information**
  - Displays current weather condition and temperature.
  - Provides clothing recommendations based on weather conditions.

- **Stock Market Prices**
  - Live tracking of stock prices for AAPL, TSLA, and GOOG.

- **3D Printer Monitoring**
  - Displays current job name, progress percentage, and a visual progress bar.
  - Live MJPEG camera stream of the 3D printer.

- **3D Printer Controls**
  - Start, pause, and cancel print jobs directly from the web interface.

- **User Notes Section**
  - A simple text area for saving personal notes directly in the dashboard.

- **Live Time Display**
  - Updates every second using NTP synchronization.

- **Green-Themed Design**
  - Clean and responsive design with visually appealing green blocks.

## Getting Started

### Prerequisites

- ESP8266 (e.g., D1 Mini)
- Arduino IDE or Platform IO
- Required libraries:
  - `ESPAsyncWebServer`
  - `ESP8266HTTPClient`
  - `ArduinoJson`

### Setup

1. Clone the repository and open the project in Arduino IDE.
2. Install the required libraries from the Library Manager.
3. Update the following placeholders in the code:
   - Wi-Fi credentials (`ssid`, `password`)
   - API keys for weather, stocks, and OctoPrint.
   - URLs for OctoPrint API and MJPEG stream.
4. Upload the code to your ESP8266 board.

### Accessing the Dashboard

1. Open the Serial Monitor to get the ESP8266's IP address.
2. Enter the IP address in any web browser on the same network.

## Known Issues

- OctoPrint API keys must match your server settings.
- MJPEG stream depends on your 3D printer camera setup.

## License

This project is open-source and licensed under the MIT License.
