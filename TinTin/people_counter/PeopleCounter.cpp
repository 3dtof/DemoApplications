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
#include <pcl/visualization/image_viewer.h>
#include <pcl/visualization/common/float_image_utils.h>
#include <Cluster.h>

using namespace std;
using namespace Voxel;
using namespace pcl::visualization;

#define XDIM		160 
#define YDIM		120	
#define MAX_DIST	3


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


void frameCallback(DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c)
{
   if (qFrame.size() < 2) 
   {
      const DepthFrame *f = dynamic_cast<const DepthFrame *>(&frame);
      qFrame.push_back(*f);
   }
}


int main (int argc, char* argv[])
{
  ImageViewer *iv;

  logger.setDefaultLogLevel(LOG_INFO);
  CameraSystem sys;
  DepthCameraPtr depthCamera;
  ClusterMap cmap(0.8, 0.4, 3);
  DepthFrame d, bkgnd;
  FrameSize frame_sz;

  frame_sz.width = XDIM;
  frame_sz.height = YDIM;

  d.size.width = bkgnd.size.width = XDIM;
  d.size.height = bkgnd.size.height = YDIM;

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


  iv= new pcl::visualization::ImageViewer("Depth");
  iv->setSize(XDIM, YDIM); 

  depthCamera->setFrameSize(frame_sz);
  depthCamera->start();


  bool done = false;
  bool is_running = false;

  while (!done) {

      char key = getkey();
      
      if ('q' == key) {
         done = true;
         cout << "Exit." << endl;
      }
      else if ('s' == key) {
         if (is_running) {
            is_running = false;
            cout << "Stopped." << endl;
         }
         else {
            is_running = true;
            cout << "Started." << endl;
         }
      }

      if (!qFrame.empty()) {

         frm = &qFrame.front(); 

         if (!is_running) {
            bkgnd = *frm;
            unsigned char *rgb = FloatImageUtils::getVisualImage(bkgnd.amplitude.data(), XDIM, YDIM, 0, MAX_DIST);
            iv->showRGBImage(rgb, XDIM, YDIM);             
            iv->removeLayer("rectangles");
            delete rgb;
         }
         else {
            d.depth.clear();
            d.amplitude.clear();
            for (int i = 0; i < frm->depth.size(); i++) {
               float diff = bkgnd.depth[i] - frm->depth[i];
               d.depth.push_back((diff < 0) ? 0 : diff);
            }
            cmap.Scan(d);
            unsigned char *rgb = FloatImageUtils::getVisualImage(frm->amplitude.data(), XDIM, YDIM, 0, MAX_DIST);
            iv->showRGBImage(rgb, XDIM, YDIM); 
            iv->removeLayer("rectangles");
            for (int i = 0; i < cmap.GetClusters().size(); i++) {
               if (cmap.GetClusters()[i].GetArea() > 100) {
                  iv->addRectangle(cmap.GetClusters()[i].GetMin().x, cmap.GetClusters()[i].GetMax().x,
                                YDIM-cmap.GetClusters()[i].GetMin().y, YDIM-cmap.GetClusters()[i].GetMax().y,
                                1.0, 1.0, 1.0);
               }
            }

            delete rgb;
         }  

         qFrame.pop_front();
      }
  } // end while 

  delete iv;
  depthCamera->stop();
  return 0;
}
