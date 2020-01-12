#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <Hash.h>

#define COMPUTER_RELAY 5
#define FAN_RELAY 16

// @@@@@@@@@@@@@@@ You only need to midify modify wi-fi and domain info @@@@@@@@@@@@@@@@@@@@
const char* ssid     = "FASTWEB-813A0E"; //enter your ssid/ wi-fi(case sensitive) router name - 2.4 Ghz only
const char* password = "Alexinside00";     // enter ssid password (case sensitive)
char host[] = "alexagoogleifttt.herokuapp.com"; //192.168.0.100 -enter your Heroku domain name like  "alexagoogleifttt.herokuapp.com" 
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

int port = 80;
char path[] = "/ws";
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
int relayPin = 16;
DynamicJsonBuffer jsonBuffer;
String currState;
int pingCount = 0;
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) { //uint8_t *


  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected! ");
      Serial.println("Connecting...");
      webSocket.begin(host, port, path);
      webSocket.onEvent(webSocketEvent);
      break;

    case WStype_CONNECTED:
      {
        Serial.println("Connected! ");
        // send message to server when Connected
        webSocket.sendTXT("Connected");
      }
      break;

    case WStype_TEXT:
      Serial.println("Got data");
      //data = (char*)payload;
      processWebScoketRequest((char*)payload);
      break;

    case WStype_BIN:

      hexdump(payload, length);
      Serial.print("Got bin");
      // send data to server
      webSocket.sendBIN(payload, length);
      break;
  }

}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  pinMode(COMPUTER_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);

  for (uint8_t t = 4; t > 0; t--) {
    delay(1000);
  }
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");

  //Serial.println(ssid);
  WiFiMulti.addAP(ssid, password);

  //WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("Connected to wi-fi");
  webSocket.begin(host, port, path);
  webSocket.onEvent(webSocketEvent);

}

void loop() {
  webSocket.loop();
  //If you make change to delay make sure adjust the ping
  delay(2000);
  // make sure after every 40 seconds send a ping to Heroku
  //so it does not terminate the websocket connection
  //This is to keep the conncetion alive between ESP and Heroku
  if (pingCount > 20) {
    pingCount = 0;
    webSocket.sendTXT("\"heartbeat\":\"keepalive\"");
  }
  else {
    pingCount += 1;
  }
}

void processWebScoketRequest(String data) {

  JsonObject& root = jsonBuffer.parseObject(data);
  String device = (const char*)root["device"];
  String location = (const char*)root["location"];
  String state = (const char*)root["state"];
  String query = (const char*)root["query"];
  String message = "";

  Serial.println(data);
  Serial.println(state);

  if (query == "cmd" && device == "computer" && state == "on") {digitalWrite(COMPUTER_RELAY, HIGH);} 
  if (query == "cmd" && device == "fan" && state == "on") {digitalWrite(FAN_RELAY, HIGH);}
  if (query == "cmd" && device == "computer" && state == "off") {digitalWrite(COMPUTER_RELAY, LOW);} 
  if (query == "cmd" && device == "fan" && state == "off") {digitalWrite(FAN_RELAY, LOW);}  

  Serial.print("Sending response back");
  Serial.println(message);
  // send message to server
  webSocket.sendTXT(message);
  if (query == "cmd" || query == "?") {webSocket.sendTXT(message);}
}
