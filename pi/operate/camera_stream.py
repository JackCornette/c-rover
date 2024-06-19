import cv2
from flask import Flask, render_template, Response, jsonify
from flask_socketio import SocketIO, emit
import serial
import json
import threading
import time

app = Flask(__name__)
socketio = SocketIO(app)

# Set up the serial connection (adjust the port and baud rate as necessary)
ser = serial.Serial('/dev/ttyAMA0', 1000000, timeout=1)

# Function to send DEVICE_INFO command periodically
def send_device_info():
    device_info_command = json.dumps({"T": 74}) + '\n'
    while True:
        ser.write(device_info_command.encode('utf-8'))
        time.sleep(5)

# Start the DEVICE_INFO sender thread
device_info_thread = threading.Thread(target=send_device_info)
device_info_thread.daemon = True
device_info_thread.start()

# Function to read serial data
def read_serial():
    while True:
        data = ser.readline().decode('utf-8').strip()
        if data:
            # print(f"Received: {data}")
            try:
                data_json = json.loads(data)
                socketio.emit('robot_data', data_json)
            except json.JSONDecodeError:
                pass

# Start the serial reader thread
serial_read_thread = threading.Thread(target=read_serial)
serial_read_thread.daemon = True
serial_read_thread.start()

def gen_frames():
    camera = cv2.VideoCapture(0)
    camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    camera.set(cv2.CAP_PROP_FPS, 15)

    while True:
        success, frame = camera.read()
        if not success:
            break
        else:
            ret, buffer = cv2.imencode('.jpg', frame, [int(cv2.IMWRITE_JPEG_QUALITY), 90])
            frame = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

@app.route('/video_feed')
def video_feed():
    return Response(gen_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/')
def index():
    return render_template('index.html')

@socketio.on('control')
def handle_control(data):
    left_speed = data.get('left_speed', 0)
    right_speed = data.get('right_speed', 0)
    if (abs(left_speed) < 0.05): left_speed = 0 
    if (abs(right_speed) < 0.05): right_speed = 0 
    json_message = json.dumps({"T": 1, "L": left_speed, "R": right_speed})
    ser.write(json_message.encode('utf-8') + b'\n')
    print(f'Sent: {json_message}')
    emit('response', {'status': 'success', 'left_speed': left_speed, 'right_speed': right_speed})

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)