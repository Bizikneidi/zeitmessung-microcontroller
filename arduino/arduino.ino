#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsClient.h>

#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial

int flash_button = 0;
//char* host = "192.168.0.123";
//int port = 81;
//char* path = "/";

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  const char* message = reinterpret_cast<const char*>(payload);
  switch (type) {
    case WStype_TEXT: USE_SERIAL.printf("[WSc] get text: %s\n", payload);
      if (strstr(message, "\"Command\":\"StartMeasuring\"") != NULL) {
        //TODO play tune
        char buf[64];
        unsigned long timeLong = millis();
        sprintf(buf, "{\"Command\":\"MeasuredStart\",\"Data\":\"%lu\"}", timeLong);
        USE_SERIAL.printf(buf);
        webSocket.sendTXT(buf);
      }
      break;
    default: USE_SERIAL.printf("[WSc] other: %s\n", payload);
      break;
  }

}

//TODO call sendCurrentTime
void sendCurrentTime() {
  char buf[64];
  unsigned long timeLong = millis();
  sprintf(buf, "{\"Command\":\"MeasuredEnd\",\"Data\":\"%lu\"}", timeLong);
  USE_SERIAL.printf(buf);
  webSocket.sendTXT(buf);
}

void setup() {
  USE_SERIAL.begin(115200);
  USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  pinMode(flash_button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flash_button), sendCurrentTime, RISING);

  WiFiMulti.addAP("4ahif", "4ahifIstGeil");

  //WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // server address, port and URL
  webSocket.begin("192.168.0.123", 81, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  //webSocket.setAuthorization("user", "Password");
  webSocket.setAuthorization(WiFi.macAddress().c_str());

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    webSocket.loop();
  } else {
    webSocket.disconnect();
  }
}
