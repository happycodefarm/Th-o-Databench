/***************************************************
  Databench ESP12E code
  MCP3008 spi
  DHT sensor 1 wire

  Author: Guillaume Stagnaro
  License: WTFPL (Do What the Fuck You Want To Public License)
****************************************************/

#include <Adafruit_MCP3008.h>
#include "DHT.h"
#include <Ticker.h>
#include <ESP8266WiFi.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

Adafruit_MCP3008 adc0;
Adafruit_MCP3008 adc1;

#define DHTPIN 4  // D2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

Ticker ticker;

const char* ssid     = "Atelier_Hypermedias";
const char* password = "pas_de_clef_wifi";
const char* host = "databench.fr";

void syncData(float _humidity,
              float _soundVolume,
              int _peopleCount,
              int _windSpeed,
              float _temperature,
              int _raining,
              int _places);

void sync();

int soundVolumeReadings[256] = {0};
int totalSoundVolume = 0;

unsigned char soundVolumeReadingsIndex = 0;

float humidity = 0;
float soundVolume = 0;
int peopleCount = 0;
int windSpeed = 0;
float temperature = 0;
int raining = 0;
int places = 0;
int lastPlaces = 0;

void setup() {
  Serial.begin(9600);
  //while (!Serial);

  Serial.println("Hello World !");

  adc0.begin(2); // first mcp3008
  adc1.begin(0); // second mcp3008
  dht.begin();


  // connect to wifi network
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

/*  syncData(humidity,
           soundVolume,
           peopleCount,
           windSpeed,
           temperature,
           raining,
           places);*/


  // Read humidity
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    humidity = 0;
    temperature = 0;
  }

  Serial.print("h: ");
  Serial.print(humidity);
  Serial.print("%\t");
  Serial.print("t: ");
  Serial.print(temperature);
  Serial.print("*C\t");

  // read LDRs values for people counting
  int ldrCeil = 300;
  places = 0;

  for (int chan = 0; chan < 7; chan++) { // Placess
    boolean val = adc0.readADC(chan) < ldrCeil;
    places |= val << chan;
  }
  
  Serial.print("p count: ");
  //Serial.print(peopleBitMask, BIN);
  Serial.printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(places));
  Serial.print("\t");

  // here we smooth the sound volume values
  totalSoundVolume -= soundVolumeReadings[soundVolumeReadingsIndex];
  soundVolumeReadings[soundVolumeReadingsIndex] = adc1.readADC(7); // sound
  totalSoundVolume += soundVolumeReadings[soundVolumeReadingsIndex];
  soundVolumeReadingsIndex++;

  soundVolume = totalSoundVolume / 255;

  Serial.println(soundVolume);

  if (lastPlaces != places) {
    if (places != 0) {
      Serial.println("-----------------\n\nattach ticker\n\n------------");
      sync();
      //ticker.attach(10, sync);
    } else {
      Serial.print("-----------------\n\ndetach ticker\n\n------------");
      //ticker.detach();
    }
  }


  lastPlaces = places;

  delay(200);
}

void sync() {
  syncData(humidity,
           soundVolume,
           peopleCount,
           windSpeed,
           temperature,
           raining,
           places);
}

void syncData(float _humidity,
              float _soundVolume,
              int _peopleCount,
              int _windSpeed,
              float _temperature,
              int _raining,
              int _places) {

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
  String url = "/databench.php?feed=true";

  Serial.print("Requesting URL: ");
  Serial.println(url);

  String json = "{\"humidity\":" + String(_humidity) +
                ",\"soundVolume\":" + String(_soundVolume) +
                ",\"peopleCount\":" + String(_peopleCount) +
                ",\"windSpeed\":" + String(_windSpeed) +
                ",\"temperature\":" + String(_temperature) +
                ",\"raining\":" + String(_raining) +
                ",\"places\":" + String(_places) + "}";

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
