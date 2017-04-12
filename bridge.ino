#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <RCSwitch.h>

#include <stdint.h>

#define WIFI_SSID "..."
#define WIFI_PASSWORD "..."

#define MQTT_TOPIC "switch/rf/+"

#define TX_PIN 4 // Receiver on GPIO4 / D2.
#define TX_REPEAT 15

RCSwitch mySwitch = RCSwitch();
WiFiClient espClient;
PubSubClient client(espClient);

char hostString[16];
IPAddress brokerIp;
uint16_t brokerPort;

void setup() {
  Serial.begin(115200);
  sprintf(hostString, "ESP_%06X", ESP.getChipId());

  /* Setup transmitter */
  mySwitch.enableTransmit(TX_PIN);
  mySwitch.setRepeatTransmit(TX_REPEAT);
  
  delay(10);
  setup_wifi();
  if(!find_raspberry()){
    // reboot?
  }
  Serial.print("IP: ");
  Serial.print(brokerIp);
  Serial.printf(", Port: %d\n", brokerPort);
  client.setServer(brokerIp, brokerPort);
  client.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  uint8_t protocol;
  uint16_t pulselen;
  char* code;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // In future handle out of bounds check on topic (strcmp + len)
  int switchNr = atoi((const char*)(topic + sizeof(MQTT_TOPIC) - 2)); //-2, 1 for null-termination, 1 for wildcard +
  Serial.printf("Switch: %d\n", switchNr);

  // Parse using strtok in format `protocol|pulselen|bincode`
  char *tmp = (char*)payload;
  char* tok = strtok(tmp, "|");
  if(tok){
    protocol = atoi(tok);
    tok = strtok(NULL, "|");
    if(tok && protocol <= 5){
      pulselen = atoi(tok);
      tok = strtok(NULL, "|");
      if(tok){
        code = tok;
        
        Serial.printf("prot: %u, pulselen: %u, code: %s\n", protocol, pulselen, code);
        mySwitch.setProtocol(protocol,pulselen);
        mySwitch.send(code);
      }
    }
  }
}


/* Discover raspberry using MDNS */
boolean find_raspberry() {
  if (!MDNS.begin(hostString)) {
    Serial.println("Error setting up MDNS responder!");
    return false;
  } else {
    Serial.println("MDNS responder started...");
    delay(10);

    /* Start discovery of MDNS (e.g. avahi) service advertisements for MQTT */
    int n = -1;
    do {
      if(!n) {
        delay(1000);
        Serial.println("Did not find MQTT broker, retrying...");
      }
      n = MDNS.queryService("mqtt", "tcp");
    } while(n == 0);
    Serial.println("MDNS broker found!");

    brokerIp = MDNS.IP(0);
    brokerPort = MDNS.port(0);
    return true;
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.hostname(hostString);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe to the topic
      client.subscribe(MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

