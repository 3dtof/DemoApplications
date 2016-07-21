/*! 
 * ============================================================================
 *
 * @addtogroup		TOFApp
 * @{
 *
 * @file		TOFApp.cpp
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
#define __TOFAPP_CPP__

#include "TOFApp.h"

#define FRAME_QUEUE_SZ		3

static deque< DepthFrame > qFrame; 
static pthread_mutex_t gmtx;

static void frameCallback(DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c)
{
   pthread_mutex_lock(&gmtx);
   if (qFrame.size() < FRAME_QUEUE_SZ) {
      const DepthFrame *f = dynamic_cast<const DepthFrame *>(&frame);
      qFrame.push_back(*f);
   }
   pthread_mutex_unlock(&gmtx);
}

void TOFApp::setIllumPower(uint power) 
{ 
   _illum_power = power; 
   _depthCamera->set("illum_power_percentage", _illum_power); 
}

void TOFApp::setExposure(uint exposure) 
{
   _intg = exposure;
   _depthCamera->set("intg_duty_cycle", _intg); 
}

void *TOFApp::eventLoop(void *p)
{
   bool done = false;
   bool empty;

   TOFApp *app = (TOFApp *)p;
   logger.setDefaultLogLevel(LOG_INFO);   

   if (!app->connect())
      goto err_exit;

   app->_isRunning = true;

   while (!done) {

      pthread_mutex_lock(&gmtx);
      empty = qFrame.empty();      
      pthread_mutex_unlock(&gmtx);
 
      if (!empty) {

         pthread_mutex_lock(&gmtx);
         DepthFrame *frm = &qFrame.front(); 
         pthread_mutex_unlock(&gmtx);

         app->update(frm);

         pthread_mutex_lock(&gmtx);
         qFrame.pop_front();
         pthread_mutex_unlock(&gmtx);
      }

      done = !app->_isRunning;    
      waitKey(app->_loopDelay);
   }
   
   app->disconnect();

err_exit:
   pthread_exit(NULL);
}


TOFApp::TOFApp()
{
   Init(TOF_WIDTH, TOF_HEIGHT);
}

TOFApp::TOFApp(int w, int h)
{
   Init(w, h);
}

TOFApp::~TOFApp()
{
}

void TOFApp::Init(int w, int h)
{
   _isRunning = false;
   _dimen.width = w;
   _dimen.height = h;
   _loopDelay = 20;
   _illum_power = 80U;
   _intg = 10U;
}


bool TOFApp::connect()
{
   const vector<DevicePtr> &devices = _sys.scan();
   if (devices.size() > 0) {
      _depthCamera = _sys.connect(devices[0]);
      if (!_depthCamera) 
         return false; 
      if (!_depthCamera->isInitialized()) 
         return false;
   }
   else 
      return false;

    int profile_id;
    const Map<int, Voxel::String> &profiles = _depthCamera->getCameraProfileNames();
    for(auto &p: profiles)
    {
      std::cout << p.first << ", " << p.second;

      if (p.second == "MetrilusLongRange")
         profile_id = p.first;
      
      ConfigurationFile *c = _depthCamera->configFile.getCameraProfile(p.first);
      
      if(c && c->getLocation() == ConfigurationFile::IN_CAMERA)
        std::cout << " (HW)";
      
      std::cout << std::endl;
    }

    cout << "Profile ID = " << profile_id << endl;
    if (_depthCamera->setCameraProfile(profile_id)) 
       cout << "Profile setting succeeded." << endl;

   _depthCamera->registerCallback(DepthCamera::FRAME_DEPTH_FRAME, frameCallback);
   _depthCamera->setFrameSize(_dimen);  
   _depthCamera->set("illum_power_percentage", _illum_power);
   _depthCamera->set("intg_duty_cycle", _intg);
   _depthCamera->start();

   return true;
}

void TOFApp::start()
{
   if (!_isRunning) 
      pthread_create(&_thread, NULL, &TOFApp::eventLoop, this);
}

void TOFApp::stop()
{      
   if (_isRunning) {
      _isRunning = false;
      pthread_join(_thread, NULL);
   }
}


void TOFApp::disconnect()
{
   _depthCamera->stop();
}


void TOFApp::update(DepthFrame *frm)
{
}

void TOFApp::lock()
{
   pthread_mutex_lock(&_mtx);
}

void TOFApp::unlock()
{
   pthread_mutex_unlock(&_mtx);
}


#undef __JIVE_CPP__
/*! @} */
