// this sample code provided by www.programmingboss.com
//#include <SoftwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define soil_rx 3 // data pin from soil sensor
#define soil_pwr 18 // pwn pin to power sensor
#define sleep_time 14400000000
const int air_moisture = 1024;
const int water_moisture = 500;

const char* ssid = "NETGEAR29";
const char* password = "festiveunicorn516";
const char* serverName = "http://192.168.0.22/soil_monitor/post_esp_data.php";
const char* plantName = "phineas"; // Designates which plant's table to put data into

int rawSoilMoisture;  // Will hold the raw analog read data from the sensor
int percentMoisture;  // Hold a usable percent moisture level to transmit to database

int bootCount = 0;

String apiKeyValue = "tPmAT5Ab3j7F9";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  initWiFi();
  analogReadResolution(10);
  pinMode(soil_pwr, OUTPUT);
  esp_sleep_enable_timer_wakeup( sleep_time);
}
void loop() {
  if(bootCount%4==0)
  {
   // Serial.println(Serial1.readStringUntil('\n'));
   if(WiFi.status()==WL_CONNECTED)
   {
      WiFiClient client;
      HTTPClient http;

      http.begin(client, serverName);
      http.addHeader("Content-Type","application/x-www-form-urlencoded");
  
      delay(5000);

      readSoilLevel();
      Serial.print("Raw moisture level: ");
      Serial.println(rawSoilMoisture);
      Serial.print("Soil moisture level: ");
      Serial.println(percentMoisture);
      // Set up a string with our post request
      String httpRequestData = "api_key=" + apiKeyValue + "&moisture=" + percentMoisture + "%&plantname=" + plantName + "";

      Serial.print("httpRequestData: ");
      Serial.println(httpRequestData);
    


      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
     

      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
   }
   else
   {
     Serial.println(F("Wifi disconnected..."));
   }
   bootCount = 0;
  }
  bootCount++;
  esp_deep_sleep_start();
}
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
void readSoilLevel()
{
  digitalWrite(soil_pwr, HIGH);
  delay(250);
  rawSoilMoisture = analogRead(soil_rx);
  digitalWrite(soil_pwr, LOW);

  percentMoisture = map(rawSoilMoisture, air_moisture, water_moisture, 0, 100);
}