  // Include Libraries
#include <esp_now.h>
#include <WiFi.h>

void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
}
 
void loop(){
Serial.println(WiFi.macAddress());
}