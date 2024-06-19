import serial
import argparse
import threading

""" 
To run, find the port connected via uart, should be /dev/ttyAMA0 on pi OS:

python3 uart_test.py /deb/ttyAMA0

Some useful commands:
  SPEED_INPUT     : {"T":1,"L":0.5,"R":0.5}
  EMERGENCY_STOP  : {"T":0}
  IMU_INFO        : {"T":71}
  INA219_INFO     : {"T":70}
  DEVICE_INFO     : {"T":74}

"""

def read_serial():
    """Function to continuously read data from the serial port."""
    while True:
        data = ser.readline().decode('utf-8')
        if data:
            print(f"Received: {data}", end='')

def main():
    global ser
    
    # Set up argument parsing for the serial port
    parser = argparse.ArgumentParser(description='Serial JSON Communication')
    parser.add_argument('port', type=str, help='Serial port name (e.g., /dev/ttyAMA0 or COM1)')

    args = parser.parse_args()

    # Configure the serial connection
    ser = serial.Serial(args.port, baudrate=1000000, dsrdtr=None)
    ser.setRTS(False)
    ser.setDTR(False)

    # Start the thread to read from the serial port
    serial_recv_thread = threading.Thread(target=read_serial)
    serial_recv_thread.daemon = True
    serial_recv_thread.start()

    try:
        while True:
            command = input("Enter command: ")
            ser.write(command.encode() + b'\n')
    except KeyboardInterrupt:
        print("Program terminated by user.")
    finally:
        ser.close()

if __name__ == "__main__":
    main()