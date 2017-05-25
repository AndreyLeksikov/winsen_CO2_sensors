#include <winsen_CO2_sensors.h>
#include <SoftwareSerial.h>

// RX/TX pins for MH-Z19 sensor.
#define MHZ_RX 10
#define MHZ_TX 11 

SoftwareSerial co2Srl(MHZ_RX, MHZ_TX);
MHZSensor co2Sensor(co2Srl);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  co2Srl.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  MHZResponse* response = co2Sensor.getResponse();
  // Check response
  if (response == NULL) {
    Serial.println("No response or bad response from the CO2 sensor!");    
  } else {
      // Check status byte
      if (response->status != 0x40) {
        Serial.println ("Warning! Bad Status Byte...");
      }
    // Printing results
    Serial.println("Current CO2 ppm value: " + String(response->ppm));
    Serial.println("Current temperature: " + String(response->temp) + (char) 0xB0);
    // Release allocated memory
    delete response;    
  }
  delay(30000);
}
