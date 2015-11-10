/*!
 *=============================================================================
 *
 *  @file	irobot_serial.h
 *
 *  @brief	iRobot Create controller API 
 *	
 *=============================================================================
 */
#ifndef __IROBOT_SERIAL_H__
#define __IROBOT_SERIAL_H__

#include <SerialStream.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <stdint.h>
#include <bitset>
#include <climits>

using namespace std;
using namespace LibSerial;

class iRobot
{
public:
   enum _Command {
      CMD_START 		= 128,
      CMD_BAUD			= 129,
      CMD_CONTROL		= 130,
      CMD_SAFE			= 131,
      CMD_FULL			= 132,
      CMD_SPOT 			= 134,
      CMD_COVER			= 135,
      CMD_DEMO			= 136,
      CMD_DRIVE			= 137,
      CMD_LOW_SIDE_DRIVERS	= 138,
      CMD_LEDS			= 139,
      CMD_SONG			= 140,
      CMD_PLAY			= 141,
      CMD_SENSORS		= 142,
      CMD_COVER_AND_DOCK	= 143,
      CMD_PWM_LOW_SIDE_DRIVERS	= 144,
      CMD_DRIVE_DIRECT		= 145,
      CMD_DIGITAL_OUTPUTS	= 147,
      CMD_STREAM		= 148,
      CMD_QUERY_LIST		= 149,
      CMD_PAUSE_RESUME_STREAM	= 150,
      CMD_SEND_IR		= 151,
      CMD_SCRIPT		= 152,
      CMD_PLAY_SCRIPT		= 153,
      CMD_SHOW_SCRIPT		= 154,
      CMD_WAIT_TIME		= 155,
      CMD_WAIT_DISTANCE		= 156,
      CMD_WAIT_ANGLE            = 157,
      CMD_WAIT_EVENT		= 158
   };

   enum _BaudRate {
      BR_300,
      BR_600,
      BR_1200,
      BR_2400,
      BR_4800,
      BR_9600,
      BR_14400,
      BR_19200,
      BR_28800,
      BR_38400,
      BR_57600,
      BR_115200
   };

   enum _Demo {
      DEMO_ABORT 		= -1, 
      DEMO_COVER 		=  0, 
      DEMO_COVER_AND_DOCK	=  1, 
      DEMO_SPOT_COVER 		=  2, 
      DEMO_MOUSE 		=  3, 
      DEMO_DRIVE_FIGURE_8	=  4, 
      DEMO_WIMP			=  5, 
      DEMO_HOME			=  6, 
      DEMO_TAG			=  7, 
      DEMO_PACHELBEL		=  8, 
      DEMO_BANJO		=  9  
   };

   enum _Mode {
      MODE_NO_INIT		= 0,
      MODE_PASSIVE		= 1,
      MODE_SAFE			= 2,
      MODE_FULL			= 3
   };

   enum _Speed {
      SPD_STOP			= 0,
      SPD_TRAV_LOW		= 80,
      SPD_TRAV_FULL		= 200,
      SPD_TURN_LOW		= 80,
      SPD_TURN_FULL		= 80, 
   };

   typedef enum { 
      OK = 0,
      COMM_UNKNOWN_ERROR,
      COMM_OPEN_ERROR,
      COMM_CLOSE__ERROR,
      COMM_SETBAUDRATE_ERROR,
      COMM_SETCHARSIZE_ERROR,
      COMM_SETPARITY_ERROR,
      COMM_SETFLOWCONTROL_ERROR,
      COMM_SETSTOPBITS_ERROR,
      COMM_TIMEOUT_ERROR,
      COMM_WRITE_ERROR,
      STA_ILLEGAL 
   } Status;
 
   struct _Sensors {
      unsigned char bumpsWheel;
      unsigned char wall;
      unsigned char cliffLeft;
      unsigned char cliffFrontLeft;
      unsigned char cliffFrontRight;
      unsigned char cliffRight;
      unsigned char virtualWall;
      unsigned char overCurrent;
      unsigned char unused1;
      unsigned char unused2;
      unsigned char infrared;
      unsigned char buttons;
      unsigned char distanceHi;
      unsigned char distanceLo;
      unsigned char angleHi;
      unsigned char angleLo;
      unsigned char chargeState;
      unsigned char batteryVoltageHi;
      unsigned char batteryVoltageLo;
      unsigned char batteryCurrentHi;
      unsigned char batteryCurrentLo;
      unsigned char batteryTemp;
      unsigned char batteryChargeHi;
      unsigned char batteryChargeLo;
      unsigned char batteryCapacityHi;
      unsigned char batteryCapacityLo;
   };

   enum _BumpsWheel {
      WHEELDROP_CASTER	= 0x10, 
      WHEELDROP_LEFT	= 0x08, 
      WHEELDROP_RIGHT	= 0x04, 
      BUMP_LEFT		= 0x02, 
      BUMP_RIGHT	= 0x01, 
   };
  
   enum _OverCurrent {
      WHEEL_LEFT	= 0x10,
      WHEEL_RIGHT	= 0x08,
      LD_3		= 0x04,
      LD_2		= 0x02,
      LD_1		= 0x01 
   };

   enum _Buttons {
      BUTTON_ADV	= 0x04, 
      BUTTON_PLAY	= 0x01 
   };

   enum _ChargeState {
      NOT_CHARGING	= 0,
      RECOND_CHARGING   = 1,
      FULL_CHARGING	= 2,
      TRICKLE_CHARGING	= 3,
      WAITING		= 4,
      CHARGE_FAULT	= 5
   }; 

public:
   iRobot();
   ~iRobot();
   Status connect(string port);  
   Status disconnect();  
  
   Status start(); 
   Status setBaudRate(enum _BaudRate br);
   Status setSafeMode(); 
   Status setFullMode(); 
   Status setPassiveMode(); 
   enum _Mode getMode() {return mode;}
   Status setDemo(enum _Demo demo);
   Status drive(int16_t velocity, int16_t radius);
   Status directDrive(int16_t right_velocity, int16_t left_velocity);
   Status move(int16_t trav, int16_t turn);
   void printSensors();

   Status updateSensorData();
   bool getBumpsWheel(unsigned char bits);
   bool wallDetected(); 
   bool cliffLeft(); 
   bool cliffRight(); 
   bool cliffFrontLeft(); 
   bool cliffFrontRight(); 
   bool virtualWallDetected(); 
   bool getOverCurrent(unsigned char bits);
   unsigned char getInfrared();
   bool getButtons(unsigned char bits);
   long getDistance();
   int16_t getDeltaDistance();
   long getAngle();
   int16_t getDeltaAngle();
   enum _ChargeState getChargeState();
   uint16_t getBatteryVoltage();
   int16_t getBatteryCurrent();
   char getBatteryTemp();
   uint16_t getBatteryCharge();
   uint16_t getBatteryCapacity();
 
private:
   Status rxData(char *data, int &length, int timeout);
   Status txData(char *data, int length);

private:
   SerialStream mSerialPort;
   int mCommTimeOut;
   enum _Mode mode;
   int16_t deltaAngle;
   int16_t deltaDistance;
   long angle;
   long distance;

public:
   struct _Sensors mSensor;
};


#endif  // __IROBOT_SERIAL_H__
/*! @} */
