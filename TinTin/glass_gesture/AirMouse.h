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
#include <pthread.h>
#include <pcl/visualization/image_viewer.h>
#include <pcl/visualization/common/float_image_utils.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <Cluster.h>
#include <FakeMouse.h>
#include <Hyster.h>

#ifndef __AIRMOUSE_H__
#define __AIRMOUSE_H__


#define XDIM			80	
#define YDIM			60	
#define MAX_AMPLITUDE      	2.0
#define REFERENCE_SAMPLES  	16
#define WAIT_COUNT		16

using namespace std;
using namespace Voxel;
using namespace cv;
using namespace pcl::visualization;


class AirMouse : public FakeMouse
{
private:
   CameraSystem _sys;
   DepthCameraPtr _depthCamera;
   FrameSize _real;
   DepthFrame * _frm;
   DepthFrame _hand;
   float _ampGain;
   float _depthClip;
   float _ampClip;
   float _area;
   int _max_id;
   uint _illum_power;
   uint _intg;
   int _loopDelay;
   ClusterMap _cmap;
   pthread_t _thread;
   pthread_mutex_t _mtx;
   bool _debug;
   bool _isRunning;
   bool _lefthanded;
   bool _refSet;
   cv::Point _tip, _palm;
   float _referenceDepth;
   Hyster _buttonHyst;
   Mat _ampImg, _depthImg, _filterImg, _origImg;

public:
   AirMouse();
   ~AirMouse();
   void start();
   void stop();
   bool connect();
   void disconnect();
   Mat &clipBackground();
   cv::Point getPos();  
   float clickDist();
   int getButton();
   bool findKeyPoints();
   void action(cv::Point &mouse, int buttonState);
   void displayDebugInfo();
   inline bool isRunning() {return _isRunning;}
   inline void setDebug(bool v) {_debug = v;}
   inline bool debugging() {return _debug;}
   inline void setLeftHanded(bool v) {_lefthanded = v;}
   static void *eventLoop(void *m);
};

#endif // __AIRMOUSE_H__

