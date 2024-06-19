# c-rover
C as in cornette... This rover is currently a wireless toycar with camera feed. On the same wifi network, a user can acess a webpage hosted by a Pi 5 on the rover. This web page displays the camera view and a user can connect a controller to their device to drive the rover around. This means I can drive it around my house without leaving my desk.

Future plans are to include a LiDAR sensor with the IMU data for autonomous navigation, and some computer vision to do things like talk to people it sees in the house.

## Code Organization

### Chassis
Contains the code running on the ESP32 that operates the chassis. This controller 
reads UART json commands from the pi and either uses it to move wheels, or sends back information like IMU data, battery voltage, wifi info and more. It also controls the OLED display.

### Pi
Contains the code on the Pi 5. There is a UART communications test script and a camera stream that allows an end user to connect to the rover and send motor commands. The stream displays a camera and data from the chassis and reads user controller inputs to drive around.

## Parts List
- Wave-Share Rover Chassis with ESP32 PCB
- 3 18650 LiPo Batteries
- Raspberry pi 5
- 64 GB SD Card
- Desktop USB Camera
- 2D-LiDAR sensor

