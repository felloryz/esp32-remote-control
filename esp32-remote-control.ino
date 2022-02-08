#include <WiFi.h>
#include <WiFiUdp.h>

#define LED_BUILTIN 33

// WiFi network name and password:
const char * networkName = "MGTS_GPON_73A8";
const char * networkPswd = "NSLPRG5N";

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address
const char * udpAddress = "89.108.114.104";
const int udpPort = 50000;

typedef enum ledStatusVar {
  LED_STATUS,
  LED_ON,
  LED_OFF,
  LED_SWITCH
} ledStatusVar;

//Are we currently connected?
boolean connected = false;

//The udp library class
WiFiUDP udp;

char incomingPacket[1446];

void setup(){
  // Initilize hardware serial:
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED works with inverted logic
  
  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);

  delay(1000);

  int i = 0;
  while(1) {
    if(connected) {
      udp.beginPacket(udpAddress,udpPort);
      udp.printf("ESP_CODE");
      udp.endPacket();
      Serial.printf("Send ESP_CODE #%d\n", i++);
    }
    unsigned long currentMillis = millis();
    while(millis() - currentMillis < 1000) {
      int packetSize = udp.parsePacket();
      if (packetSize) {
        Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
        int len = udp.read(incomingPacket, 1446);
        if(len > 0) {
          incomingPacket[len] = 0;
        }
        Serial.printf("UDP packet contents: %s\n", incomingPacket);
      }
    }
    if(strcmp(incomingPacket, "VERIFIED") == 0) {
      if(connected) {
        udp.beginPacket(udpAddress,udpPort);
        udp.printf("VERIFIED");
        udp.endPacket();
        Serial.printf("Send VERIFIED\n");
      }
      Serial.printf("CONNECTED TO SERVER\n");
      break;
    }
  }
  
}

void loop(){
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
    int len = udp.read(incomingPacket, 1446);
    if(len > 0) {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);

    udp.beginPacket(udpAddress,udpPort);
    //udp.beginPacket(udp.remoteIP(),udp.remotePort());
    if(strcmp(incomingPacket, "LED_STATUS") == 0) {
      LedStatus(LED_STATUS);
    }
    else if(strcmp(incomingPacket, "LED_ON") == 0) {
      LedStatus(LED_ON);
    }
    else if(strcmp(incomingPacket, "LED_OFF") == 0) {
      LedStatus(LED_OFF);
    }
    else if(strcmp(incomingPacket, "LED_SWITCH") == 0) {
      LedStatus(LED_SWITCH);;
    }
    else {
      udp.printf("ERROR MESSAGE");
    }
    /*
    switch(incomingPacket) {
      case "LED_STATUS":
        LedStatus(0) == true ? udp.printf("LED_ON") : udp.printf("LED_OFF");
        break;
      case "LED_ON":
        LedStatus(1) == true ? udp.printf("LED_ON") : udp.printf("LED_OFF");
        break;
      case "LED_OFF":
        LedStatus(2) == true ? udp.printf("LED_ON") : udp.printf("LED_OFF");
        break;
      case "LED_SWITCH":
        LedStatus(3) == true ? udp.printf("LED_ON") : udp.printf("LED_OFF");
        break;
    }
    */
    udp.endPacket();
  }

  /*
  //only send data when connected
  if(connected){
    //Send a packet
    udp.beginPacket(udpAddress,udpPort);
    udp.printf("Seconds since boot: %lu", millis()/1000);
    udp.endPacket();
  }
  
  unsigned long currentMillis = millis();
  while(millis() - currentMillis < 1000) {
    int packetSize = udp.parsePacket();
    if (packetSize) {
      Serial.printf("Received %d bytes from %s, port %d\n", packetSize, udp.remoteIP().toString().c_str(), udp.remotePort());
      int len = udp.read(incomingPacket, 1446);
      if(len > 0) {
        incomingPacket[len] = 0;
      }
      Serial.printf("UDP packet contents: %s\n", incomingPacket);
    }
  }
  */
  
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(WiFi.localIP(),udpPort);
          connected = true;
          break;
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
}

void LedStatus(int ledStatusVar) {
  switch(ledStatusVar) {
    case LED_STATUS:
      if(digitalRead(LED_BUILTIN) == LOW)
        udp.printf("LED ON");
      else
        udp.printf("LED OFF");
      break;
    case LED_ON:
      digitalWrite(LED_BUILTIN, LOW);
      udp.printf("LED ON");
      break;
    case LED_OFF:
      digitalWrite(LED_BUILTIN, HIGH);
      udp.printf("LED OFF");
      break;
    case LED_SWITCH:
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      if(digitalRead(LED_BUILTIN) == LOW)
        udp.printf("LED ON");
      else
        udp.printf("LED OFF");
      break;
    default:
      udp.printf("Incorrect ledStatusVar");
      break;
  }
}
