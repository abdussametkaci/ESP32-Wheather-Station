#include <ESPmDNS.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SPIFFS.h>

#include "esp_wpa2.h"

// Libraries for SD card
#include "FS.h"
#include <SPI.h>
#include <SD.h>


// Library for JSON Data
#include <Arduino_JSON.h>

typedef struct {
  bool has_wifi;
  bool wpa2_enterprise;
  String ssid = ""; // FSMVU
  String username =  ""; // ad.soyad@fsm.edu.tr adresindeki @'den onceki kisim -> ad.soyad
  String password = "";
  String hostname = "";
} wifi_configuraion_t;


AsyncWebServer server(80);

typedef struct {
  float temperature[3]; // index: 0->current data, 1->max value, 2->min value
  float humidity[3];
} datas_t;

Adafruit_BME280 bme; // I2C
datas_t datas;
wifi_configuraion_t wifi_config;
JSONVar json_data;

String dataMessage = "";
bool stopped_saving = false;
bool getFirstData = false;

void setup() {
  Serial.begin(115200);
  //SPI.begin();
  //SPI.setClockDivider(SPI_CLOCK_DIV32);
  Serial.println("BME280 test");

  if (!bme.begin(0x76)) {
    Serial.println("Failed to start sensor! Please check your wiring.");
  }


  datas.temperature[2] = bme.readTemperature();
  datas.temperature[2] = format(datas.temperature[2], 2);

  // Initialize SPIFFS

  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }

  // Initialize SD card
  
  SD.end();
  if (!SD.begin()) { // CS PIN
    Serial.println("Card Mount Failed");

  }


  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");

  }

  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  if (!SD.exists("/data.csv")) {
    Serial.println("Creating file...");
    appendFile(SD, "/data.csv", "Temperature(C),Humidity(%)\n");
  }

  if (SD.exists("/config.txt")) {
    Serial.println("Configuration wifi settings...");
    SD.end();
    SD.begin();
    File file = SD.open("/config.txt", FILE_READ);
    String json = file.readString();
    file.close();
    JSONVar jsonObject = JSON.parse(json.c_str());
    wifi_config.has_wifi = (bool) jsonObject["has_wifi"];
    wifi_config.wpa2_enterprise = (bool)jsonObject["wpa2_enterprise"];
    wifi_config.username = jsonObject["username"];
    wifi_config.ssid = jsonObject["ssid"];
    wifi_config.password = jsonObject["password"];
    wifi_config.hostname = jsonObject["hostname"];

  }
  


  if(!wifi_config.has_wifi){
    // access point mode -> kendi internet agini yayar
    WiFi.mode(WIFI_AP);
    WiFi.softAP(wifi_config.ssid.c_str(), wifi_config.password.c_str());
    // Print ESP32 Local IP Address
    Serial.print("\nAP IP: ");
    Serial.println(WiFi.softAPIP());
  }
  else if (wifi_config.wpa2_enterprise) {

    // WPA2 enterprise
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);  // burasi cok onemli normalde default olarak WiFi mode STA'dir
    // ancak wifi kapatildiginda modu belirtilmesi gerekmektedir
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)wifi_config.username.c_str(), strlen(wifi_config.username.c_str()));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)wifi_config.username.c_str(), strlen(wifi_config.username.c_str()));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)wifi_config.password.c_str(), strlen(wifi_config.password.c_str()));
    esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); //set config settings to default
    esp_wifi_sta_wpa2_ent_enable(&config); //set config settings to enable function


    WiFi.begin(wifi_config.ssid.c_str());
    Serial.print("Connecting to WiFi..");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    // Print ESP32 Local IP Address
    Serial.print("\nLocal IP: ");
    Serial.println(WiFi.localIP());

  } else {
    // Connect to Wi-Fi
    WiFi.begin(wifi_config.ssid.c_str(), wifi_config.password.c_str());
    Serial.println("Connecting to WiFi..");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    // Print ESP32 Local IP Address
    Serial.print("\nLocal IP: ");
    Serial.println(WiFi.localIP());

  }

  // http://esp32.local adresi ile cihaza erisilebilir
  if (!MDNS.begin(wifi_config.hostname.c_str())) {
    Serial.println("Error starting mDNS");
  }

  Serial.print("\nhostname: http://");
  Serial.print(wifi_config.hostname);
  Serial.println(".local");
  

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    if(wifi_config.has_wifi){
      request->send(SPIFFS, "/index_wifi.html");
    } else {
      request->send(SPIFFS, "/index.html");
    }
    
  });


  server.on("/data", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", bme280_datas().c_str());
  });


  server.on("/download", HTTP_GET, [](AsyncWebServerRequest * request) {
    stopped_saving = true;
    SD.end();
    SD.begin();
    request->send(SD, "/data.csv", "text/csv", true);
    stopped_saving = false;
  });

  server.begin();

}
uint32_t timer = millis();
void loop() {
  if (millis() - timer > 1000) {
    timer = millis();

    datas.temperature[0] = bme.readTemperature();
    datas.temperature[0] = format(datas.temperature[0], 2);
    datas.humidity[0] = bme.readHumidity();
    datas.humidity[0] = format(datas.humidity[0], 2);

    if (!getFirstData) {
      if (datas.temperature[0] != 0 && datas.humidity[0] != 0) {
        datas.temperature[2] = datas.temperature[0];
        datas.humidity[2] = datas.humidity[0];
        getFirstData = true;
      }

    }


    if (datas.temperature[0] > datas.temperature[1]) { // max temperature
      datas.temperature[1] = datas.temperature[0];
    }

    if (datas.temperature[0] < datas.temperature[2]) { // min temperature
      datas.temperature[2] = datas.temperature[0];
    }

    if (datas.humidity[0] > datas.humidity[1]) { // max humidity
      datas.humidity[1] = datas.humidity[0];
    }

    if (datas.humidity[0] < datas.humidity[2]) { // min humidity
      datas.humidity[2] = datas.humidity[0];
    }

    // server'dan sd karttaki dosya indirildigi zaman dosyaya yazim yapilmayacak
    // o yuzden kayit durdurulur
    if (!stopped_saving) {
      logSDCard();
    }

  }

}


String bme280_datas() {
  //json_data["sensor"] = "bme280";
  json_data["temperature"] = String(datas.temperature[0]); // datayi formatlayip float deerden atsak bile cop data gidiyor
  // ancak String olarak yolladigimda ise noktadan sonra 2 basamka gitektedir.
  // web sayfasinda ise bunu tekrar float olarak cast ediyorum
  json_data["humidity"] = String(datas.humidity[0]);

  json_data["temperature_max"] = String(datas.temperature[1]);
  json_data["humidity_max"] = String(datas.humidity[1]);

  json_data["temperature_min"] = String(datas.temperature[2]);
  json_data["humidity_min"] = String(datas.humidity[2]);

  String jsonString = JSON.stringify(json_data);
  return jsonString;
}

// Write the sensor readings on the SD card
void logSDCard() {
  dataMessage = String(datas.temperature[0]) + "," + String(datas.humidity[0]) + "\n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/data.csv", dataMessage.c_str());
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);
  SD.end();
  SD.begin();
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);
  SD.end();
  SD.begin();
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

// formatting float number
float format(float f_value, int digit) {
  float basamak = pow(10, digit);
  f_value *= basamak;
  f_value = ((int)f_value) / basamak;
  return f_value;
}
