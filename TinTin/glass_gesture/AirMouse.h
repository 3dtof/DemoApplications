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

#ifndef __AIRMOUSE_H__
#define __AIRMOUSE_H__


#define XDIM		80	
#define YDIM		60	
#define MAX_AMPLITUDE   2.0

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
   DepthFrame * _hand;
   float _ampGain;
   float _proximity;
   float _area;
   uint _illum_power;
   uint _intg;
   int _loopDelay;
   ClusterMap _cmap;
   pthread_t _thread;
   pthread_mutex_t _mtx;
   bool _debug;
   bool _isRunning;
   bool _lefthanded;

public:
   AirMouse();
   ~AirMouse();
   void start();
   void stop();
   inline bool isRunning() {return _isRunning;}
   inline void setDebug(bool v) {_debug = v;}
   inline bool debugging() {return _debug;}
   static void *eventLoop(void *m);
};

#endif // __AIRMOUSE_H__

