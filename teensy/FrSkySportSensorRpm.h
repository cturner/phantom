/*
  FrSky RPM sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20141120
  Not for commercial use
*/

#ifndef _FRSKY_SPORT_SENSOR_RPM_H_
#define _FRSKY_SPORT_SENSOR_RPM_H_

#include "FrSkySportSensor.h"

#define RPM_DEFAULT_ID ID5
#define RPM_DATA_COUNT 3
#define RPM_T1_DATA_ID 0x0400
#define RPM_T2_DATA_ID 0x0410
#define RPM_ROT_DATA_ID 0x0500

class FrSkySportSensorRpm : public FrSkySportSensor
{
  public:
    FrSkySportSensorRpm(SensorId id = RPM_DEFAULT_ID);
    void setData(float rpm, float t1 = 0.0, float t2 = 0.0);
    virtual void send(FrSkySportSingleWireSerial& serial, uint8_t id);

  private:
    uint32_t rpm;
    int32_t t1;
    int32_t t2;
};

#endif // _FRSKY_SPORT_SENSOR_RPM_H_
