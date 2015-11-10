/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */
#include <cstdlib>
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

#define XDIM		(80) 
#define YDIM		(60) 
#define FIFO_SIZE	(3)
#define DELAY_COUNT	(250)

deque < DepthFrame > qFrame;
DepthFrame *frm;
DepthFrame *frmbuf;
DepthFrame *frmbknd;

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
   if (qFrame.size() < FIFO_SIZE) 
   {
      const DepthFrame* f = dynamic_cast< const DepthFrame* > (&frame);
      qFrame.push_back(*f);
   }
}

DepthFrame *InitDepthFrame(DepthFrame *f, int x_sz, int y_sz)
{
  for (int i = 0; i < x_sz*y_sz; i++) 
  {
     f->depth.push_back(0);
     f->amplitude.push_back(0);
  }
  f->size.width = y_sz;
  f->size.height = x_sz;
  return f;
}


DepthFrame *iirFilter(DepthFrame *f, float coef)
{
   for (int i = 0; i < f->size.width * f->size.height; i++) 
   {
      frmbuf->depth[i] = (1.0f - coef) * (frmbuf->depth[i])
                        + coef * (f->depth[i]);
   }
   return frmbuf;
}

bool detectObject(float dist)
{
#if 0
   for (int x = XDIM/8; x < XDIM-XDIM/8; x++) {
      for (int y = YDIM/8; y < YDIM-YDIM/8; y++) { 
         int i = XDIM*y+x;
         if (frmbuf->depth[i] < dist && fabs(frmbknd->depth[i] - frmbuf->depth[i]) > 0.3) 
             return true;
      }
   }
#else
   int i = XDIM*(YDIM/2)+XDIM/2;
   if (frmbuf->depth[i] < dist && fabs(frmbknd->depth[i] - frmbuf->depth[i]) > 0.3) 
       return true;
#endif
   return false;
}

int main (int argc, char* argv[])
{
  ImageViewer *iv;

  logger.setDefaultLogLevel(LOG_INFO);
  CameraSystem sys;
  DepthCameraPtr depthCamera;
  

  if (argc != 2) {
     cout << "Usage: sudo TVDemo <dist>" << endl;
     exit(1);
  }
  
  float dist = atof(argv[1]);

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
  

  frmbuf = InitDepthFrame(new DepthFrame(), XDIM, YDIM);
  frmbknd = InitDepthFrame(new DepthFrame(), XDIM, YDIM);

  depthCamera->registerCallback(DepthCamera::FRAME_DEPTH_FRAME, frameCallback);

  iv = new pcl::visualization::ImageViewer("Depth");
  iv->setSize(320, 240); 

  depthCamera->start();

  bool done = false;
  bool prevDetected = false;
  bool init = false;
  bool background_init = false;
  int count = 0;
  int tv_count = DELAY_COUNT; 

  while (!done) {
  
      int key = getkey();
      if (key == 'q') done = true;

      if (!qFrame.empty()) {

         frm = &qFrame.front(); 
         int index = YDIM*(XDIM/2)+YDIM/2;
         iirFilter(frm, 0.1f);
         cout << frmbuf->depth.data()[index] << endl;

         unsigned char *rgb = FloatImageUtils::getVisualImage(
                              frmbuf->depth.data(), XDIM, YDIM, 0, 6);
         iv->showRGBImage(rgb, XDIM, YDIM); 
         delete rgb;

         if (count > 30) {
            init = true;
            cout << "init = true" << endl;
            if (!background_init) {
               *frmbknd = *frmbuf;
               background_init = true;
               cout << "background_init = true" << endl;
            }
         }
         else 
            count++; 

         qFrame.pop_front();
      }

      if (background_init) {
         if (detectObject(dist)) {
            cout << "People in Range" << endl;
            tv_count = DELAY_COUNT; 
            if (!prevDetected) {
               system("irsend -d /var/run/lirc/lircd1 SEND_ONCE samsung.conf KEY_POWER");
               prevDetected = true;
            }
         }
         else {
            cout << "..............." << endl;
            if (prevDetected && tv_count-- == 0) { 
                system("irsend -d /var/run/lirc/lircd1 SEND_ONCE samsung.conf KEY_POWER");
                prevDetected = false; 
            }
         }
      }  
      usleep(66000);

  } // end while 

  delete iv;
  depthCamera->stop();
  return 0;
}
