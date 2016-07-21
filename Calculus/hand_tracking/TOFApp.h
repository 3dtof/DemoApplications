/*! 
 * ============================================================================
 *
 * @addtogroup		TOFApp
 * @{
 *
 * @file		TOFApp.h
 * @version		1.0
 * @date		12/14/2015
 *
 * @note		Generalized TOF Application class
 * 
 * Copyright(c) 2007-2012 Texas Instruments Corporation, All Rights Reserved.
 * TI makes NO WARRANTY as to software products, which are supplied "AS-IS"
 *
 * ============================================================================
 */
#include <deque>
#include <CameraSystem.h>
#include <Common.h>
#include <unistd.h>
#include <termio.h>
#include <pthread.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifndef __TOFAPP_H__
#define __TOFAPP_H__

using namespace std;
using namespace Voxel;
using namespace cv;

#define TOF_WIDTH		320
#define TOF_HEIGHT		240

class TOFApp
{
public:
   TOFApp();
   TOFApp(int w, int h);
   ~TOFApp();
   void Init(int w, int h);
   inline void setDim(int w, int h) {_dimen.width=w; _dimen.height=h;};
   inline DepthCameraPtr getDepthCamera() {return _depthCamera;}
   inline FrameSize &getDim() {return _dimen;}
   inline void setLoopDelay(int delay) {_loopDelay = delay;}
   inline int getLoopDelay() {return _loopDelay;}
   inline uint getIllumPower() { return _illum_power; }
   inline uint getExposure() { return _intg; }
   void setIllumPower(uint power);
   void setExposure(uint exposure);
   bool isRunning();
   void start();
   void stop();
   bool connect();
   void disconnect();
   void lock();
   void unlock();
   static void *eventLoop(void *app);
   virtual void update(DepthFrame *frm);

private:
   pthread_t _thread;
   pthread_mutex_t _mtx;
   bool _isRunning;
   CameraSystem _sys;
   DepthCameraPtr _depthCamera;
   FrameSize _dimen;
   uint _illum_power;
   uint _intg;
   int _loopDelay;
};

#endif // __AIRMOUSE_H__
/*! @} */

