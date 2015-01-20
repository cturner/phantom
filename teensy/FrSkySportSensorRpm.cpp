/*
  FrSky RPM sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20141120
  Not for commercial use
*/

#include "FrSkySportSensorRpm.h" 

FrSkySportSensorRpm::FrSkySportSensorRpm(SensorId id) : FrSkySportSensor(id) { }

void FrSkySportSensorRpm::setData(float rpm, float t1, float t2)
{
  FrSkySportSensorRpm::rpm = (uint32_t)(rpm * 2);
  FrSkySportSensorRpm::t1 = (int32_t)t1;
  FrSkySportSensorRpm::t2 = (int32_t)t2;
}

void FrSkySportSensorRpm::send(FrSkySportSingleWireSerial& serial, uint8_t id)
{
  if(sensorId == id)
  {
    switch(sensorDataIdx)
    {
      case 0:
        serial.sendData(RPM_ROT_DATA_ID, rpm);
        break;
      case 1:
        serial.sendData(RPM_T1_DATA_ID, t1);
        break;
      case 2:
        serial.sendData(RPM_T2_DATA_ID, t2);
        break;
    }
    sensorDataIdx++;
    if(sensorDataIdx >= RPM_DATA_COUNT) sensorDataIdx = 0;
  }
}

