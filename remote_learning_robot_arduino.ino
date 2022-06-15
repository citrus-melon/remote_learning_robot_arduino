#include "credentials.h"
#include <AccelStepper.h>

#include <ESP8266WiFi.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>


const uint32_t connectTimeoutMs = 10000;

AccelStepper cameraStepper(AccelStepper::DRIVER, 13, 15);
SocketIOclient socketIO;

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
            long newPosition = data[1];
            cameraStepper.moveTo(newPosition);
            if (!offsetRecieved) {
              offsetRecieved = true;
              cameraStepper.setCurrentPosition(newPosition);
            }
            break;
    }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  cameraStepper.setAcceleration(600);
  cameraStepper.setMaxSpeed(200);

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
  cameraStepper.run();
}