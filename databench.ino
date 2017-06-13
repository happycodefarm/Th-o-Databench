/***************************************************
  Simple example of reading the MCP3008 analog input channels and printing
  them all out.

  Author: Carter Nelson
  License: Public Domain
****************************************************/

#include <Adafruit_MCP3008.h>
#include "DHT.h"
#include <Ticker.h>

Adafruit_MCP3008 adc0;
Adafruit_MCP3008 adc1;

#define DHTPIN 4  // D2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

Ticker ticker;

const char* ssid     = "Flatland SE";
const char* password = "allYourBase";
const char* host = "www.stagnaro.net";

void syncData(float humidity,
              int soundVolume,
              int peopleCount,
              int windSpeed,
              float temperature,
              int raining,
              int place);


void setup() {
  Serial.begin(9600);
  //while (!Serial);

  Serial.println("MCP3008 simple test.");
  //pinMode(2, OUTPUT);
  // Hardware SPI (specify CS, use any available digital)
  // Can use defaults if available, ex: UNO (SS=10) or Huzzah (SS=15)
  adc0.begin(2);
  adc1.begin(0);
  dht.begin();


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

}

void loop() {
  //for (int chan=0; chan<8; chan++) {
  //  Serial.print(adc.readADC(chan)); Serial.print("\t");
  ///}


  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");

  } else {

    Serial.print("h: ");
    Serial.print(h);
    Serial.print("%\t");
    Serial.print("t: ");
    Serial.print(t);
    Serial.print("*C\t");
  }

  for (int chan = 0; chan < 7; chan++) { // places
    int val = adc0.readADC(chan);
    Serial.print(val);
    Serial.print("\t");
  }

  int capt1 = adc1.readADC(7); // sound

  Serial.println(capt1);
  //

  //Serial.println(1024-val);

  delay(200);
}

void syncData(float humidity,
              int soundVolume,
              int peopleCount,
              int windSpeed,
              float temperature,
              int raining,
              int place) {

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/databench/databench.php?feed=true";

  Serial.print("Requesting URL: ");
  Serial.println(url);

  String json = "{\"humidity\":" + String(humidity) +
                ",\"soundVolume\":" + String(soundVolume) +
                ",\"peopleCount\":" + String(peopleCount) +
                ",\"windSpeed\":" + String(windSpeed) +
                ",\"temperature\":" + String(temperature) +
                ",\"raining\":" + String(raining) +
                ",\"place\":" + String(place) + "}";

  // This will send the json to the server
  client.println(String("POST ") + url + " HTTP/1.1");
  client.println(String("Host: ") + host);
  client.println("Accept: */*");
  client.println(String("Content-Length: ") + String(json.length()));
  client.println("Content-Type: application/json");
  client.println();
  client.println(json);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");

  delay(1000);
}
