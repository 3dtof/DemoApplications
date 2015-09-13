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

using namespace std;
using namespace Voxel;
using namespace pcl::visualization;

#define XDIM		80 
#define YDIM		60 

deque < DepthFrame > qFrame;
DepthFrame *frm;

int getkey();


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
      const DepthFrame* f = dynamic_cast< const DepthFrame* > (&frame);
      qFrame.push_back(*f);
   }
}


int main (int argc, char* argv[])
{
  ImageViewer *iv;

  logger.setDefaultLogLevel(LOG_INFO);
  CameraSystem sys;
  DepthCameraPtr depthCamera;
  
  const Vector<DevicePtr> &devices = sys.scan();
  
  if(devices.size() > 0) {
    depthCamera = sys.connect(devices[0]);  // Connect to first available device
  }
  else
  {
    cerr << "TVDemo: Could not find a compatible device." << endl;
    return -1;
  }
  
  if(!depthCamera)
  {
    cerr << "TVDemo: Could not open a depth camera." << endl;
    return -1;
  }
 
  if (!depthCamera->isInitialized())   
  {
     cerr << "Depth camera not initialized " << endl;
     return -1;
  }
 
  cout << "Successfully loaded depth camera " << endl; 
  
  depthCamera->registerCallback(DepthCamera::FRAME_DEPTH_FRAME, frameCallback);

  iv = new pcl::visualization::ImageViewer("Depth");
  iv->setSize(320, 240); 

  depthCamera->start();

  while ('q' != getkey()) {

      if (!qFrame.empty()) {

         frm = &qFrame.front(); 
         
         unsigned char *rgb = FloatImageUtils::getVisualImage(
                           frm->depth.data(), XDIM, YDIM, 0, 4);
         #iv->showRGBImage(rgb, XDIM, YDIM); 
         iv->showRGBImage(rgb, 320, 240); 
         delete rgb;

         qFrame.pop_front();
      }

      usleep(100000);

  } // end while 

  delete iv;
  depthCamera->stop();
  return 0;
}
