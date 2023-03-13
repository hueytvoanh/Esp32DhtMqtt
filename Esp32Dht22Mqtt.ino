/*
 Publishing in the callback

  - connects to an MQTT server
  - subscribes to the topic "inTopic"
  - when a message is received, republishes it to "outTopic"

  This example shows how to publish messages within the
  callback function. The callback function header needs to
  be declared before the PubSubClient constructor and the
  actual callback defined afterwards.
  This ensures the client reference in the callback function
  is valid.

*/
#include <WiFi.h>
#include <SPI.h>
#include <PubSubClient.h>


#include "DHT.h"

#define DHTPIN 21     // Digital pin connected to the DHT sensor

//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);



TaskHandle_t checkTempHumTask;
TaskHandle_t mqttKeepAliveTask;
TaskHandle_t mqttUploadTempHumTask;


const char* ssid = "Ngoc My";
const char* password = "0857270632";
// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(172, 16, 0, 100);
IPAddress server(192, 168, 1, 7);

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

WiFiClient WifiClient;
PubSubClient client(server, 1883, callback, WifiClient);

// Callback function
void callback(char* topic, byte* payload, unsigned int length) {
  // In order to republish this payload, a copy must be made
  // as the orignal payload buffer will be overwritten whilst
  // constructing the PUBLISH packet.
  Serial.print("Call Back ");
  // Allocate the correct amount of memory for the payload copy
  byte* p = (byte*)malloc(length);
  // Copy the payload to the new buffer
  memcpy(p,payload,length);
  //client.publish("outTopic", p, length);
  client.publish("sensors", p, length);
  // Free the memory
  free(p);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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


void setup()
{
  Serial.begin(115200);
  setup_wifi();
  //Ethernet.begin(mac, ip);
  if (client.connect("arduinoClient")) {
    Serial.print("Connected to broker ");
    //client.publish("outTopic","hello world");
    //client.subscribe("inTopic");
    client.publish("sensors","hello world");
    client.subscribe("sensors");
  }
  else{
    Serial.print("Can not connected to broker ");
  }

  dht.begin();
  delay(2000);

  xTaskCreatePinnedToCore(
     checkTempHumTaskFunction,
     "checkTempHumTaskFunction",
     10000,
     NULL,
     1,
     &checkTempHumTask,
     1);

  xTaskCreatePinnedToCore(
     mqttKeepAliveTaskFunction,
     "mqttKeepAliveTaskFunction",
     10000,
     NULL,
     1,
     &mqttKeepAliveTask,
     1);

  xTaskCreatePinnedToCore(
     mqttUploadTempHumTaskFunction,
     "mqttUploadTempHumTaskFunction",
     10000,
     NULL,
     1,
     &mqttUploadTempHumTask,
     1);     
 
}


void mqttUploadTempHumTaskFunction( void * pvParameters ){
  
  while(1){
      if (client.connect("arduinoClient")) {
          Serial.print("Connected to broker ");
          //client.publish("outTopic","hello world");
          //client.subscribe("inTopic");
          client.publish("sensors","hello world");
          client.subscribe("sensors");
      }
      else{
          Serial.print("Can not connected to broker ");
     }

     delay(30000);
  }
}


void mqttKeepAliveTaskFunction( void * pvParameters ){
  
  while(1){
      client.loop();
      delay(2000);
  }
}



void checkTempHumTaskFunction( void * pvParameters ){
  
  while(1){
      delay(5000);
      float h = dht.readHumidity(); 
      float t = dht.readTemperature();  
      float f = dht.readTemperature(true);
   
      if (isnan(h) || isnan(t) || isnan(f)) {
          Serial.println(F("Failed to read from DHT sensor!"));
          //return;
      }

      float hif = dht.computeHeatIndex(f, h);
      float hic = dht.computeHeatIndex(t, h, false);

      Serial.print(F("Humidity: "));
      Serial.print(h);
      Serial.print(F("%  Temperature: "));
      Serial.print(t);
      Serial.print(F("°C "));
      Serial.print(f);
      Serial.print(F("°F  Heat index: "));
      Serial.print(hic);
      Serial.print(F("°C "));
      Serial.print(hif);
      Serial.println(F("°F"));
  }
}



void loop()
{
  //client.loop();


  //delay(2000);
/*
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
  */
}
