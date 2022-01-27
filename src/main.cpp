#include <Arduino.h>
#if defined(ESP32)  
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Wire.h>
#include <BH1750.h>
#include <DHTesp.h> // Click here to get the library: http://librarymanager/All#DHTesp
#include <ThingsBoard.h>
#include "device.h"
#include "wifi_id.h"
#include "thingsboard_id.h"

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
#define THINGSBOARD_SERVER       "demo.thingsboard.io"

WiFiClient espClient;
ThingsBoard tb(espClient);

DHTesp dht;
BH1750 lightMeter;

void WifiConnect();
void onSendSensor();

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
#if defined(ESP8266)  
  pinMode(LED_YELLOW, OUTPUT);
#endif
  pinMode(PIN_SW, INPUT_PULLUP);
  dht.setup(PIN_DHT, DHTesp::DHT11);
  Wire.begin(PIN_SDA, PIN_SCL);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);

  WifiConnect();
  if (tb.connect(THINGSBOARD_SERVER, THINGSBOARD_ACCESS_TOKEN))
    Serial.println("Connected to thingsboard");
  else
    Serial.println("Error connected to thingsboard");
  Serial.println("System ready.");
  digitalWrite(LED_BUILTIN, LED_BUILTIN_OFF);
}

void loop() {
  if (millis() % 3000 ==0 && tb.connected())
    onSendSensor();
  tb.loop();
}

void onSendSensor()
{
  digitalWrite(LED_BUILTIN, LED_BUILTIN_ON);
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  float lux = lightMeter.readLightLevel();
  if (dht.getStatus()==DHTesp::ERROR_NONE)
  {
    Serial.printf("Temperature: %.2f C, Humidity: %.2f %%, light: %.2f\n", 
      temperature, humidity, lux);
    if (tb.connected())
    {
      tb.sendTelemetryFloat("temperature", temperature);
      tb.sendTelemetryFloat("humidity", humidity);
    }
  }
  else
    Serial.printf("Light: %.2f lx\n", lux);

  tb.sendTelemetryFloat("light", lux);
  digitalWrite(LED_BUILTIN, LED_BUILTIN_OFF);
}

void WifiConnect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }  
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}
