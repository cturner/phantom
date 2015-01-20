/*
  FrSky GPS sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20141129
  Not for commercial use
*/

#include "FrSkySportSensorGps.h" 

FrSkySportSensorGps::FrSkySportSensorGps(SensorId id) : FrSkySportSensor(id) { }

void FrSkySportSensorGps::setData(float lat, float lon, float alt, float speed, float cog, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
  FrSkySportSensorGps::lat = setLatLon(lat, true);
  FrSkySportSensorGps::lon = setLatLon(lon, false);
  FrSkySportSensorGps::cog = cog * 100;
  FrSkySportSensorGps::speed = speed * 1944; // Convert m/s to knots
  FrSkySportSensorGps::alt = alt * 100;
  FrSkySportSensorGps::date = setDateTime(year, month, day, true);
  FrSkySportSensorGps::time = setDateTime(hour, minute, second, false);
}

uint32_t FrSkySportSensorGps::setLatLon(float latLon, bool isLat)
{
  uint32_t data = (uint32_t)((latLon < 0 ? -latLon : latLon) * 60 * 10000) & 0x3FFFFFFF;
  if(isLat == false) data |= 0x80000000;
  if(latLon < 0) data |= 0x40000000;

  return data;
}

uint32_t FrSkySportSensorGps::setDateTime(uint8_t yearOrHour, uint8_t monthOrMinute, uint8_t dayOrSecond, bool isDate)
{
  uint32_t data = yearOrHour;
  data <<= 8;
  data |= monthOrMinute;
  data <<= 8;
  data |= dayOrSecond;
  data <<= 8;
  if(isDate == true) data |= 0xFF;

  return data;
}

void FrSkySportSensorGps::send(FrSkySportSingleWireSerial& serial, uint8_t id)
{
  if(sensorId == id)
  {
    switch(sensorDataIdx)
    {
      case 0:
        serial.sendData(GPS_LAT_LON_DATA_ID, lat);
        break;
      case 1:
        serial.sendData(GPS_LAT_LON_DATA_ID, lon);
        break;
      case 2:
        serial.sendData(GPS_ALT_DATA_ID, alt);
        break;
      case 3:
        serial.sendData(GPS_SPEED_DATA_ID, speed);
        break;
      case 4:
        serial.sendData(GPS_COG_DATA_ID, cog);
        break;
      case 5:
        serial.sendData(GPS_DATE_TIME_DATA_ID, date);
        break;
      case 6:
        serial.sendData(GPS_DATE_TIME_DATA_ID, time);
        break;
    }
    sensorDataIdx++;
    if(sensorDataIdx >= GPS_DATA_COUNT) sensorDataIdx = 0;
  }
}
