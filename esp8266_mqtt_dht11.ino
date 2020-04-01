#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Thread.h>
#include <ThreadController.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define MQTT_SERVER ""
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASS ""
#define MQTT_TEMP_TOPIC ""
#define MQTT_HUMID_TOPIC ""
#define PIN_DADOS 2
#define LED_STATUS 1

WiFiClient wifiClient;
PubSubClient client(wifiClient);
ThreadController cpu = ThreadController();
DHT dht(PIN_DADOS, DHT11);

Thread ledWifiThread = Thread();
void ledWifiOnRun() {
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
  }
}

Thread ledMqttThread = Thread();
void ledMqttOnRun() {
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
  }
}

Thread ledOkThread = Thread();
void ledOkOnRun() {
  if (WiFi.status() == WL_CONNECTED && client.connected()) {
    digitalWrite(LED_STATUS, LOW);
  }
}

Thread wifiThread = Thread();
void wifiOnRun() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

Thread mqttThread = Thread();
void mqttOnRun() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  if (!client.connected()) {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    String clientName;
    clientName += "esp8266-";
    clientName += macToStr(mac);
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqttCallback);
    client.connect((char*) clientName.c_str(), MQTT_USER, MQTT_PASS);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
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

Thread readDHTAndPublishThread = Thread();
void readDHTAndPublishOnRun() {
  if (WiFi.status() != WL_CONNECTED || !client.connected())
    return;

  digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));

  char temperature[5];
  dtostrf(dht.readTemperature(), 4, 2, temperature);
  client.publish(MQTT_TEMP_TOPIC, temperature);

  char humidity[5];
  dtostrf(dht.readHumidity(), 4, 2, humidity);
  client.publish(MQTT_HUMID_TOPIC, humidity);

  delay(10);
  digitalWrite(LED_STATUS, !digitalRead(LED_STATUS));
}

void setup() {
  pinMode(LED_STATUS, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  dht.begin();

  ledWifiThread.setInterval(100);
  ledWifiThread.onRun(ledWifiOnRun);

  ledMqttThread.setInterval(500);
  ledMqttThread.onRun(ledMqttOnRun);

  ledOkThread.setInterval(1000);
  ledOkThread.onRun(ledOkOnRun);

  wifiThread.setInterval(20000);
  wifiThread.onRun(wifiOnRun);

  mqttThread.setInterval(10000);
  mqttThread.onRun(mqttOnRun);

  readDHTAndPublishThread.setInterval(5000);
  readDHTAndPublishThread.onRun(readDHTAndPublishOnRun);

  cpu.add(&ledWifiThread);
  cpu.add(&ledMqttThread);
  cpu.add(&ledOkThread);
  cpu.add(&wifiThread);
  cpu.add(&mqttThread);
  cpu.add(&readDHTAndPublishThread);
}

void loop() {
  cpu.run();
  client.loop();
}
