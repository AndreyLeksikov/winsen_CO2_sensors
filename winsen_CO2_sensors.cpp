/*************************************************** 
  This is a library for the Winsen MH-Z* sensors.
  These sensors use Serial interface to communicate.
  
  Written by Andrey Leksikov.  

  Licensed under MIT license
 ****************************************************/

#include "winsen_CO2_sensors.h"
 
MHZSensor::MHZSensor(Stream &serial) {
  _serial = &serial;
}


// Calculate packet's checksum 
uint8_t MHZSensor::getCheckSum(uint8_t *pct) {
  uint8_t i, checksum;
  
  for (i = 1; i < 8; i++) {
      checksum += pct[i];
    }
    
  checksum = ~checksum + 1;
  return checksum;
}


void  MHZSensor::printPacket(uint8_t *pct) {
        char buff[44];
        sprintf(buff, "Packet: %02X %02X %02X %02X %02X %02X %02X %02X %02X", pct[0], pct[1], pct[2], pct[3], pct[4], pct[5], pct[6], pct[7], pct[8]);
        Serial.println(buff);
}


// Clear the incoming Serial buffer
void MHZSensor::flushRxBuffer(void) {
  while(_serial->available())
    _serial->read();
}


void  MHZSensor::preparePacket(uint8_t command, uint8_t *data) {
  _packet[0] = MHZ_START_BYTE;
  _packet[1] = MHZ_SENSOR_NUMBER;
  _packet[2] = command;
  _packet[3] = data[0];
  _packet[4] = data[1];
  _packet[5] = data[2];
  _packet[6] = data[3];
  _packet[7] = data[4];
  _packet[8] = getCheckSum(_packet);
}

// Validate checksum, command and start byte
bool MHZSensor::checkResponse(uint8_t *pct) {
  
  #if (MHZ_DEBUG == 1)
    printPacket(pct);
  #endif  

  if ((pct[0] != MHZ_START_BYTE) || (pct[1] != MHZ_READ_CO2)) {
    #if (MHZ_DEBUG == 1)
      Serial.println ("Unrecognized response from the CO2 sensor!");
    #endif
    return false;
  }
  
  if (pct[8] != getCheckSum(pct)) {
    #if (MHZ_DEBUG == 1)
      Serial.println ("Checksum error!");
    #endif
    return false;
  }
  
  return true;
}

// Read CO2 concentration
int32_t MHZSensor::readCO2(void) {
  uint8_t data[] = {0, 0, 0, 0, 0};
  uint8_t response[MHZ_PACKET_LENGTH];
  uint8_t rTime = 0;

  preparePacket(MHZ_READ_CO2, data);
  
  #if (MHZ_DEBUG == 1)
    printPacket(_packet);
  #endif
  
  // Send coomand
  flushRxBuffer();
  _serial->write(_packet, MHZ_PACKET_LENGTH);
  _serial->flush();
  
  
  // Waiting for response from the sensor
  while((!_serial->available()) && (rTime < MHZ_RESPONSE_TIMEOUT)) {
    delay(5);
    rTime += 5;
  }
  
  // Check for timeout and get response
  if (_serial->available()) {
    _serial->readBytes(response, MHZ_PACKET_LENGTH);
  } else {
    #if (MHZ_DEBUG == 1)
      Serial.println ("No response from the CO2 sensor! Check connection.");
    #endif
    return -1;
  }  

  if (!checkResponse(response)) {
	return -1;  
  }

  if (response[5] != 0x40) {
    #if (MHZ_DEBUG == 1)
      Serial.println ("Warning! Bad Status Byte...");
    #endif
    // return -1;
  }

  // calculate CO2 value from HIGHBYTE and LOWBYTE
  uint16_t ppm = (response[2] << 8) + response[3];
  return ppm; 
}

// Return full response
MHZResponse* MHZSensor::getResponse(void) {
  uint8_t data[] = {0, 0, 0, 0, 0};
  uint8_t response[MHZ_PACKET_LENGTH];
  uint8_t rTime = 0;
  MHZResponse* mhzResponse = new MHZResponse;

  preparePacket(MHZ_READ_CO2, data);
  
  #if (MHZ_DEBUG == 1)
    printPacket(_packet);
  #endif
  
  // Send coomand
  flushRxBuffer();
  _serial->write(_packet, MHZ_PACKET_LENGTH);
  _serial->flush();
  
  
  // Waiting for response from the sensor
  while((!_serial->available()) && (rTime < MHZ_RESPONSE_TIMEOUT)) {
    delay(5);
    rTime += 5;
  }
  
  // Check for timeout and get response
  if (_serial->available()) {
    _serial->readBytes(response, MHZ_PACKET_LENGTH);
  } else {
    #if (MHZ_DEBUG == 1)
      Serial.println ("No response from the CO2 sensor! Check connection.");
    #endif
    return NULL;
  }  

  if (!checkResponse(response)) {
	return NULL;  
  }

  // calculate CO2 value from HIGHBYTE and LOWBYTE
  mhzResponse->ppm = (response[2] << 8) + response[3];
  // calculate temerature
  mhzResponse->temp = response[4] - 40;
  // status byte
  mhzResponse->status = response[5];
  
  return mhzResponse; 
}


/* Automatic Baseline Correction (ABC logic function)
 * ABC logic function refers to that sensor itself do zero 
 * point judgment and automatic calibration procedure
 * intelligently after a continuous operation period.
 * The automatic calibration cycle is every 24 hours
 * after powered on. The zero point of automatic
 * calibration is 400ppm. From July 2015, the default 
 * setting is with built-in automatic calibration function 
 * if no special request. 
 * This function is usually suitable for indoor air
 * quality monitor such as offices, schools and homes, 
 * not suitable for greenhouse, farm and refrigeratory 
 * where this function should be off.
 */

void MHZSensor::enableABCLogic(bool enable = true) {
  uint8_t data[] = {0, 0, 0, 0, 0};
  
  if (enable) {
    data[0] = MHZ_ENABLE_ABC_FLAG;  
  }
  
  preparePacket(MHZ_CONFIG_ABC_LOGIC, data);
  flushRxBuffer();
  _serial->write(_packet, MHZ_PACKET_LENGTH);
}

