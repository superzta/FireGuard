# FireGuard Web Interface

This web application provides a user interface for the FireGuard fire detection system. It connects to the FireGuard hardware via serial communication and displays real-time fire detection data.

## Features

- Real-time monitoring of fire detection status
- Temperature data visualization with heat map
- Fire alerts with detailed information
- Remote system test and reset capabilities

## Prerequisites

- Python 3.7 or higher
- FireGuard hardware with firmware installed
- USB connection to the FireGuard device

## Installation

### Using the Setup Scripts (Recommended)

#### On macOS/Linux:

```bash
cd /path/to/FireGuard/App
chmod +x setup.sh
./setup.sh
```

#### On Windows:

```cmd
cd \path\to\FireGuard\App
setup.bat
```

### Manual Installation

1. Create a virtual environment:

```bash
# macOS/Linux
python3 -m venv venv
source venv/bin/activate

# Windows
python -m venv venv
venv\Scripts\activate
```

2. Install the required packages:

```bash
pip install -r requirements.txt
```

## Running the Application

1. Make sure the FireGuard hardware is connected to your computer via USB.

2. Activate the virtual environment (if not already activated):

```bash
# macOS/Linux
source venv/bin/activate

# Windows
venv\Scripts\activate
```

3. Run the server:

```bash
python server.py
```

4. Open a web browser and go to http://localhost:3000

## Usage

The web interface automatically connects to the FireGuard hardware when started. It will display:

- **All Clear**: When no fire is detected
- **Fire Alert**: When a fire is detected, showing the temperature matrix as a heat map
- **Fire Extinguished**: When a fire has been detected and subsequently extinguished

### Controls

- **Run Test**: Simulates a fire detection event (useful for testing without actual fire)
- **System Check**: Performs a system self-test
- **Emergency Override**: For manually controlling the system during a fire event
- **Reset System**: Resets the system after a fire has been extinguished

### Running Without Hardware

For testing and demonstration purposes, you can:

1. Start the Python server without hardware connected - it will provide simulated data through the test endpoint
2. Use the "Run Test" button to simulate a fire detection event

## Troubleshooting

If the web interface shows "Disconnected" in the status indicator:

1. Make sure the FireGuard hardware is properly connected via USB
2. Check that the correct serial port is being detected (the server prints this information on startup)
3. Try manually specifying your serial port by editing the `specific_port` variable in `server.py`
4. Restart the server with the hardware connected
5. If problems persist, check the Arduino IDE's serial monitor to verify the hardware is working correctly

## Developer Information

- The server runs on Flask and communicates with the web interface via JSON API endpoints at port 3000
- Real-time updates are implemented using AJAX polling
- Temperature matrix visualization uses CSS grid and dynamic color mapping 