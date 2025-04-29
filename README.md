# FireGuard: Automated Fire Detection and Monitoring System

FireGuard is an intelligent fire detection and monitoring system designed to proactively identify fire hazards using thermal imaging technology and provide real-time alerts through a web interface.

## System Overview

FireGuard consists of three main components:
- **Hardware**: An embedded system built on AVR microcontroller with thermal imaging camera, stepper motors for scanning, and other sensors
- **Firmware**: C code running on the AVR microcontroller
- **Web Interface**: Python Flask server and responsive web UI for monitoring and control

## System Features

- **360° Thermal Scanning**: Automated patrol mode that continuously scans the environment
- **High-Precision Fire Detection**: Uses the MLX90640 thermal imaging sensor to detect heat signatures
- **Real-time Monitoring**: Web interface displays temperature data and fire detection status
- **Automatic Targeting**: When a potential fire is detected, the system locks onto the target for monitoring
- **Distance Measurement**: Uses ultrasonic sensor to measure distance to the fire
- **Alert System**: Visual and audible alerts through LCD display and buzzer
- **Remote Control**: Web interface allows remote testing and system reset

## Hardware Components

### Sensors
- **MLX90640 Thermal Camera**: 32x24 IR array thermal imaging sensor (I²C)
- **HC-SR04 Ultrasonic Sensor**: Measures distance to fire
- **16x2 LCD Display**: Provides status information

### Actuators
- **Bipolar Stepper Motors (x2)**: For horizontal and vertical scanning movements
- **Servo Motor**: For fine adjustment of thermal camera position
- **Buzzer**: For audible fire alerts

### Microcontroller
- **AVR ATmega328P**: Main processing unit running at 7.3728MHz

### Connectivity
- **UART-to-USB Converter**: For communication with the web interface (230400 baud)

## Circuit Connections

### MLX90640 Thermal Camera
- SDA → PC4 (AVR I²C Data)
- SCL → PC5 (AVR I²C Clock)
- VCC → 3.3V
- GND → GND

### Stepper Motors (using EasyDriver)
- STEP_PIN_BTM → PC1 (Bottom stepper step control)
- STEP_PIN_TOP → PC2 (Top stepper step control)
- DIR_PIN → PC3 (Direction control for both steppers)
- VCC → 5V
- GND → GND

### Ultrasonic Sensor (HC-SR04)
- TRIG → PD6
- ECHO → PD7
- VCC → 5V
- GND → GND

### LCD Display (16x2)
- RS → PB0
- E → PB1
- D4 → PB2
- D5 → PB3
- D6 → PB4
- D7 → PB5
- VCC → 5V
- GND → GND

### Buzzer
- Signal → PD5
- GND → GND

### UART Connection
- TX → PD1
- RX → PD0
- Connected to computer via USB-to-UART adapter

## Software Architecture

### Firmware (AVR C)
- **FireGuard.c**: Main program logic
- **I2C.c/h**: Communication with MLX90640 thermal camera
- **stepper.c/h**: Stepper motor control for scanning
- **servo.c/h**: Servo motor control for fine positioning
- **ultrasonic.c/h**: Distance measurement
- **buzzer.c/h**: Alert system
- **lcd.c/h**: Display interface

### Web Interface
- **server.py**: Flask server that handles serial communication and API endpoints
- **FireGuard.html**: Responsive web UI with real-time data visualization
- **assets/**: CSS, JavaScript, and image resources

## Installation and Setup

### Firmware
1. Connect the AVR programmer to your computer
2. Navigate to the Firmware directory
3. Compile and upload using the included Makefile:
   ```bash
   cd FireGuard/Firmware/src
   make
   make flash
   ```

### Web Interface
1. Install Python 3.7 or higher
2. Install required packages:
   ```bash
   cd FireGuard/App
   pip install -r requirements.txt
   ```
3. Connect the FireGuard hardware via USB
4. Run the server:
   ```bash
   python server.py
   ```
5. Open a web browser and navigate to http://localhost:3000

## Troubleshooting

### Serial Connection Issues
- The default serial port is `/dev/cu.usbserial-A101167E`. If your system uses a different port, modify the `specific_port` variable in `server.py`
- Ensure the baud rate is set to 230400
- Check USB connections and drivers

### Temperature Sensor Issues
- The MLX90640 requires a warm-up period of approximately 10 seconds
- Ensure proper I²C connections and address (0x33)

### Motor Movement Issues
- Check stepper motor connections and power supply
- Verify the EasyDriver configuration (microstepping, current limiting)

## Future Enhancements

- Integration with smart home systems (HomeKit, Google Home)
- Cloud connectivity for remote monitoring
- Machine learning for improved fire detection accuracy
- Automatic fire suppression capabilities

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributors
- Owen Zeng
- Leikhang Xiong
- Robert Zhang