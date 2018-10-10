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
#define START_MEASURING "\"Command\":0"
#define MEASURED_START "{\"Command\":\"MeasuredStart\",\"Data\":%lu}"
#define MEASURED_STOP "{\"Command\":\"MeasuredStop\",\"Data\":%lu}"

int flash_button = 0;
int mes_counter = 0;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  const char* message = reinterpret_cast<const char*>(payload);
  if (type == WStype_TEXT) {
    USE_SERIAL.printf("[WSc] get text: %s\n", payload);
    if (strstr(message, START_MEASURING) != NULL) {
      //TODO play tune
      char buf[64];
      unsigned long timeLong = millis();
      sprintf(buf, MEASURED_START, timeLong);
      USE_SERIAL.printf(buf);
      webSocket.sendTXT(buf);
      mes_counter++;
    }
  }
  else {
    USE_SERIAL.printf("[WSc] other: %s\n", payload);
  }
}

void sendCurrentTime() {
  if (mes_counter > 0) {
    char buf[64];
    unsigned long timeLong = millis();
    sprintf(buf, MEASURED_STOP, timeLong);
    USE_SERIAL.printf(buf);
    webSocket.sendTXT(buf);
    mes_counter--;
  }
  else {
    USE_SERIAL.printf("Can't stop, no measurment running!");
  }
}

void setup() {
  USE_SERIAL.begin(115200);
  USE_SERIAL.setDebugOutput(true);

  pinMode(flash_button, INPUT);
  attachInterrupt(digitalPinToInterrupt(flash_button), sendCurrentTime, RISING);

  //WIFI SSID and password
  WiFiMulti.addAP("4ahif", "4ahifIstGeil");

  //try as long as the WIFI is not connected
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // server address, port and URL
  webSocket.begin("172.18.2.16", 53933, "/station");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
}
