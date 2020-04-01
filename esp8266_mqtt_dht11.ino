#include <ESP8266WiFi.h>
#include <DHT.h>
#include <PubSubClient.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define MQTT_SERVER ""
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASS ""
#define PIN_DATA 2

WiFiClient wifiClient;
PubSubClient client(wifiClient);
DHT dht(PIN_DATA, DHT11);

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println("Let' start now");

  connectWifi();
  connectMqtt();

  dht.begin();
}

void loop() {
  client.loop();

  char temperature[5];
  dtostrf(dht.readTemperature(), 4, 2, temperature);
  client.publish("casa/sala/temperatura", temperature);
  
  char humidity[5];
  dtostrf(dht.readHumidity(), 4, 2, humidity);
  client.publish("casa/sala/umidade", humidity);

  delay(5000);
}

void connectWifi() {
  Serial.println("Connecting to wifi..");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectMqtt() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String clientName;
  clientName += "esp8266-";
  clientName += macToStr(mac);

  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);

  while (!client.connected()) {
    if (client.connect((char*) clientName.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("Connected to MQTT Server");
    } else {
      Serial.print("[FAILED] [ rc = ");
      Serial.print(client.state() );
      Serial.println(" : retrying in 1 seconds]");
      delay(1000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}
