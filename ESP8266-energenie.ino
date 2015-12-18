#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";
const char* password = "";
const char* mqttUsername = "";
const char* mqttPassword = "";
const char* server = "";
const char* clientNameFormat = "esp-sockets%d";
char clientName[20];

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

RCSwitch energenie = RCSwitch();

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected"); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  randomSeed(analogRead(A0));
  sprintf(clientName, clientNameFormat, random(1000));
  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName);
  
  
  if (!connectMqtt()) {
    Serial.println("Will reset and try again...");
    abort();
  }

  //setup mqtt
  client.setCallback(callback);
  
  // Transmitter is connected to Arduino Pin #10  
  energenie.enableTransmit(2);
}

bool connectMqtt() {
  if (client.connect(clientName, mqttUsername, mqttPassword)) {
    Serial.println("Connected to MQTT broker");
    client.subscribe("switches");
    client.publish("devices/connected", clientName);
    return true;
  } else {
    Serial.println("MQTT connect failed");
    return false;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received ");
  
  char code[length+1];
  for (int i=0;i<length;i++) {
    code[i]= (char)payload[i];
  }
  code[length] = '\0';
  Serial.println(code);

  energenie.send(atoi(code), 24);
}

void loop() {
  while (!client.connected()){
    connectMqtt();
    delay(1000);
  }

  client.loop();
}
