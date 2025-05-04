# FireGuard Project
## Software Implementation Details

Team Members:
- Owen Zeng
- Leikhang Xiong
- Robert Zhang

--

## Project Overview

- **FireGuard**: Automated fire detection and monitoring system
- **Software Focus**: Memory optimization, thermal imaging, and real-time web interface

*Note: Hardware components and 3D printed model will be covered separately*

**Image instruction**: Create a software architecture diagram showing the three main software components: Firmware (AVR C), Communication Layer (Serial/UART), and Web Interface (Python/HTML/JS), with arrows showing data flow between components. Use circuit board imagery for firmware, data packets for communication, and a modern web UI for interface.

--

## Memory Optimization Challenges

### ATmega328P Limitations:
- 2KB SRAM (Working Memory)
- 32KB Flash (Program Storage)

### The Memory Challenge:
- Thermal camera produces 768 temperature points (32×24 resolution)
- Standard floating-point storage would require ~3KB
- Additional memory needed for processing and operation

**Image instruction**: Create a visual representation showing an ATmega328P microcontroller with memory constraints highlighted. Show a thermal camera producing data that exceeds the available memory. Use size-proportional containers to visualize the mismatch between data requirements and available memory.

--

## Fixed-Point Arithmetic Solution

### The Problem:
- Floating-point operations require significant memory
- Example: A single `float` uses 4 bytes

### Our Solution:
- Store temperatures as fixed-point integers (centidegrees)
- Example: 32.5°C → 3250
- Operations performed on integers, then scaled for display

### Memory Impact:
- Reduced memory usage by ~2KB
- Simplified calculations with minimal accuracy loss

**Image instruction**: Create a comparison showing floating-point vs fixed-point representation. Show a thermometer with temperature readings and how they're stored in memory. Include memory usage comparison through visualization (e.g., memory blocks filled vs. empty).

--

## Resolution Optimization

### Thermal Camera Full Resolution:
- MLX90640 provides 32×24 pixel thermal image (768 points)
- All points require processing and storage

### Our Optimization:
- Focus on center 16×16 region (256 points)
- 67% reduction in data processing requirements
- Maintained detection accuracy for targeted region

**Image instruction**: Create a 3D visualization of the MLX90640 thermal camera capturing a scene, with the full 32×24 grid shown, and the center 16×16 region highlighted in a different color. Show example heat signatures within this region to emphasize that critical data is preserved.

--

## I2C Communication Architecture

### ATmega328P & MLX90640 Thermal Camera Integration:
- Communication via I²C protocol
- No hardware I²C controller available with required features

### Implementation Method:
- Bit-banged I²C implementation using GPIO pins
- Direct port manipulation for precise timing control
- Customized for MLX90640's specific requirements

**Image instruction**: Create a timing diagram showing I²C communication signals (SDA and SCL) with START condition, address byte, data transfer, and STOP condition. Include an illustration of the MLX90640 thermal camera connected to an ATmega328P with I²C lines highlighted.

--

## MLX90640 Thermal Data Processing

### Technical Challenge:
1. Raw sensor data requires complex calibration
2. Memory-efficient processing algorithm needed

### Processing Pipeline:
1. Read raw infrared data from sensor registers
2. Apply calibration factors and offset compensation
3. Convert to absolute temperatures
4. Find maximum temperature and its position in array
5. Output formatted temperature matrix

**Image instruction**: Create a flowchart visualizing the thermal data processing pipeline. Show the MLX90640 camera with raw data outputting to processing blocks for calibration, conversion, detection, and visualization. Use heat map coloring (blue to red) to show the transformation from raw data to interpreted temperatures.

--

## Stepper Motor Control System

### Design Requirements:
- Precise 360° scanning motion
- Position tracking for fire source targeting
- Synchronized operation with thermal sensing

### Implementation Approach:
- Direct port register manipulation for maximum speed
- Microstepping control for smooth motion
- Adaptive scanning pattern based on thermal readings

**Image instruction**: Create a technical illustration of a stepper motor with control lines connected to an ATmega328P. Show the scanning pattern with directional arrows indicating how the system changes scanning behavior when detecting elevated temperatures. Include timing diagrams for the step and direction signals.

--

## Ultrasonic Sensor Distance Measurement

### Technical Challenge:
- Accurate distance measurement to fire source
- Precise timing for echo pulse measurement

### Implementation Method:
- Hardware timer capture for microsecond precision
- Interrupt-driven measurement process
- Automatic timeout detection for out-of-range readings

### Effect on System:
- Allows distance-based decision making
- Enhances situational awareness in fire scenarios

**Image instruction**: Create a technical illustration showing ultrasonic waves traveling from the sensor to a fire source and back. Include a timing diagram showing the relationship between trigger pulse, echo pulse, and distance calculation. Show the ATmega microcontroller with timer registers visualized receiving the signal.

--

## Timer & PWM Architecture

### Timer Usage in FireGuard:
1. **Timer0**: System timekeeping
2. **Timer1**: Ultrasonic echo pulse measurement
3. **Timer2**: Servo motor control via PWM

### PWM Implementation:
- Fast PWM mode for precise servo positioning
- Configurable prescalers for optimal timing resolution
- Direct register configuration for maximum efficiency

**Image instruction**: Create a diagram of the AVR timer architecture showing the three timer blocks with their respective functions. For Timer2/PWM, show the waveform generation with duty cycle control and how it connects to the servo motor. Include register configurations visualized as control panels.

--

## Interrupt-Driven Architecture

### Interrupt Sources:
- Pin change interrupts (PCINT) for ultrasonic echo detection
- Timer overflow interrupts for timeouts
- UART receive interrupts for command handling

### Benefits:
- Real-time responsiveness to external events
- Efficient CPU utilization 
- Reliable timing-critical operations

**Image instruction**: Create a visualization of the interrupt architecture showing multiple interrupt sources connecting to the microcontroller's interrupt controller. Show how normal program flow is suspended when interrupts occur, with priority levels indicated. Use a timeline visualization showing how interrupts are handled in real-time.

--

## Fire Detection Algorithm

### Detection Strategy:
1. Continuous thermal scanning of environment
2. Temperature threshold monitoring (50°C default)
3. Position validation to reduce false positives
4. Tracking and targeting when fire detected

### Algorithm Refinements:
- Spatial temperature gradient analysis
- Temporal pattern recognition
- Position-based threshold adjustments

**Image instruction**: Create a visualization showing the thermal scanning process with a heat map overlay. Illustrate how the algorithm detects a fire source, validates its position, and then tracks it. Show decision thresholds with temperature gradients and highlight the fire detection zone.

--

## UART Communication Protocol

### Communication Architecture:
- High-speed serial link (230400 baud)
- Binary and text-based data formats
- Optimized for real-time temperature matrix transmission

### Protocol Design:
- Simple command/response structure
- Efficient data packaging
- Error detection and recovery mechanisms

**Image instruction**: Create a diagram showing the UART communication between the FireGuard device and computer. Visualize the data packets with different sections (headers, commands, data payloads). Include a timing diagram showing the serial data transmission at the bit level.

--

## Flask Server Architecture

### Server Components:
1. **Serial Interface**: Communicates with FireGuard hardware
2. **Data Processor**: Parses and interprets sensor data
3. **State Manager**: Tracks fire detection status
4. **REST API**: Provides data to web interface
5. **Event Handler**: Manages system commands and responses

### Implementation Features:
- Multithreaded design for responsive UI during data processing
- Automatic reconnection handling
- Data validation and error recovery

**Image instruction**: Create a software architecture diagram of the Flask server showing its components and data flow. Visualize the threading model with the main thread handling API requests and a background thread processing serial data. Show how data flows from the hardware through the server to the web interface.

--

## Web Interface Design

### UI Components:
1. **Status Dashboard**: Overall system state
2. **Temperature Matrix Visualization**: Heat map display
3. **Alert Panel**: Fire detection notifications
4. **Control Interface**: System testing and reset

### Technical Implementation:
- Responsive design for mobile and desktop
- Dynamic temperature visualization using CSS grid
- AJAX polling for real-time updates
- Color mapping algorithm for intuitive temperature display

**Image instruction**: Create a mockup of the FireGuard web interface showing the main components. Include a heat map visualization of the temperature matrix with color gradients from blue (cool) to red (hot), and show how a fire detection would be highlighted. Include control buttons and status indicators.

--

## Memory Optimization Results

| Technique | Before | After | Savings |
|-----------|--------|-------|---------|
| Fixed-point arithmetic | 3.0 KB | 1.0 KB | 2.0 KB |
| Reduced matrix size | 2.5 KB | 0.8 KB | 1.7 KB |
| Buffer reuse | 0.7 KB | 0.2 KB | 0.5 KB |

**System Impact**:
- From exceeding memory limits to stable operation
- Reliable fire detection with no false positives
- Real-time performance maintained

**Image instruction**: Create a bar chart comparing memory usage before and after optimizations. Use a visual representation of the ATmega328P's 2KB SRAM limit as a horizontal line, showing how the original implementation exceeded this limit while the optimized version fits within constraints. Use different colors for different optimization techniques.

--

## Software-Hardware Integration

### Integration Challenges:
- Timing-critical operations across multiple peripherals
- Limited I/O pins requiring multiplexing
- Power management considerations

### Integration Solutions:
- Carefully designed state machine architecture
- Prioritized interrupt handling
- Efficient resource sharing

*Note: Detailed hardware connections will be covered in hardware section*

**Image instruction**: Create a system integration diagram showing the software components (state machine, interrupt handlers, communication modules) connecting to hardware components (sensors, motors, communication interfaces). Use a layered architecture approach to visualize how software and hardware interact.

--

## Future Software Enhancements

- **Machine Learning Integration**: Pattern recognition for improved detection
- **Cloud Connectivity**: Remote monitoring and notifications
- **Advanced Fire Analysis**: Spread prediction and containment strategies
- **System Integration**: Building management and smart home APIs

**Image instruction**: Create a futuristic visualization of the FireGuard system with enhanced capabilities. Show cloud connectivity, mobile notifications, integration with smart home systems, and advanced analytics capabilities. Use a forward-looking design aesthetic to emphasize the future potential.

--

## Questions?

**FireGuard Project Team**:
- Owen Zeng
- Leikhang Xiong
- Robert Zhang

**Image instruction**: Create a clean, professional closing slide with the FireGuard logo (a shield with flame icon in modern, minimalist style) and space for team photos. Use a dark background with accent colors that match the FireGuard interface theme.