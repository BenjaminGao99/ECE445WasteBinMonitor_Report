// Include Libraries
#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h>

 
// Variables for test data
int int_value;
float float_value;
bool bool_value = true;

const int trigPin = 5;
const int echoPin = 6;
const int MQ135Pin = 7;


#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

#define DHT_SENSOR_PIN  15 // ESP32 pin GPIO21 connected to DHT22 sensor
#define DHT_SENSOR_TYPE DHT22

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

long duration;
float distanceCm;
float distanceInch;
 
// MAC Address of responder - edit as required
uint8_t broadcastAddress[] = {0xF4, 0x12, 0xFA, 0xED, 0x4A, 0x1C}; //PCB
// uint8_t broadcastAddress[] = {0xFF, 0x8A, 0xFC, 0xAA, 0xEA, 0x40}; //Dev Board
 
// Define a data structure
typedef struct data_packet {
  int canId;
  int percentFull;
  float temperature;
  float humidity;
  int CO2Concentration;
} data_packet;
 
// Create a structured object
data_packet myData;
 
// Peer info
esp_now_peer_info_t peerInfo;
 
// Callback function called when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  // Set up Serial Monitor
  Serial.begin(115200);
 
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
 
  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
 
  // Register the send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  dht_sensor.begin(); // initialize the DHT sensor
}
 
void loop() {
 
  duration = 0;
  for (int i = 0; i < 5; i++){
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
    duration += pulseIn(echoPin, HIGH);
    delay(10);
  }
  duration = duration/5; //Gets the average of 5 read in values
  // read humidity
  float humi  = dht_sensor.readHumidity();
  // read temperature in Celsius
  float tempC = dht_sensor.readTemperature();
  // read temperature in Fahrenheit
  float tempF = dht_sensor.readTemperature(true);

  int MQ135_data = analogRead(MQ135Pin);
  // check whether the reading is successful or not
  if ( isnan(tempC) || isnan(tempF) || isnan(humi)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print("%");

    Serial.print("  |  ");

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.print("°C  ~  ");
    Serial.print(tempF);
    Serial.println("°F");

    Serial.println(MQ135_data);
  }


  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
 
  // Format structured data
  myData.canId = 1;     //Can 1
  myData.percentFull = distanceInch;
  myData.temperature = tempF;
  myData.humidity = humi;
  myData.CO2Concentration = MQ135_data;
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sending confirmed");
  }
  else {
    Serial.println("Sending error");
  }
  delay(1500);
}