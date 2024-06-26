#include <Preferences.h>
#include <nvs_flash.h>
Preferences preferences;

#include "math.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include "WebPage.h"
WebServer server(80);
#include <INA219_WE.h>

#include <ArduinoJson.h>
DynamicJsonDocument jsonCmdReceive(256);
DynamicJsonDocument jsonInfoSend(1024);

TaskHandle_t serialCtrlHandle;
TaskHandle_t spdGetHandle;

#include "IMU.h"
#include "config.h"
#include "speedGet.h"

#include "IRCutCtrl.h"
#include "motorCtrl.h"
#include "pwmServoCtrl.h"
#include "screenCtrl.h"
#include "powerInfo.h"
//#include "busServoCtrl.h"

#include "baseFunctions.h"
#include "connectionFuncs.h"


// EMERGENCY_STOP: {"T":0}
// SPEED_INPUT   : {"T":1,"L":0.5,"R":0.5}
// PID_SET       : {"T":2,"P":0.01,"I":90}

// OLED_SET      : {"T":3,"lineNum":0,"Text":"nigga"}
// OLED_DEFAULT  : {"T":-3}

// PWM_SERVO_CTRL    : {"T":40,"pos":90,"spd":30}
// PWM_SERVO_MID     : {"T":-4}

// BUS_SERVO_CTRL    : {"T":50,"id":1,"pos":2047,"spd":500,"acc":30}
// BUS_SERVO_MID     : {"T":-5,"id":1}
// BUS_SERVO_SCAN    : {"T":52,"num":20}
// BUS_SERVO_INFO    : {"T":53,"id":1}
// BUS_SERVO_ID_SET  : {"T":54,"old":1,"new":2}
// BUS_SERVO_TORQUE_LOCK  : {"T":55,"id":1,"status":1}
// BUS_SERVO_TORQUE_LIMIT : {"T":56,"id":1,"limit":500}
// BUS_SERVO_MODE    : {"T":57,"id":1,"mode":0}

// WIFI_SCAN       : {"T":60}
// WIFI_TRY_STA    : {"T":61}
// WIFI_AP_DEFAULT : {"T":62}
// WIFI_INFO       : {"T":65}
// WIFI_OFF        : {"T":66}

// INA219_INFO     : {"T":70}
// IMU_INFO        : {"T":71}
// ENCODER_INFO    : {"T":73}
// DEVICE_INFO     : {"T":74}

// IO_IR_CUT       : {"T":80,"status":1}

// SET_SPD_RATE    : {"T":901,"L":1.0,"R":1.0}
// GET_SPD_RATE    : {"T":902}
// SPD_RATE_SAVE   : {"T":903}

// GET_NVS_SPACE   : {"T":904}
// NVS_CLEAR       : {"T":905}


void setup() {
  Wire.begin(S_SDA, S_SCL);
  Serial.begin(UART_BAUD);
//  Serial1.begin(UART_BAUD, SERIAL_8N1, 16, 17);
//  Serial1.println("HELLO");
  preferences.begin("config", false);

  while(!Serial){}
  pinInit();
  IRIO_Init();
  pwmServoInit();
  pwmServoCtrl(90);
  InitINA219();
//  busServoInit();

  InitScreen();
  allDataUpdate();

  wifiInit();
  imuInit();

  xTaskCreate(&commandThreading, "serialCtrl", 4000, NULL, 5, &serialCtrlHandle);
  // encoder threading does nothing
  xTaskCreate(&spdGetThreading, "motorSpeedGet", 4000, NULL, 5, &spdGetHandle);

//  webServerSetup();
  Serial.println("Setup complete");
}


void loop() {
//  Serial.println("HELLO");
  getIMU();
  Serial.println(currentMillis);

  if(currentMillis - lastCmdTime > HEART_BEAT){
    setpointA = 0;
    setpointB = 0;
  }

  outputB = constrain(speedPIControl_B(outputB, setpointB, KpInputSet)*rightRate, -255, 255);
  outputA = constrain(speedPIControl_A(outputA, setpointA, KpInputSet)*leftRate, -255, 255);
//  Serial.print(outputB);
//  Serial.print("  ");
//  Serial.println(outputA);
  // outputB = constrain(speedPIControl_B(inputB, setpointB, KpInputSet), -255, 255);
 
  rightCtrl(outputB);
  leftCtrl(outputA);
  // rightCtrl(outputB);

  // linearSpeedPrint();

  if(currentMillis - lastScreenUpdateMillis > screenUpdateInterval){
    lastScreenUpdateMillis = currentMillis;

    InaDataUpdate();
    getWifiStatus();
    allDataUpdate();
  }
//  Serial.println("loop");
  delay(interval);
}
