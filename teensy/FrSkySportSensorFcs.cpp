/*
  FrSky FCS-40A/FCS-150A current sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20141120
  Not for commercial use
*/

#include "FrSkySportSensorFcs.h" 

FrSkySportSensorFcs::FrSkySportSensorFcs(SensorId id) : FrSkySportSensor(id) { }

void FrSkySportSensorFcs::setData(float current, float voltage)
{
  FrSkySportSensorFcs::current = (uint32_t)(current * 10);
  FrSkySportSensorFcs::voltage = (uint32_t)(voltage * 100);
}

void FrSkySportSensorFcs::send(FrSkySportSingleWireSerial& serial, uint8_t id)
{
  if(sensorId == id)
  {
    switch(sensorDataIdx)
    {
      case 0:
        serial.sendData(FCS_CURR_DATA_ID, current);
        break;
      case 1:
        serial.sendData(FCS_VOLT_DATA_ID, voltage);
        break;
    }
    sensorDataIdx++;
    if(sensorDataIdx >= FCS_DATA_COUNT) sensorDataIdx = 0;
  }
}
