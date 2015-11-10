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

#define SAMPLE_COUNT	10
#define XDIM		80 
#define YDIM		60 

enum _State {
   START = 0,  
   FORWARD,		  
   SLOW,  
   PAUSE,  
   SEARCH,
   STOP 
};

enum _Situation {
   NO_OBS	= 0,
   NEAR_OBS,
   FAR_OBS
};

deque < DepthFrame > qFrame;
int gf = 0;
DepthFrame *frm;
int16_t dir = 1;


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



void frameCallback(DepthCamera &dc, const Frame &frame, 
                   DepthCamera::FrameType c)
{
   if (qFrame.size() < 2) 
   {
      const DepthFrame* f = 
                    dynamic_cast< const DepthFrame* > (&frame);
      qFrame.push_back(*f);
   }
   gf++;
}


int main (int argc, char* argv[])
{
  iRobot::Status rc;
  iRobot robot;
  int count = SAMPLE_COUNT;

  rc = robot.connect("/dev/ttyUSB0");
  if (rc != iRobot::OK) {
     cerr << "No iRobot found." << endl;
     return -1;
  }

  if (robot.start() != iRobot::OK) {
     cerr << "start() error." << endl;
     return -1;
  }

  robot.setFullMode();

  while ('q' != getkey()) {
     rc = robot.updateSensorData();
     if (rc != iRobot::OK) 
        cerr << "updateSensorData() error: " << rc << endl; 
     robot.printSensors();
     cout << "Distance = " << robot.getDistance() << endl;
     cout << "Angle = " << robot.getAngle() << endl;

     usleep(10000);
  } 

  robot.disconnect();

  return 0;
}
