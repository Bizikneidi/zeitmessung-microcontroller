#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_TEXT:
      USE_SERIAL.printf("[WSc] get text: %s\n", payload);

      if(strstr(reinterpret_cast<const char*>(payload), "\"Command\":\"Start\"") != NULL){
        //TODO play tune
      }

      break;
    default:
      USE_SERIAL.printf("[WSc] other: %s\n", payload);
      break;
  }

}

//TODO call sendCurrentTime
void sendCurrentTime(){
  char buf[64];
  unsigned long timeLong = millis();
  sprintf(buf, "{\"Command\":\"1\",\"Data\":\"%lu\"}", timeLong); 
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

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

}

void loop() {
  webSocket.loop();
}
