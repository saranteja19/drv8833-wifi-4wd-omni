from inputs import get_gamepad
import serial
import time

# Configure the serial port and baud rate
try:
    arduino = serial.Serial('COM9', 115200)  # Adjust the port as necessary
    time.sleep(2)  # Wait for Arduino to reset
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    exit(1)


def handle_event(type, event):
    message = f'{type} {event.code} {event.state}\n'
    arduino.write(message.encode())
    print(message)


def print_gamepad_inputs():
    while True:
        events = get_gamepad()
        for event in events:
            if event.ev_type == 'Key':
                handle_event('B', event)
            elif event.ev_type == 'Absolute':
                if event.code in ['ABS_X', 'ABS_Y', 'ABS_Z', 'ABS_RX', 'ABS_RY', 'ABS_RZ']:
                    handle_event('A', event)
                elif event.code in ['ABS_HAT0X', 'ABS_HAT0Y']:
                    handle_event('D', event)


if __name__ == "__main__":
    try:
        print_gamepad_inputs()
    except KeyboardInterrupt:
        print("Exiting...")
    finally:
        arduino.close()
