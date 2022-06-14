#include "credentials.h"
#include <Stepper.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>


const uint32_t connectTimeoutMs = 10000;
// const int ledPin = LED_BUILTIN;

Stepper cameraStepper(200, 13, 15);
ESP8266WiFiMulti wiFiMulti;
SocketIOclient socketIO;

// int ledState = LOW;
// unsigned long previousMillis = 0;
// int ledInterval = 1000;

int currentRotation = 0;
int rotationTarget = 0;
bool offsetRecieved = false;

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            Serial.printf("Server disconnected!");
            break;
        case sIOtype_CONNECT:
            Serial.printf("Server connected!");
            // join default namespace (no auto join in Socket.IO V3)
            socketIO.send(sIOtype_CONNECT, "/");
            offsetRecieved = false;
            break;
        case sIOtype_EVENT:
            DynamicJsonDocument data(1024);
            deserializeJson(data, payload);
            const String eventType = data[0];
            if (!eventType.equals("rotationUpdate")) break;
            rotationTarget = data[1];
            if (!offsetRecieved) {
              offsetRecieved = true;
              currentRotation = rotationTarget;
            }
            Serial.println(rotationTarget);
            break;
    }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  cameraStepper.setSpeed(100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println(WiFi.status());
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  socketIO.begin(SERVER_HOSTNAME, SERVER_PORT, "/socket.io/?EIO=4");
  socketIO.onEvent(socketIOEvent);
}

void loop() {
  socketIO.loop();

  if (currentRotation < rotationTarget) {
    cameraStepper.step(1);
    currentRotation++;
  } else if (currentRotation > rotationTarget) {
    cameraStepper.step(-1);
    currentRotation--; 
  }

  // LED blinking
  // unsigned long currentMillis = millis();
  // if (currentMillis - previousMillis >= ledInterval) {
  //   previousMillis = currentMillis;
  //   if (ledState == LOW) {
  //     ledState = HIGH;
  //   } else {
  //     ledState = LOW;
  //   }
  //   digitalWrite(ledPin, ledState);
  // }
}