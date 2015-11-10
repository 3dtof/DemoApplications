/*!
 *=============================================================================
 *
 *  @file	irobot_serial.cpp
 *
 *  @brief	iRobot controller API
 *	
 *=============================================================================
 */
#define __IROBOT_SERIAL_CPP__

#include "irobot_serial.h"


/*!
 *=============================================================================
 *  
 *  @function		iRobot::iRobot()
 *  
 *  @brief	 	Constructor
 *
 *=============================================================================
 */
iRobot::iRobot()
{
   mCommTimeOut = 1000;
   mode = iRobot::MODE_NO_INIT;
   angle = distance = deltaAngle = deltaDistance = 0;
}

/*!
 *=============================================================================
 *  
 *  @function		iRobot::~iRobot()
 *  
 *  @brief	        Destructure	
 *
 *=============================================================================
 */
iRobot::~iRobot()
{
}


/*!
 *=============================================================================
 *  
 *  @function		iRobot::Status iRobot::connect(string port)
 *  
 *  @brief	 	Connect to iRobot via a serial port	
 *
 *  @return		error status
 * 
 *=============================================================================
 */
iRobot::Status iRobot::connect(string port)
{
   system("stty -F /dev/ttyUSB0 cs8 57600 ignbrk -brkint -icrnl -imaxbel \
            -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl \
            -echoke noflsh -ixon -crtscts");

   mSerialPort.Open(port) ;
   if ( !mSerialPort.IsOpen() ) 
      return iRobot::COMM_OPEN_ERROR;
   mSerialPort.SetBaudRate( SerialStreamBuf::BAUD_57600 ) ;
   if ( !mSerialPort.good() ) 
      return iRobot::COMM_SETBAUDRATE_ERROR;
   mSerialPort.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 ) ;
   if ( ! mSerialPort.good() )
      return iRobot::COMM_SETCHARSIZE_ERROR;
   mSerialPort.SetParity( SerialStreamBuf::PARITY_NONE ) ;
   if ( ! mSerialPort.good() )
      return iRobot::COMM_SETPARITY_ERROR;
   mSerialPort.SetNumOfStopBits( 1 ) ;
   if ( ! mSerialPort.good() ) 
      return iRobot::COMM_SETSTOPBITS_ERROR;
   mSerialPort.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_NONE ) ;
   if ( ! mSerialPort.good() ) 
      return iRobot::COMM_SETFLOWCONTROL_ERROR;

   return iRobot::OK;
}


/*!
 *=============================================================================
 *  
 *  @function		iRobot::Status iRobot::disconnect()
 *
 *  @brief	 	Disconnect from iRobot	
 * 
 *=============================================================================
 */
iRobot::Status iRobot::disconnect()
{
   //SendCommand(iRobot::PASSIVE) ;
   mSerialPort.Close();
}


/*!
 *=============================================================================
 *  
 *  @function		iRobot::Status iRobot::rxData(char *data, int &length) 
 *
 *  @brief	 	Send a byte array of data	
 * 
 *  @return		error status
 *
 *=============================================================================
 */
iRobot::Status iRobot::rxData(char *data, int &length, int timeout)
{
   int to = timeout;
   int count = 0;
   char *p = data;

   while (count < length && timeout--) {
      if (mSerialPort.rdbuf()->in_avail() > 0) {
         mSerialPort.get(*p++);
         timeout = to;
         count++;
      } 
      else {
         usleep(100);
      }
   }

   if (timeout <= 0) {
	return iRobot::COMM_TIMEOUT_ERROR;
   }
   else {
	length = count;
   	return iRobot::OK;
   }
}


/*!
 *=============================================================================
 *  
 *  @function		iRobot::Status iRobot::txData(char *data, int length)
 *  
 *  @brief	 	Send a byte array of data	
 *
 *  @return		error status
 * 
 *=============================================================================
 */
iRobot::Status iRobot::txData(char *data, int length)
{
     mSerialPort.write(data, length);  
     if ( !mSerialPort.good() ) 
	return iRobot::COMM_WRITE_ERROR;
     else
        return iRobot::OK;
}


iRobot::Status iRobot::start()
{
   char cmd[50];
   int len = 0;
   cmd[len++] = iRobot::CMD_START;
   return txData(cmd, len);
}


iRobot::Status iRobot::setBaudRate(enum _BaudRate br)
{
   char cmd[50];
   int len = 0;
   cmd[len++] = iRobot::CMD_BAUD;
   cmd[len++] = br; 
   return txData(cmd, len);
}

iRobot::Status iRobot::setSafeMode()
{
   iRobot::Status rc;
   char cmd[50];
   int len = 0;

   cmd[len++] = iRobot::CMD_SAFE;
   if ((rc = txData(cmd, len)) == iRobot::OK) 
      mode = iRobot::MODE_SAFE;

   return rc;
}

iRobot::Status iRobot::setFullMode()
{
   iRobot::Status rc;
   char cmd[50];
   int len = 0;

   cmd[len++] = iRobot::CMD_FULL;
   if ((rc = txData(cmd, len)) == iRobot::OK) 
   	mode = iRobot::MODE_FULL;

   return rc;
}

iRobot::Status iRobot::setPassiveMode()
{
   return start();
}

iRobot::Status iRobot::setDemo(enum iRobot::_Demo demo)
{
   iRobot::Status rc;
   char cmd[50];
   int len = 0;

   cmd[len++] = iRobot::CMD_DEMO;
   cmd[len++] = demo;
   if ((rc = txData(cmd, len)) == iRobot::OK) 
      this->mode = iRobot::MODE_PASSIVE;

   return rc;
} 


iRobot::Status iRobot::drive(int16_t velocity, int16_t radius)
{
   iRobot::Status rc;
   char cmd[50];
   int len = 0;

   if ( mode == iRobot::MODE_FULL || mode == iRobot::MODE_SAFE ) {
      cmd[len++] = iRobot::CMD_DRIVE;
      cmd[len++] = (char)((velocity & 0xff00) >> 8);
      cmd[len++] = (char)(velocity & 0xff);
      cmd[len++] = (char)((radius & 0xff00) >> 8);
      cmd[len++] = (char)(radius & 0xff);
      return txData(cmd, len);
   }
   return iRobot::STA_ILLEGAL;
}


iRobot::Status iRobot::directDrive(int16_t right_velocity, int16_t left_velocity)
{
   iRobot::Status rc;
   char cmd[50];
   int len = 0;

   if ( mode == iRobot::MODE_FULL || mode == iRobot::MODE_SAFE ) {
      cmd[len++] = iRobot::CMD_DRIVE_DIRECT;
      cmd[len++] = (char)((right_velocity & 0xff00) >> 8);
      cmd[len++] = (char)(right_velocity & 0xff);
      cmd[len++] = (char)((left_velocity & 0xff00) >> 8);
      cmd[len++] = (char)(left_velocity & 0xff);
      return txData(cmd, len);
   }
   return iRobot::STA_ILLEGAL;
}


iRobot::Status iRobot::move(int16_t trav, int16_t turn)
{
   directDrive(trav-turn, trav+turn);
}

iRobot::Status iRobot::updateSensorData()
{
   iRobot::Status rc;
   char cmd[50];
   int len = 0;
   
   mSensor.angleHi = mSensor.angleLo = 0;
   mSensor.distanceHi = mSensor.distanceLo = 0;

   cmd[len++] = iRobot::CMD_SENSORS;
   cmd[len++] = 0; 
   rc = txData(cmd, len);
   if (rc != iRobot::OK) return rc;
   len = sizeof(mSensor);
   rc = rxData((char *)&mSensor, len, 10000); 
   if (rc != iRobot::OK) return rc; 

   deltaAngle = *(signed char *)&mSensor.angleHi;  
   deltaAngle *= 1 << CHAR_BIT;
   deltaAngle |= mSensor.angleLo;
   angle += deltaAngle; 

   deltaDistance = *(signed char *)&mSensor.distanceHi;  
   deltaDistance *= 1 << CHAR_BIT;
   deltaDistance |= mSensor.distanceLo;  
   distance += deltaDistance; 

   return iRobot::OK;
}


bool iRobot::getBumpsWheel(unsigned char bits)
{
   return ((mSensor.bumpsWheel & bits) != 0);
}

bool iRobot::wallDetected()
{
   return (mSensor.wall != 0);
}

bool iRobot::cliffLeft()
{
   return (mSensor.cliffLeft != 0);
}
   
bool iRobot::cliffRight()
{
   return (mSensor.cliffRight != 0);
}

bool iRobot::cliffFrontLeft()
{
   return (mSensor.cliffFrontLeft != 0);
}

bool iRobot::cliffFrontRight()
{
   return (mSensor.cliffFrontRight != 0);
}


bool iRobot::virtualWallDetected()
{
   return (mSensor.virtualWall == 0);
}


bool iRobot::getOverCurrent(unsigned char bits)
{
   return ((mSensor.overCurrent & bits) != 0); 
}


unsigned char iRobot::getInfrared()
{
   return mSensor.infrared;
}


bool iRobot::getButtons(unsigned char bits)
{
   return ((mSensor.buttons & bits) != 0);
}


long iRobot::getDistance()
{
   return distance;
}


long iRobot::getAngle()
{
   return angle;
}

int16_t iRobot::getDeltaDistance()
{
   return deltaDistance;
}

int16_t iRobot::getDeltaAngle()
{
   return deltaAngle;
}

enum iRobot::_ChargeState iRobot::getChargeState()
{
   return (enum iRobot::_ChargeState)mSensor.chargeState; 
}


uint16_t iRobot::getBatteryVoltage()
{
   uint16_t value;
   value = mSensor.batteryVoltageHi;
   value = value << 8 + mSensor.batteryVoltageLo;
   return value;
}


int16_t iRobot::getBatteryCurrent()
{
   int16_t value;
   value = mSensor.batteryCurrentHi;
   value = value << 8 + mSensor.batteryCurrentLo;
   return value;
}

char iRobot::getBatteryTemp()
{
   return (char) mSensor.batteryTemp;
}


uint16_t iRobot::getBatteryCharge()
{
   uint16_t value;
   value = mSensor.batteryChargeHi;
   value = value << 8 + mSensor.batteryChargeLo;
   return value;
}


uint16_t iRobot::getBatteryCapacity()
{
   uint16_t value;
   value = mSensor.batteryCapacityHi;
   value = value << 8 + mSensor.batteryCapacityLo;
   return value;
}

void iRobot::printSensors()
{
   printf("bumpsWheel       : 0x%02X\n", mSensor.bumpsWheel); 
   printf("wall             : 0x%02X\n", mSensor.wall); 
   printf("cliffLeft        : 0x%02X\n", mSensor.cliffLeft); 
   printf("cliffFrontLeft   : 0x%02X\n", mSensor.cliffFrontLeft); 
   printf("cliffFrontRight  : 0x%02X\n", mSensor.cliffFrontRight); 
   printf("cliffRight       : 0x%02X\n", mSensor.cliffRight); 
   printf("virtualWall      : 0x%02X\n", mSensor.virtualWall); 
   printf("overCurrent      : 0x%02X\n", mSensor.overCurrent); 
   printf("infrared         : 0x%02X\n", mSensor.infrared); 
   printf("buttons          : 0x%02X\n", mSensor.buttons); 
   printf("distanceHi       : 0x%02X\n", mSensor.distanceHi); 
   printf("distanceLo       : 0x%02X\n", mSensor.distanceLo); 
   printf("angleHi          : 0x%02X\n", mSensor.angleHi); 
   printf("angleLo          : 0x%02X\n", mSensor.angleLo); 
   printf("chargeState      : 0x%02X\n", mSensor.chargeState); 
   printf("batteryVoltageHi : 0x%02X\n", mSensor.batteryVoltageHi); 
   printf("batteryVoltageLo : 0x%02X\n", mSensor.batteryVoltageLo); 
   printf("batteryCurrentHi : 0x%02X\n", mSensor.batteryCurrentHi); 
   printf("batteryCurrentLo : 0x%02X\n", mSensor.batteryCurrentLo); 
   printf("batteryTemp      : 0x%02X\n", mSensor.batteryTemp); 
   printf("batteryChargeHi  : 0x%02X\n", mSensor.batteryChargeHi); 
   printf("batteryChargeLo  : 0x%02X\n", mSensor.batteryChargeLo); 
   printf("batteryCapacityHi: 0x%02X\n", mSensor.batteryCapacityHi); 
   printf("batteryCapacityLo: 0x%02X\n", mSensor.batteryCapacityLo); 
}

#undef __IROBOT_SERIAL_CPP__
/*! @} */

