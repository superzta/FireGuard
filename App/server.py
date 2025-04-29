import serial
import threading
import time
import json
from flask import Flask, render_template, jsonify, request
from flask_cors import CORS
import serial.tools.list_ports

# Flask app setup
app = Flask(__name__, static_folder='UI/assets', template_folder='UI')
CORS(app)  # Enable CORS for all routes

# Global variables to store fire detection state
fire_data = {
    "state": "no-alert",  # Possible states: no-alert, fire-alert, extinguishing, extinguished
    "max_temp": 0.0,
    "max_temp_position": [0, 0],
    "detection_time": "",
    "temperature_matrix": [],  # Will store the temperature matrix when available
    "distance": 0.0,           # Distance to fire
    "last_update": time.time(),
    "connection_status": "disconnected",
    "signal_strength": 0
}

# Serial connection
serial_connection = None

def find_arduino_port():
    """Find the serial port that the Arduino is connected to"""
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        # Common Arduino identifiers - adjust based on your specific board
        if "Arduino" in port.description or "CH340" in port.description or "USB Serial" in port.description:
            return port.device
    return None

def init_serial():
    """Initialize serial connection to the FireGuard hardware"""
    global serial_connection
    try:
        # Use the specific port instead of auto-detection
        specific_port = '/dev/cu.usbserial-A101167E'
        
        print(f"Attempting to connect to port: {specific_port}")
        serial_connection = serial.Serial(specific_port, 230400, timeout=1)
        time.sleep(2)  # Allow time for connection to establish
        
        fire_data["connection_status"] = "connected"
        print("Successfully connected to FireGuard hardware")
        return True
    except Exception as e:
        print(f"Serial connection error: {e}")
        print("Tip: Check that the device is properly connected and not in use by another program.")
        
        # Fallback to auto-detection if specific port fails
        try:
            port = find_arduino_port()
            if port:
                print(f"Attempting fallback connection to auto-detected port: {port}")
                serial_connection = serial.Serial(port, 230400, timeout=1)
                time.sleep(2)
                fire_data["connection_status"] = "connected"
                print("Successfully connected to FireGuard hardware via auto-detection")
                return True
        except Exception as fallback_error:
            print(f"Fallback connection failed: {fallback_error}")
        
        return False

def parse_temperature_matrix(matrix_data):
    """Parse the temperature matrix from the serial data"""
    try:
        matrix = []
        lines = matrix_data.strip().split('\n')
        for line in lines:
            if '|' in line:  # This is a data row
                # Extract the row number and temperature values
                parts = line.split('|', 1)
                if len(parts) > 1:
                    temp_values = parts[1].strip().split()
                    # Convert temperature values to numbers, handle "ERR" values
                    temps = []
                    for val in temp_values:
                        try:
                            temps.append(int(val))
                        except ValueError:
                            temps.append(None)  # For "ERR" values
                    matrix.append(temps)
        return matrix
    except Exception as e:
        print(f"Error parsing temperature matrix: {e}")
        return []

def read_serial_data():
    """Read and process data from the serial connection"""
    global fire_data
    
    if serial_connection is None or not serial_connection.is_open:
        print("No serial connection available. Waiting...")
        time.sleep(2)
        return
    
    print("Starting serial data reading loop. Waiting for data...")
    signal_strength = 0  # Counter for received data
    
    try:
        buffer = ""
        matrix_data = ""
        reading_matrix = False
        last_log_time = time.time()
        
        while serial_connection.is_open:
            if serial_connection.in_waiting:
                line = serial_connection.readline().decode('utf-8', errors='replace').strip()
                signal_strength += 1
                
                # Log a sample of the data every few seconds for debugging
                current_time = time.time()
                if current_time - last_log_time > 5:  # Log every 5 seconds
                    print(f"Serial signal active. Messages received: {signal_strength}")
                    print(f"Sample line: {line[:100]}")
                    last_log_time = current_time
                    fire_data["signal_strength"] = signal_strength
                    signal_strength = 0  # Reset counter
                
                # Check if we're starting to read the matrix data
                if "Center Matrix Data" in line:
                    print("Found temperature matrix data")
                    reading_matrix = True
                    matrix_data = line + "\n"
                    continue
                
                # If we're reading the matrix, accumulate the data
                if reading_matrix:
                    matrix_data += line + "\n"
                    # Check if we've reached the end of the matrix data
                    if line == "" or line == "---":
                        reading_matrix = False
                        fire_data["temperature_matrix"] = parse_temperature_matrix(matrix_data)
                        print(f"Parsed temperature matrix with {len(fire_data['temperature_matrix'])} rows")
                        continue
                
                # Process regular data lines
                if "FIRE DETECTED" in line:
                    print("FIRE DETECTION EVENT TRIGGERED")
                    fire_data["state"] = "fire-alert"
                    fire_data["detection_time"] = time.strftime("%H:%M:%S")
                    
                    # Extract temperature and position from the message
                    try:
                        # Expected format: "FIRE DETECTED! Temp: 50.00°C at [5][14]"
                        temp_part = line.split("Temp:")[1].split("at")[0].strip()
                        pos_part = line.split("at")[1].strip()
                        
                        fire_data["max_temp"] = float(temp_part.replace("°C", ""))
                        
                        # Extract position values from [row][col] format
                        row = int(pos_part.split('][')[0].replace('[', ''))
                        col = int(pos_part.split('][')[1].replace(']', ''))
                        fire_data["max_temp_position"] = [row, col]
                        print(f"Fire detected at temp: {fire_data['max_temp']}°C, position: [{row}][{col}]")
                    except Exception as e:
                        print(f"Error parsing fire detection data: {e}")
                
                elif "Motor stopped - FIRE ALERT MODE" in line:
                    print("FIRE ALERT MODE ACTIVATED")
                    fire_data["state"] = "fire-alert"
                
                elif "Alert! Temp:" in line:
                    try:
                        # Expected format: "Alert! Temp: 50.00°C at [5][14]"
                        temp_part = line.split("Temp:")[1].split("at")[0].strip()
                        pos_part = line.split("at")[1].strip()
                        
                        fire_data["max_temp"] = float(temp_part.replace("°C", ""))
                        
                        # Extract position values from [row][col] format
                        row = int(pos_part.split('][')[0].replace('[', ''))
                        col = int(pos_part.split('][')[1].replace(']', ''))
                        fire_data["max_temp_position"] = [row, col]
                    except Exception as e:
                        print(f"Error parsing temperature alert data: {e}")
                
                elif "Distance to fire:" in line:
                    try:
                        # Expected format: "Distance to fire: 120.50 cm"
                        distance_str = line.split(":")[1].split("cm")[0].strip()
                        fire_data["distance"] = float(distance_str)
                        print(f"Distance to fire: {fire_data['distance']} cm")
                    except Exception as e:
                        print(f"Error parsing distance data: {e}")
                
                elif "Fire alert mode ended" in line:
                    print("FIRE ALERT MODE ENDED")
                    fire_data["state"] = "extinguished"
                
                # Update the last update timestamp
                fire_data["last_update"] = time.time()
                
            time.sleep(0.01)  # Short sleep to prevent CPU hogging
    except Exception as e:
        print(f"Error reading serial data: {e}")
        fire_data["connection_status"] = "disconnected"
        # Attempt to reconnect
        try:
            if serial_connection and serial_connection.is_open:
                print("Closing problematic connection and attempting to reconnect...")
                serial_connection.close()
            time.sleep(2)
            init_serial()
        except Exception as reconnect_error:
            print(f"Reconnection attempt failed: {reconnect_error}")

@app.route('/')
def index():
    """Serve the main web interface"""
    return render_template('FireGuard.html')

@app.route('/api/status')
def get_status():
    """API endpoint that returns the current fire detection status"""
    return jsonify(fire_data)

@app.route('/api/reset', methods=['POST'])
def reset_status():
    """Reset the system back to monitoring state"""
    fire_data["state"] = "no-alert"
    fire_data["max_temp"] = 0.0
    fire_data["temperature_matrix"] = []
    
    # If connected to hardware, send a reset command
    if serial_connection and serial_connection.is_open:
        try:
            serial_connection.write(b'RESET\n')
        except:
            pass
    
    return jsonify({"status": "success"})

@app.route('/api/test', methods=['POST'])
def test_system():
    """Simulate a fire detection for testing the web interface"""
    # This is just for testing when hardware isn't connected
    fire_data["state"] = "fire-alert"
    fire_data["max_temp"] = 65.75
    fire_data["max_temp_position"] = [7, 14]
    fire_data["detection_time"] = time.strftime("%H:%M:%S")
    fire_data["distance"] = 125.5
    
    # Simulate a temperature matrix
    matrix = []
    for i in range(15):
        row = []
        for j in range(16):
            if i == 7 and j == 14:
                row.append(65)  # Hot spot
            elif i in range(6,9) and j in range(13,16):
                row.append(50 + (abs(i-7) + abs(j-14)))  # Heat gradient
            else:
                row.append(25 + (i+j)%10)  # Ambient temp variation
        matrix.append(row)
    
    fire_data["temperature_matrix"] = matrix
    
    return jsonify({"status": "success"})

@app.route('/api/connection_status')
def get_connection_status():
    """Check and return the status of the serial connection"""
    status_data = {
        "connected": False,
        "port": None,
        "available_ports": [],
        "last_message_time": None,
        "message_count": fire_data.get("signal_strength", 0)
    }
    
    # List all available ports
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        status_data["available_ports"].append({
            "port": port.device,
            "description": port.description,
            "manufacturer": port.manufacturer if hasattr(port, 'manufacturer') else None
        })
    
    # Check current connection
    if serial_connection and serial_connection.is_open:
        status_data["connected"] = True
        status_data["port"] = serial_connection.port
        if "last_update" in fire_data:
            status_data["last_message_time"] = fire_data["last_update"]
    
    return jsonify(status_data)

@app.route('/api/reconnect', methods=['POST'])
def reconnect_serial():
    """Force reconnection to the serial port"""
    global serial_connection
    
    # Close existing connection if open
    if serial_connection and serial_connection.is_open:
        try:
            serial_connection.close()
        except Exception as e:
            print(f"Error closing existing connection: {e}")
    
    # Try to reconnect
    success = init_serial()
    
    # Start the serial reading thread if not already running
    if success:
        if not any(thread.name == "serial_thread" for thread in threading.enumerate()):
            serial_thread = threading.Thread(target=read_serial_data, daemon=True, name="serial_thread")
            serial_thread.start()
    
    return jsonify({
        "success": success,
        "connection_status": fire_data["connection_status"]
    })

if __name__ == '__main__':
    # Start the serial reading thread
    serial_connected = init_serial()
    if serial_connected:
        serial_thread = threading.Thread(target=read_serial_data, daemon=True, name="serial_thread")
        serial_thread.start()
        print(f"Serial thread started. Connected to port: {serial_connection.port}")
    else:
        print("Running in offline mode. Use the test button to simulate fire detection.")
    
    # Start the Flask web server
    print("Starting web server on http://localhost:3000")
    app.run(host='0.0.0.0', port=3000, debug=True, use_reloader=False) 