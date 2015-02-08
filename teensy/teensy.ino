// required for all FrSky telemetry
#include "FrSkySportSensor.h"
#include "FrSkySportSingleWireSerial.h"
#include "FrSkySportTelemetry.h"
#include "SoftwareSerial.h"

// software FrSky telemetry sensors
#include "FrSkySportSensorGps.h"
#include "FrSkySportSensorFlvss.h"
#include "FrSkySportSensorFcs.h"
#include "FrSkySportSensorRpm.h"
#include "FrSkySportSensorVario.h"

// FlexCAN and Naza CAN decoder
#include "NazaCanDecoderLib.h"
#include "FlexCAN.h"

// for SD card logging
#include <SdFat.h>

#define SERIAL_PORT Serial
#define OSD_SERIAL Serial3
#define DEBUG false
#define LOG_WRITE_DELAY 250   // interval to write telemetry to SD card (ms)
#define LOG_SYNC_DELAY  3000  // interval to flush to disk

FrSkySportSensorGps gps;
FrSkySportSensorFlvss flvss;
FrSkySportSensorRpm rpm;
FrSkySportSensorVario vario;
FrSkySportTelemetry telemetry;

SdFat sd;
SdFile logFile;
boolean logReady = false;   // used to disable LED blinks once SD logging is running

const uint8_t chipSelect = 10;
uint32_t currTime, otherTime, gpsTime, sdTime, sdFlushTime = 0;
char dateTime[20];
uint32_t messageId;
char cell1Volts[5];
char cell2Volts[5];
char cell3Volts[5];
int led_state = LOW;
long led_interval = 1000;
long led_time = 0;

void setup() {
  pinMode(13, OUTPUT);  // Assign pin 13 (LED) to OUTPUT
  SERIAL_PORT.begin(115200);
  OSD_SERIAL.begin(115200);
  NazaCanDecoder.begin();
  telemetry.begin(FrSkySportSingleWireSerial::SERIAL_1, &gps, &flvss, &rpm);
}

//==============================================================================
// Error messages stored in flash.
#define error(msg) error_P(PSTR(msg))
//------------------------------------------------------------------------------
void error_P(const char* msg) {
  sd.errorHalt_P(msg);
}

void initLogging() {
  if (logReady) return;
  
  SERIAL_PORT.println("calling sd.begin()"); 
  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();
   char fileName[13];
   uint8_t logNum = 0;
   sprintf(fileName, "%02u%2u%2u%02u.log", NazaCanDecoder.getMonth(), NazaCanDecoder.getDay(),
                                           NazaCanDecoder.getYear(), logNum);

//  sprintf(fileName, "%02u%2u%2u%02u.log", 1, 18,
//                                             15, logNum);
                                             
   while (sd.exists(fileName)) {
     SERIAL_PORT.print("checking "); SERIAL_PORT.println(fileName);
     sprintf(fileName, "%02u%2u%2u%02u.log", NazaCanDecoder.getMonth(), NazaCanDecoder.getDay(),
                                             NazaCanDecoder.getYear(), logNum);
     logNum++;
//     sprintf(fileName, "%02u%2u%2u%02u.log", 1, 18,
//                                             15, logNum);
     // can't exceed 99 files per day, also prevent infinite loop in case something bad happens
     if (logNum > 99) {
       // reduce LED blink interval so we can tell something bad has happened
       led_interval = 500;
       return;
     }
   }
   SERIAL_PORT.print("opening "); SERIAL_PORT.println(fileName);
   if (!logFile.open(fileName, O_CREAT | O_WRITE | O_EXCL)) {
       led_interval = 1500; // blink LED faster so we know something bad happened
       return;
   }
   logFile.println("TIME, MILLIS, LATITUDE, LONGITUDE, ALTITUDE, HEADING, SPEED (kts), NUM_SATS, BATTERY_PERCENT");
   logReady = true;
}

// Send NMEA message with prefix, suffix and checksum
void sendNmea(char* msg) {
  unsigned char chcksm = 0;
  OSD_SERIAL.print("$");
  while(*msg) {
    OSD_SERIAL.write(*msg);
    chcksm = chcksm ^ *msg++;
  }
  OSD_SERIAL.print("*"); 
  if(chcksm < 16) OSD_SERIAL.write('0'); 
  OSD_SERIAL.print(chcksm, HEX);
  OSD_SERIAL.print("\r\n");
}

void sendOSDTelemetry() {
  // - split latitude into degrees, minutes integer and minutes fractional part (4 digit precision)
  int8_t   latDeg    = (int8_t)NazaCanDecoder.getLat();            
  double   latMin    = abs(NazaCanDecoder.getLat() - latDeg) * 60;
  uint8_t  latMinInt = (uint8_t)latMin;
  uint16_t latMinFra = (uint16_t)((latMin - latMinInt) * 10000);

  // - split longitude into degrees, minutes integer and minutes fractional part (4 digit precision)
  int16_t  lonDeg    = (int16_t)NazaCanDecoder.getLon();
  double   lonMin    = abs(NazaCanDecoder.getLon() - lonDeg) * 60;
  uint8_t  lonMinInt = (uint8_t)lonMin;
  uint16_t lonMinFra = (uint16_t)((lonMin - lonMinInt) * 10000);

  double      alt = NazaCanDecoder.getAlt();
  int32_t  altInt = (int32_t)alt;
  uint8_t  altFra = (uint8_t)(abs(alt - altInt) * 100);

  // - split speed into integer and fractional part (2 digit precision)
  double      spd = NazaCanDecoder.getSpeed() * 1.943884; // convert from m/s to knots
  uint32_t spdInt = (uint32_t)spd;
  uint8_t  spdFra = (uint8_t)((spd - spdInt) * 100);

  double     heading = NazaCanDecoder.getHeading();
  int16_t    hdopInt = (int16_t)NazaCanDecoder.getHdop();
  uint8_t    hdopFra = (uint8_t)(abs(NazaCanDecoder.getHdop() - hdopInt) * 100);
  int32_t headingInt = (int32_t)heading;
  uint8_t headingFra = (uint8_t)(abs(heading - headingInt) * 100);

  NazaCanDecoderLib::fixType_t fix = NazaCanDecoder.getFixType();

  char nmeaGGA[128];
  sprintf(nmeaGGA, "GPGGA,%02u%02u%02u.000,%02d%02u.%04u,%c,%03d%02u.%04u,%c,%u,%02u,%d.%02u,%ld.%02u,M,,M,,", 
          NazaCanDecoder.getHour(), NazaCanDecoder.getMinute(), NazaCanDecoder.getSecond(),
          abs(latDeg), latMinInt, latMinFra, latDeg < 0 ? 'S' : 'N',
          abs(lonDeg), lonMinInt, lonMinFra, lonDeg < 0 ? 'W' : 'E',
          (fix == NazaCanDecoderLib::NO_FIX) ? 0 : ((fix == NazaCanDecoderLib::FIX_DGPS) ? 2 : 1),
          NazaCanDecoder.getNumSat(),
          hdopInt, hdopFra,
          altInt, altFra);
  sendNmea(nmeaGGA);

  char nmeaRMC[128];
  sprintf(nmeaRMC, "GPRMC,%02u%02u%02u.000,%c,%02d%02u.%04u,%c,%03d%02u.%04u,%c,%ld.%02u,%ld.%02u,%02u%02u%02u,,", 
          NazaCanDecoder.getHour(), NazaCanDecoder.getMinute(), NazaCanDecoder.getSecond(),
          (fix == NazaCanDecoderLib::NO_FIX) ? 'V' : 'A',
          abs(latDeg), latMinInt, latMinFra, latDeg < 0 ? 'S' : 'N',
          abs(lonDeg), lonMinInt, lonMinFra, lonDeg < 0 ? 'W' : 'E',
          spdInt, spdFra,
          headingInt, headingFra,
          NazaCanDecoder.getDay(), NazaCanDecoder.getMonth(), NazaCanDecoder.getYear());
  sendNmea(nmeaRMC);
}

void loop() {
  messageId = NazaCanDecoder.decode();  
  currTime = millis();
  
  // Blink the on-board LED once per minute so we know we have power
  // if (!logReady) {
    if (!logReady && currTime > led_time) {
      led_time = currTime + led_interval;
      if (led_state == LOW) {
        led_state = HIGH;
      } else {
        led_state = LOW;
      }
      digitalWrite(13, led_state);
    }
  // }
 
  if (DEBUG == true && (otherTime < currTime)) {
    otherTime = currTime + 500;
    SERIAL_PORT.print("Lat: "); SERIAL_PORT.print(NazaCanDecoder.getLat(),7 );
    SERIAL_PORT.print(", Lon: "); SERIAL_PORT.print(NazaCanDecoder.getLon(), 7);
    SERIAL_PORT.print(", GPS alt: "); SERIAL_PORT.print(NazaCanDecoder.getGpsAlt());
    SERIAL_PORT.print(", COG: "); SERIAL_PORT.print(NazaCanDecoder.getCog());
    SERIAL_PORT.print(", Speed: "); SERIAL_PORT.print(NazaCanDecoder.getSpeed());
    
    sprintf(dateTime, "%4u.%02u.%02u %02u:%02u:%02u", 
          NazaCanDecoder.getYear() + 2000, NazaCanDecoder.getMonth(), NazaCanDecoder.getDay(),
          NazaCanDecoder.getHour(), NazaCanDecoder.getMinute(), NazaCanDecoder.getSecond());
    SERIAL_PORT.print(", Date/Time: "); SERIAL_PORT.print(dateTime);
    SERIAL_PORT.print(", Bat: "); SERIAL_PORT.print(NazaCanDecoder.getBatteryPercent()); SERIAL_PORT.print("%");
    SERIAL_PORT.print(", Bat Capacity: "); SERIAL_PORT.print(NazaCanDecoder.getBatteryCurrentCapacity());
    SERIAL_PORT.print(", Sats: "); SERIAL_PORT.println(NazaCanDecoder.getNumSat());
    
    sprintf(cell1Volts, "%.2f", (double)NazaCanDecoder.getBatteryCell(NazaCanDecoderLib::CELL_1) / 1000);
    sprintf(cell2Volts, "%.2f", (double)NazaCanDecoder.getBatteryCell(NazaCanDecoderLib::CELL_2) / 1000);
    sprintf(cell3Volts, "%.2f", (double)NazaCanDecoder.getBatteryCell(NazaCanDecoderLib::CELL_3) / 1000);

    // SERIAL_PORT.print("Cell 1 voltage: "); SERIAL_PORT.print(cell1Volts);   
    // SERIAL_PORT.print(", Cell 2 voltage: "); SERIAL_PORT.print(cell2Volts);
    // SERIAL_PORT.print(", Cell 3 voltage: "); SERIAL_PORT.println(cell3Volts);
  }

  if(currTime > gpsTime) {
    // don't start logging telemetry until we have at least 5 satellites;  it's useless before that anyway
    if (NazaCanDecoder.getNumSat() > 4) initLogging();
    
    sendOSDTelemetry();
    gpsTime = currTime + 200; // 5Hz = every 200 milliseconds
  }  

  if (logReady && currTime > sdTime) {
    char buf[1024];
    sprintf(dateTime, "%4u.%02u.%02u %02u:%02u:%02u", 
            NazaCanDecoder.getYear() + 2000, NazaCanDecoder.getMonth(), NazaCanDecoder.getDay(),
            NazaCanDecoder.getHour(), NazaCanDecoder.getMinute(), NazaCanDecoder.getSecond());
    sprintf(buf, "%s,%d,%.5f,%.5f,%d,%d,%d,%d,%d", dateTime, currTime, NazaCanDecoder.getLat(), NazaCanDecoder.getLon(), 
                                        (int32_t)NazaCanDecoder.getGpsAlt(),  (int32_t)NazaCanDecoder.getHeading(),
                                        (uint32_t)NazaCanDecoder.getSpeed(), NazaCanDecoder.getNumSat(), NazaCanDecoder.getBatteryPercent());
    logFile.println(buf);
    if (currTime> sdFlushTime) {
      logFile.sync();
      sdFlushTime = currTime + LOG_SYNC_DELAY;
    }
    sdTime = currTime + LOG_WRITE_DELAY;
  }
 
  flvss.setData((float)NazaCanDecoder.getBatteryCell(NazaCanDecoderLib::CELL_1) / 1000,
                (float)NazaCanDecoder.getBatteryCell(NazaCanDecoderLib::CELL_2) / 1000,
                (float)NazaCanDecoder.getBatteryCell(NazaCanDecoderLib::CELL_3) / 1000);
                
  gps.setData(NazaCanDecoder.getLat(), NazaCanDecoder.getLon(),
              NazaCanDecoder.getGpsAlt(), NazaCanDecoder.getSpeed(),
              NazaCanDecoder.getHeading(),
              NazaCanDecoder.getYear(), NazaCanDecoder.getMonth(), NazaCanDecoder.getDay(),
              NazaCanDecoder.getHour(), NazaCanDecoder.getMinute(), NazaCanDecoder.getSecond());
  
  // corrections required for default TARANIS display
  // rpm.setData(NazaCanDecoder.getBatteryCharge(), ((NazaCanDecoder.getNumSat() - 35) / 2), ((NazaCanDecoder.getFixType() - 35) / 2));
  
  rpm.setData(NazaCanDecoder.getBatteryPercent(), NazaCanDecoder.getNumSat(), NazaCanDecoder.getFixType());
  vario.setData(NazaCanDecoder.getAlt(), NazaCanDecoder.getVsi());
  
  telemetry.send();
  NazaCanDecoder.heartbeat();
}

