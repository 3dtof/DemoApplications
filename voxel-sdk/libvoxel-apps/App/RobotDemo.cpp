/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */
#include <deque>
#include <CameraSystem.h>
#include <Common.h>
#include <unistd.h>
#include <termio.h>
#include "PCLViewer.h"
#include <pcl/visualization/image_viewer.h>
#include <pcl/visualization/common/float_image_utils.h>
#include "irobot_serial.h"

using namespace std;
using namespace Voxel;
using namespace pcl::visualization;

#define TIMEOUT_COUNT	1000
#define MIN_AMP		(0.01)
#define SEARCH_COUNT    (7)
#define BACKUP_COUNT    (20)	
#define FAR_OBS_RANGE	(0.7)	
#define NEAR_OBS_RANGE  (0.3)	
#define XDIM		320 
#define YDIM		240 
#define VSAMP_START	(YDIM/4)
#define VSAMP_END	(2*YDIM/3)

enum _State {
   START = 0,  
   FORWARD,		  
   SLOW,  
   BACKUP,
   SEARCH_LEFT,
   SEARCH_RIGHT,
   STOP 
};

enum _Situation {
   NO_OBS	= 0,
   HIT_OBS,
   NEAR_OBS,
   FAR_OBS
};

deque < DepthFrame > qFrame;
int gf = 0;
DepthFrame *frm;
int16_t dir = 1;
int timeout = TIMEOUT_COUNT;
int search_count = SEARCH_COUNT;
int backup_count = BACKUP_COUNT;
iRobot robot;
int16_t travel, turn;
int g_dir;
bool bumped_left = false, bumped_right = false;
float minDist = 1e6;
float minAmp = -1;
float g_sum_left = 0, g_total_left = 0;
float g_sum_right = 0, g_total_right = 0;
int center = (YDIM/2)*XDIM+(XDIM/2);
float r = 0, a = 0;

int getkey();
int16_t openingDirection();
enum _Situation getSituation();


int getkey() {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}



enum _Situation getSituation()
{
   enum _Situation sit;

   minDist = 1e6;
   for (int y= VSAMP_START; y < VSAMP_END; y++) 
   { 
      for (int x= 0; x < XDIM; x++) 
      { 
         int idx = y*XDIM + x; 
         if (frm->depth[idx]>0 && frm->amplitude[idx]>MIN_AMP && frm->depth[idx]<minDist) 
         {
            minDist = frm->depth[idx];
            minAmp = frm->amplitude[idx];
         }
      } 
   }    
   if (minDist > 1e3) 
   {
      minDist = frm->depth[center];
      minAmp = frm->amplitude[center];
   } 

   if (robot.getBumpsWheel(iRobot::BUMP_LEFT | iRobot::BUMP_RIGHT)) {
      bumped_left = robot.getBumpsWheel(iRobot::BUMP_LEFT);
      bumped_right = robot.getBumpsWheel(iRobot::BUMP_RIGHT);
      sit = HIT_OBS;
   }
   else if (minDist > FAR_OBS_RANGE)
      sit = NO_OBS;
   else if (minDist > NEAR_OBS_RANGE)
      sit = FAR_OBS;
   else 
      sit = NEAR_OBS;
 
  return sit;
}


enum _State updateState(enum _State s)
{
   enum _State next = s;;
   enum _Situation sit = getSituation();

   if (s != SEARCH_LEFT && s != SEARCH_RIGHT) { 
      search_count = SEARCH_COUNT;
      timeout = TIMEOUT_COUNT;
   }
   if (s != BACKUP) 
      backup_count = BACKUP_COUNT;

   switch (s) 
   {
      case START:
         travel = iRobot::SPD_STOP;
         turn = iRobot::SPD_STOP;
         if (sit == NO_OBS)
            next = FORWARD;
         else if (sit == FAR_OBS)
            next = SLOW;
         else if (sit == NEAR_OBS) 
            next = (g_dir < 0) ? SEARCH_LEFT : SEARCH_RIGHT;
         break;

      case FORWARD:
         travel = iRobot::SPD_TRAV_FULL;
         turn = iRobot::SPD_STOP;
         if (sit == FAR_OBS) 
            next = SLOW;
         else if (sit == NEAR_OBS) 
            next = (g_dir < 0) ? SEARCH_LEFT : SEARCH_RIGHT;
         else if (sit == HIT_OBS)
            next = BACKUP;
         break;
   
      case SLOW:
         travel = iRobot::SPD_TRAV_LOW;
         turn = iRobot::SPD_STOP;
         if (sit == NO_OBS)
            next = FORWARD;
         else if (sit == NEAR_OBS) 
            next = (g_dir < 0) ? SEARCH_LEFT : SEARCH_RIGHT;
         else if (sit == HIT_OBS)
            next = BACKUP;
         break;

      case BACKUP:
         travel = -iRobot::SPD_TRAV_LOW;
         turn = iRobot::SPD_STOP;
         if (backup_count-- <= 0) {
            if (bumped_left & !bumped_right)
               next = SEARCH_RIGHT;
            else if (!bumped_left & bumped_right)
               next = SEARCH_LEFT;
            else
               next = SEARCH_RIGHT;
            bumped_left = bumped_right = false;
         }
         break;

      case SEARCH_LEFT:
         if (timeout-- > 0) 
         {
            travel = iRobot::SPD_STOP;
            turn = -iRobot::SPD_TURN_LOW;
            if (sit == NO_OBS || sit == FAR_OBS) 
               search_count--;
            else
               search_count = SEARCH_COUNT;

            if (sit == NO_OBS && search_count == 0)
               next = FORWARD;
            else if (sit == FAR_OBS && search_count == 0)
               next = SLOW; 
            else if (sit == HIT_OBS)
               next = BACKUP;
         }
         else {
            next = STOP;
         } 
         break;

      case SEARCH_RIGHT:
         if (timeout-- > 0) 
         {
            travel = iRobot::SPD_STOP;
            turn = iRobot::SPD_TURN_LOW;
            if (sit == NO_OBS || sit == FAR_OBS) 
               search_count--;
            else
               search_count = SEARCH_COUNT;

            if (sit == NO_OBS && search_count == 0)
               next = FORWARD;
            else if (sit == FAR_OBS && search_count == 0)
               next = SLOW; 
            else if (sit == HIT_OBS)
               next = BACKUP;
         }
         else {
            next = STOP;
         } 
         break;

      case STOP:
      default:
         travel = iRobot::SPD_STOP;
         turn = iRobot::SPD_STOP;
         break;     
   }

   return next;
}


int16_t openingDirection()
{
   float sum_left = 0, sum_right = 0; 
   float total_left = 1, total_right = 1;

   for (int y=VSAMP_START; y < VSAMP_END; y++) 
   {  
      for (int x=0; x < XDIM/2; x++) 
      {
         int idx = y*XDIM+x;
         if (frm->depth[idx] > 0 && frm->amplitude[idx] > 0) 
         { 
            sum_left += frm->depth[idx];
            total_left++;
         }
      }

      for (int x=XDIM/2; x < XDIM; x++) 
      { 
         int idx = y*XDIM+x;
         if (frm->depth[idx] > 0 && frm->amplitude[idx] > 0) 
         { 
            sum_right += frm->depth[idx];  
            total_right++;
         }
      }
   }

   g_total_left = total_left;
   g_total_right = total_right;
   g_sum_left = sum_left;
   g_sum_right = sum_right;

   if (sum_left/total_left >= sum_right/total_right)
      return -1;
   else 
      return 1;
}


void frameCallback(DepthCamera &dc, const Frame &frame, 
                   DepthCamera::FrameType c)
{
   if (qFrame.size() < 2) 
   {
      const DepthFrame* f = dynamic_cast< const DepthFrame* > (&frame);
      qFrame.push_back(*f);
   }
   gf++;
}


int main (int argc, char* argv[])
{
  enum _State state = START;
  iRobot::Status rc;
  bool verbose = true, simulate = false; 
  ImageViewer *iv;

  if (argc == 2 && *argv[1] == 's') 
     simulate = true;
 
  logger.setDefaultLogLevel(LOG_INFO);
  CameraSystem sys;
  DepthCameraPtr depthCamera;
  
  const Vector<DevicePtr> &devices = sys.scan();
  
  if(devices.size() > 0) {
    depthCamera = sys.connect(devices[0]);  // Connect to first available device
  }
  else
  {
    cerr << "RobotDemo: Could not find a compatible device." << endl;
    return -1;
  }
  
  if(!depthCamera)
  {
    cerr << "RobotDemo: Could not open a depth camera." << endl;
    return -1;
  }
 
  if (!depthCamera->isInitialized())   
  {
     cerr << "Depth camera not initialized " << endl;
     return -1;
  }
 
  cout << "Successfully loaded depth camera " << endl; 
  
  depthCamera->registerCallback(DepthCamera::FRAME_DEPTH_FRAME, frameCallback);

  if (verbose) {
     iv = new pcl::visualization::ImageViewer("Depth");
     iv->setSize(320, 240); 
  }

  depthCamera->start();

  rc = robot.connect("/dev/ttyUSB0");
  if (rc != iRobot::OK) {
     cerr << "No iRobot found." << endl;
     return -1;
  }

  if (robot.start() != iRobot::OK) {
     cerr << "start() error." << endl;
     return -1;
  }

  if (!simulate) {
     if (robot.setDemo(iRobot::DEMO_BANJO) != iRobot::OK) {
        cerr << "setDemo() error." << endl;
        return -1;
     }
  }

  sleep(5);

  robot.setFullMode();

  while ('q' != getkey()) {

      if (robot.updateSensorData() != iRobot::OK)
          cout << "robot.updateSensorData() error" << endl;
 
      if (!qFrame.empty()) {

         frm = &qFrame.front(); 
         
         if (verbose) {
            unsigned char *rgb = FloatImageUtils::getVisualImage(
                                  frm->depth.data(), XDIM, YDIM, 0, 4);
            iv->showRGBImage(rgb, XDIM, YDIM); 
            delete rgb;
         }

         g_dir = openingDirection();
         state = updateState(state);
   
         if (state == START)  
            cout << "START   ";  
         else if (state == FORWARD)		  
            cout << "FORWARD ";  
         else if (state == SLOW)		  
            cout << "SLOW    ";  
         else if (state == BACKUP)		  
            cout << "BACKUP  ";  
         else if (state == SEARCH_RIGHT)		  
            cout << "SCH_RT  ";  
         else if (state == SEARCH_LEFT)		  
            cout << "SCH_LF  ";  
         else if (state == STOP)		  
            cout << "STOP    ";  
       
	 r = (float)robot.getDistance();
	 a = (float)robot.getAngle(); 

        // robot.printSensors();
         printf("Range:%1.3f  Amp:%0.4f  LF:%1.3f  RT:%1.3f  dir:%s r:%f  a:%f\n",
                 minDist, minAmp, g_sum_left/g_total_left, g_sum_right/g_total_right,
                 (g_dir < 0) ? "Left" : "Right", r, a);    
        
         qFrame.pop_front();
      }

      if (!simulate) 
         robot.move(travel, turn); 
      

      usleep(10000);

  } // end while 

  // Stop the robot
  if (!simulate) 
     robot.move(iRobot::SPD_STOP, iRobot::SPD_STOP);

end_it:
  if (verbose) 
     delete iv;
  depthCamera->stop();
  return 0;
}
