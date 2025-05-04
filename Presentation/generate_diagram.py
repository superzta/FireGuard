#!/usr/bin/env python3
# FireGuard System Block Diagram Generator
# This script generates a detailed block diagram of the FireGuard system
# It visualizes the flow logic, control systems, and interactions between components

import graphviz

def generate_block_diagram(output_file='fireguard_block_diagram'):
    """
    Generate a detailed block diagram of the FireGuard system
    
    Args:
        output_file: Name of the output file (without extension)
    """
    # Create a new directed graph
    dot = graphviz.Digraph(
        name='FireGuard System',
        comment='Detailed block diagram of the FireGuard fire detection system',
        format='png',
        engine='dot',
        graph_attr={
            'rankdir': 'TB',
            'splines': 'ortho',
            'nodesep': '0.8',
            'ranksep': '1.0',
            'fontname': 'Arial',
            'fontsize': '14',
            'label': 'FireGuard System Block Diagram',
            'labelloc': 't',
            'fontcolor': '#003366',
            'bgcolor': '#FFFFFF',
            'dpi': '300'
        },
        node_attr={
            'shape': 'box',
            'style': 'rounded,filled',
            'fillcolor': '#E8E8E8',
            'fontname': 'Arial',
            'fontsize': '12',
            'margin': '0.3,0.1'
        },
        edge_attr={
            'fontname': 'Arial',
            'fontsize': '10',
            'arrowsize': '0.8'
        }
    )
    
    # Create subgraphs/clusters to organize components
    with dot.subgraph(name='cluster_initialization') as init:
        init.attr(label='System Initialization', style='filled', fillcolor='#E6F3FF', 
                  fontcolor='#003366', penwidth='2', fontsize='14')
        
        init.node('init_start', 'System Start', shape='oval', fillcolor='#ADD8E6')
        init.node('init_wdt', 'Disable Watchdog Timer', fillcolor='#D7E4F4')
        init.node('init_uart', 'Initialize UART\n(230400 baud)', fillcolor='#D7E4F4')
        init.node('init_i2c', 'Initialize I²C\nfor MLX90640', fillcolor='#D7E4F4')
        init.node('init_stepper', 'Initialize Stepper Motors\nSet Counter-clockwise Direction', fillcolor='#D7E4F4')
        init.node('init_servo', 'Initialize Servo Motor', fillcolor='#D7E4F4')
        init.node('init_ultrasonic', 'Initialize Ultrasonic Sensor', fillcolor='#D7E4F4')
        init.node('init_buzzer', 'Initialize Buzzer', fillcolor='#D7E4F4')
        init.node('init_lcd', 'Initialize LCD\nHide Cursor', fillcolor='#D7E4F4')
        init.node('init_completed', 'Initialization Completed\nBegin Patrol Mode', shape='oval', fillcolor='#ADD8E6')
    
    # Patrol Mode (Main Scanning Loop)
    with dot.subgraph(name='cluster_patrol') as patrol:
        patrol.attr(label='Patrol Mode', style='filled', fillcolor='#F5F5DC', 
                    fontcolor='#553300', penwidth='2', fontsize='14')
        
        patrol.node('patrol_start', 'Patrol Mode', shape='oval', fillcolor='#FFDEAD')
        patrol.node('stepper_move', 'Move Bottom Stepper Once', fillcolor='#F5E2C8')
        patrol.node('check_direction', 'Check if Direction\nNeeds Reversal\n(Current Step >= 800?)', shape='diamond', fillcolor='#FFDEAD')
        patrol.node('reverse_direction', 'Reverse Direction\nReset Step Counter', fillcolor='#F5E2C8')
        patrol.node('check_interval', 'Check if it\'s Time\nto Read Temperature\n(Step % 20 == 0)', shape='diamond', fillcolor='#FFDEAD')
        patrol.node('read_thermal', 'Read MLX90640\nThermal Camera Data\n(Center 16×16 Region)', fillcolor='#F5E2C8')
        patrol.node('uart_status', 'Send Status via UART\nPosition and Max Temperature', fillcolor='#F5E2C8')
        patrol.node('check_threshold', 'Is Temperature\nAbove Threshold?\n(T > 50°C)', shape='diamond', fillcolor='#FFDEAD')
        patrol.node('check_position', 'Is Max Temperature\nin Target Columns?\n(13 ≤ col ≤ 15)', shape='diamond', fillcolor='#FFDEAD')
        patrol.node('adjust_direction', 'Adjust Direction\nTo Target Fire Source', fillcolor='#F5E2C8')
        patrol.node('fire_detected', 'Fire Detected!\nTransition to Alert Mode', shape='oval', fillcolor='#FFDEAD')
    
    # Alert Mode
    with dot.subgraph(name='cluster_alert') as alert:
        alert.attr(label='Fire Alert Mode', style='filled', fillcolor='#FFE4E1', 
                   fontcolor='#8B0000', penwidth='2', fontsize='14')
        
        alert.node('alert_start', 'Fire Alert Mode', shape='oval', fillcolor='#FFC0CB')
        alert.node('stop_motors', 'Stop Stepper Motors', fillcolor='#FFD7D7')
        alert.node('display_alert_lcd', 'Display Alert on LCD', fillcolor='#FFD7D7')
        alert.node('monitor_temp', 'Monitor Temperature\nContinuously', fillcolor='#FFD7D7')
        alert.node('measure_distance', 'Measure Distance\nwith Ultrasonic Sensor', fillcolor='#FFD7D7')
        alert.node('display_distance', 'Display Distance\non LCD and UART', fillcolor='#FFD7D7')
        alert.node('activate_warning', 'Activate Buzzer Warning', fillcolor='#FFD7D7')
        alert.node('oscillate_servo', 'Oscillate Servo Motor\n(0° to 105°)', fillcolor='#FFD7D7')
        alert.node('check_fire_still', 'Is Fire Still\nDetected?', shape='diamond', fillcolor='#FFC0CB')
        alert.node('alert_end', 'End Alert Mode\nReturn to Patrol', shape='oval', fillcolor='#FFC0CB')
    
    # Output/Communication Components
    with dot.subgraph(name='cluster_output') as output:
        output.attr(label='Output and Communication Systems', style='filled', fillcolor='#E6FFE6', 
                    fontcolor='#006600', penwidth='2', fontsize='14')
        
        output.node('uart_comm', 'UART Communication\n(230400 baud)', shape='box3d', fillcolor='#BAEEBA')
        output.node('lcd_display', 'LCD Display\n(16×2 Character)', shape='box3d', fillcolor='#BAEEBA')
        output.node('buzzer_output', 'Buzzer Alert\nSystem', shape='box3d', fillcolor='#BAEEBA')
        output.node('server_interface', 'Python Server\nWeb Interface', shape='box3d', fillcolor='#BAEEBA')
    
    # Sensor System Components
    with dot.subgraph(name='cluster_sensors') as sensors:
        sensors.attr(label='Sensor Systems', style='filled', fillcolor='#F0E6FF', 
                     fontcolor='#330066', penwidth='2', fontsize='14')
        
        sensors.node('thermal_camera', 'MLX90640\nThermal Camera\n(Fixed-point Processing)', shape='box3d', fillcolor='#E6D0FF')
        sensors.node('ultrasonic_sensor', 'HC-SR04\nUltrasonic Sensor', shape='box3d', fillcolor='#E6D0FF')
    
    # Motion Control Components
    with dot.subgraph(name='cluster_motion') as motion:
        motion.attr(label='Motion Control Systems', style='filled', fillcolor='#FFE6F2', 
                    fontcolor='#660033', penwidth='2', fontsize='14')
        
        motion.node('stepper_motor', 'Stepper Motors\nScanning System', shape='box3d', fillcolor='#FFD0EB')
        motion.node('servo_motor', 'Servo Motor\nOscillation System', shape='box3d', fillcolor='#FFD0EB')
    
    # Add edges for initialization flow
    dot.edge('init_start', 'init_wdt')
    dot.edge('init_wdt', 'init_uart')
    dot.edge('init_uart', 'init_i2c')
    dot.edge('init_i2c', 'init_stepper')
    dot.edge('init_stepper', 'init_servo')
    dot.edge('init_servo', 'init_ultrasonic')
    dot.edge('init_ultrasonic', 'init_buzzer')
    dot.edge('init_buzzer', 'init_lcd')
    dot.edge('init_lcd', 'init_completed')
    dot.edge('init_completed', 'patrol_start')
    
    # Add edges for patrol mode flow
    dot.edge('patrol_start', 'stepper_move')
    dot.edge('stepper_move', 'check_direction')
    dot.edge('check_direction', 'reverse_direction', label='Yes')
    dot.edge('check_direction', 'check_interval', label='No')
    dot.edge('reverse_direction', 'check_interval')
    dot.edge('check_interval', 'read_thermal', label='Yes')
    dot.edge('check_interval', 'stepper_move', label='No')
    dot.edge('read_thermal', 'uart_status')
    dot.edge('uart_status', 'check_threshold')
    dot.edge('check_threshold', 'check_position', label='Yes')
    dot.edge('check_threshold', 'adjust_direction', label='Yes, but position\nnot optimal')
    dot.edge('adjust_direction', 'stepper_move')
    dot.edge('check_threshold', 'stepper_move', label='No')
    dot.edge('check_position', 'fire_detected', label='Yes')
    dot.edge('check_position', 'stepper_move', label='No')
    dot.edge('fire_detected', 'alert_start')
    
    # Add edges for alert mode flow
    dot.edge('alert_start', 'stop_motors')
    dot.edge('stop_motors', 'display_alert_lcd')
    dot.edge('display_alert_lcd', 'monitor_temp')
    dot.edge('monitor_temp', 'measure_distance')
    dot.edge('measure_distance', 'display_distance')
    dot.edge('display_distance', 'activate_warning')
    dot.edge('activate_warning', 'oscillate_servo')
    dot.edge('oscillate_servo', 'check_fire_still')
    dot.edge('check_fire_still', 'monitor_temp', label='Yes')
    dot.edge('check_fire_still', 'alert_end', label='No')
    dot.edge('alert_end', 'patrol_start')
    
    # Connect sensor/output components
    dot.edge('thermal_camera', 'read_thermal', style='dashed')
    dot.edge('read_thermal', 'thermal_camera', style='dashed')
    dot.edge('ultrasonic_sensor', 'measure_distance', style='dashed')
    
    dot.edge('uart_status', 'uart_comm', style='dashed')
    dot.edge('uart_comm', 'server_interface', style='dashed')
    
    dot.edge('display_alert_lcd', 'lcd_display', style='dashed')
    dot.edge('display_distance', 'lcd_display', style='dashed')
    
    dot.edge('activate_warning', 'buzzer_output', style='dashed')
    
    dot.edge('stepper_move', 'stepper_motor', style='dashed')
    dot.edge('oscillate_servo', 'servo_motor', style='dashed')
    
    # Add memory optimization highlight
    with dot.subgraph(name='cluster_memory') as memory:
        memory.attr(label='Memory Optimization Features', style='filled', fillcolor='#FFFFD9', 
                    fontcolor='#666600', penwidth='2', fontsize='14')
        
        memory.node('fixed_point', 'Fixed-Point Arithmetic\ninstead of Floating-Point\n(32.5°C → 3250)', shape='note', fillcolor='#FFFFB2')
        memory.node('reduced_resolution', 'Reduced Resolution\nProcessing\n(16×16 center region\ninstead of 32×24 full array)', shape='note', fillcolor='#FFFFB2')
        memory.node('buffer_reuse', 'Buffer Reuse Pattern\n(Single 48-byte buffer for\nall string operations)', shape='note', fillcolor='#FFFFB2')
    
    # Connect optimization features to related components
    dot.edge('fixed_point', 'read_thermal', style='dotted')
    dot.edge('reduced_resolution', 'read_thermal', style='dotted')
    dot.edge('buffer_reuse', 'uart_status', style='dotted')
    
    # Add threshold detection highlight
    with dot.subgraph(name='cluster_detection') as detection:
        detection.attr(label='Fire Detection Logic', style='filled', fillcolor='#FFE6CC', 
                      fontcolor='#CC6600', penwidth='2', fontsize='14')
        
        detection.node('temp_threshold', 'Temperature Threshold\n(50.00°C)', shape='note', fillcolor='#FFDAB9')
        detection.node('position_validation', 'Position Validation\n(Columns 13-15 considered\nvalid fire detection zone)', shape='note', fillcolor='#FFDAB9')
        detection.node('direction_adjustment', 'Intelligent Direction\nAdjustment Logic\n(Move stepper to center\nfire in detection zone)', shape='note', fillcolor='#FFDAB9')
    
    # Connect detection logic to related components
    dot.edge('temp_threshold', 'check_threshold', style='dotted')
    dot.edge('position_validation', 'check_position', style='dotted')
    dot.edge('direction_adjustment', 'adjust_direction', style='dotted')
    
    # Render the graph
    try:
        dot.render(output_file, cleanup=True)
        print(f"Block diagram successfully generated as {output_file}.png")
    except Exception as e:
        print(f"Error generating block diagram: {e}")
        
if __name__ == "__main__":
    generate_block_diagram()
    
    # Also generate SVG version for higher quality
    generate_block_diagram('fireguard_block_diagram_svg') 