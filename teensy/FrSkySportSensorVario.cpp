/*
  FrSky Variometer (high precision) sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20141120
  Not for commercial use
*/

#include "FrSkySportSensorVario.h" 

FrSkySportSensorVario::FrSkySportSensorVario(SensorId id) : FrSkySportSensor(id) { }

void FrSkySportSensorVario::setData(float altitude, float vsi)
{
  FrSkySportSensorVario::altitude = (int32_t)(altitude * 100);
  FrSkySportSensorVario::vsi = (int32_t)(vsi * 100);
}

void FrSkySportSensorVario::send(FrSkySportSingleWireSerial& serial, uint8_t id)
{
  if(sensorId == id)
  {
    switch(sensorDataIdx)
    {
      case 0:
        serial.sendData(VARIO_ALT_DATA_ID, altitude);
        break;
      case 1:
        serial.sendData(VARIO_VSI_DATA_ID, vsi);
        break;
    }
    sensorDataIdx++;
    if(sensorDataIdx >= VARIO_DATA_COUNT) sensorDataIdx = 0;
  }
}
