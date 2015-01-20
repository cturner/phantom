/*
  FrSky GPS sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20141120
  Not for commercial use
*/

#ifndef _FRSKY_SPORT_SENSOR_GPS_H_
#define _FRSKY_SPORT_SENSOR_GPS_H_

#include "FrSkySportSensor.h"

#define GPS_DEFAULT_ID ID4
#define GPS_DATA_COUNT 7
#define GPS_LAT_LON_DATA_ID   0x0800
#define GPS_ALT_DATA_ID       0x0820
#define GPS_SPEED_DATA_ID     0x0830
#define GPS_COG_DATA_ID       0x0840
#define GPS_DATE_TIME_DATA_ID 0x0850


class FrSkySportSensorGps : public FrSkySportSensor
{
  public:
    FrSkySportSensorGps(SensorId id = GPS_DEFAULT_ID);
    void setData(float lat, float lon, float alt, float speed, float cog, uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    virtual void send(FrSkySportSingleWireSerial& serial, uint8_t id);

  private:
    static uint32_t setLatLon(float latLon, bool isLat);
    static uint32_t setDateTime(uint8_t yearOrHour, uint8_t monthOrMinute, uint8_t dayOrSecond, bool isDate);
    uint32_t lat;
    uint32_t lon;
    int32_t alt;
    uint32_t speed;
    uint32_t cog;
    uint32_t date;
    uint32_t time;
};

#endif // _FRSKY_SPORT_SENSOR_GPS_H_
