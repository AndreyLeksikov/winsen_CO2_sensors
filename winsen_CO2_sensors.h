/*************************************************** 
  This is a library for the Winsen MH-Z* sensors.
  These sensors use Serial interface to communicate.
  
  Written by Andrey Leksikov.  

  Licensed under MIT license
 ****************************************************/

#ifndef WINSEN_CO2_SENSOR_H
#define WINSEN_CO2_SENSOR_H

// Enable/disable debugging messages to Serial port
#define MHZ_DEBUG 0

#include "Arduino.h"

/*************************************************** 
 MH-Z* sensors commands.
 Please see datasheet,
 https://revspace.nl/MHZ19
 and 
 https://geektimes.ru/post/285572/
****************************************************/

#define MHZ_READ_CO2                0x86
#define MHZ_ZERO_POINT_CALIBRATION  0x87
#define MHZ_SPAN_POINT_CALIBRATION  0x88
#define MHZ_CONFIG_ABC_LOGIC        0x79
#define MHZ_CONFIG_DETECTION_RANGE  0x99

// MH-Z* sensors constants.

#define MHZ_ENABLE_ABC_FLAG         0xA0
#define MHZ_START_BYTE              0xFF
#define MHZ_SENSOR_NUMBER           0x01
#define MHZ_PACKET_LENGTH           9
#define MHZ_RESPONSE_TIMEOUT        500

struct MHZResponse {
	uint16_t   ppm;
	uint8_t    temp;
	uint8_t    status;
};

class MHZSensor {
  public:
    MHZSensor(Stream &serial);
    int32_t readCO2(void);
	MHZResponse* getResponse(void);
	
  private:
    Stream* _serial;
    uint8_t _packet[MHZ_PACKET_LENGTH];

    void flushRxBuffer(void);
    void preparePacket(uint8_t command, uint8_t *data);
    uint8_t getCheckSum(uint8_t *pct);
	bool checkResponse(uint8_t *pct);
    void printPacket(uint8_t *pct);
    void enableABCLogic(bool flag);
  
};

#endif // WINSEN_CO2_SENSOR_H
