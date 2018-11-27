#define DEBUG_WEBSOCKETS(...) os_printf( __VA_ARGS__ )
#ifndef DEBUG_WEBSOCKETS
#define DEBUG_WEBSOCKETS(...)
#define NODEBUG_WEBSOCKETS
#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial

//define commands
#define START_MEASURING "\"Command\":0,\"Data\":"
#define STOP_MEASURING "\"Command\":1,\"Data\":"

#define MEASURED_START "{\"Command\":2,\"Data\":%lu}"
#define MEASURED_STOP "{\"Command\":3,\"Data\":%lu}"

//define global variables
int flash_button = 0;
int led_pin = 2;
bool measuring = false;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  const char* message = reinterpret_cast<const char*>(payload);
  if (type == WStype_TEXT) {
    USE_SERIAL.printf("[WSc] get text: %s\n", payload);
    if (strstr(message, START_MEASURING) != NULL) {
      
      //put current time into MEASURED_START
      char buf[64];
      unsigned long timeLong = millis();
      sprintf(buf, MEASURED_START, timeLong);
      //send MEASURED_START command
      USE_SERIAL.printf(buf);
      webSocket.sendTXT(buf);
      
      measuring = true;
      //light LED for start
      digitalWrite(led_pin, LOW);
      delay(1000);
      digitalWrite(led_pin, HIGH);
    }
    else if (strstr(message, STOP_MEASURING) != NULL) {
      measuring = false;
    }
  }
  else {
    USE_SERIAL.printf("[WSc] other: %s\n", payload);
  }
}

void sendCurrentTime() {
  
  //check if measurment is running
  if (measuring) {
    
    //put current time into MEASURED_STOP
    char buf[64];
    unsigned long timeLong = millis();
    sprintf(buf, MEASURED_STOP, timeLong);
    //send MEASURED_STOP
    USE_SERIAL.printf(buf);
    webSocket.sendTXT(buf);
  }
  else {
    USE_SERIAL.printf("Can't stop, no measurment running!\n");
  }
}

void setup() {
  //Debug info
  USE_SERIAL.begin(115200);
  USE_SERIAL.setDebugOutput(true);

  //set LED for start light
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);

  //set button for stop and attach stop function
  pinMode(flash_button, INPUT);
  attachInterrupt(digitalPinToInterrupt(flash_button), sendCurrentTime, RISING);

  //WIFI SSID and password
  WiFiMulti.addAP("5AHIF", "RoflCopter");

  //try as long as the WIFI is not connected
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // server address, port and URL
  webSocket.begin("172.18.2.16", 5001, "/station");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
}
