/*
  FrSky FLVSS LiPo voltage sensor class for Teensy 3.x and 328P based boards (e.g. Pro Mini, Nano, Uno)
  (c) Pawelsky 20141120
  Not for commercial use
*/

#ifndef _FRSKY_SPORT_SENSOR_FLVSS_H_
#define _FRSKY_SPORT_SENSOR_FLVSS_H_

#include "FrSkySportSensor.h"

#define FLVSS_DEFAULT_ID ID2
#define FLVSS_DATA_COUNT 3
#define FLVSS_CELL_DATA_ID 0x0300

class FrSkySportSensorFlvss : public FrSkySportSensor
{
  public:
    FrSkySportSensorFlvss(SensorId id = FLVSS_DEFAULT_ID);
    void setData(float cell1, float cell2 = 0.0, float cell3 = 0.0, float cell4 = 0.0, float cell5 = 0.0, float cell6 = 0.0);
    virtual void send(FrSkySportSingleWireSerial& serial, uint8_t id);

  private:
    static uint32_t setCellData(uint8_t cellNum, uint8_t firstCellNo, float cell1, float cell2);
    uint32_t cellData1;
    uint32_t cellData2;
    uint32_t cellData3;
};

#endif // _FRSKY_SPORT_SENSOR_FLVSS_H_
