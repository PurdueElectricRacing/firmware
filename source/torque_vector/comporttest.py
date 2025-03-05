import serial
import serial.tools.list_ports


def list_ports():
    """List available serial ports."""
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]


def monitor_serial(port, baudrate=9600, timeout=1):
    """Monitor a serial port for incoming data."""
    try:
        with serial.Serial(port, baudrate, timeout=timeout) as ser:
            print(f"Monitoring {port}... (Press Ctrl+C to stop)")
            while True:
                if ser.in_waiting > 0:
                    data = ser.read(ser.in_waiting)  # Read available data
                    print(f"Received: {data}")
    except serial.SerialException as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    available_ports = list_ports()
    if not available_ports:
        print("No serial ports found.")
    else:
        print("Available ports:", available_ports)
        port = input("Enter port to monitor: ")
        monitor_serial(port)
