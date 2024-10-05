import socket
from inputs import get_gamepad

# Set up socket connection to ESP8266
try:
    host = '192.168.1.10'  # Replace with the IP address of your ESP8266
    port = 80
    esp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    esp_socket.connect((host, port))
except socket.error as e:
    print(f"Error connecting to ESP8266: {e}")
    exit(1)


def handle_event(type, event):
    message = f'{type} {event.code} {event.state}\n'
    esp_socket.sendall(message.encode())
    # time.sleep(0.1)
    print(message)


def send_gamepad_inputs():
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
        send_gamepad_inputs()
    except KeyboardInterrupt:
        print("Exiting...")
    finally:
        esp_socket.close()
