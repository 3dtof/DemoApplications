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

#define FRAME_QUEUE_SZ		   3
#define DEFAULT_ILLUM_POWER	100
#define DEFAULT_EXPOSURE	   20


//=============================================================================
// Frame callback & Thread Control
//=============================================================================
static deque<Voxel::Frame *> qFrame; 
static pthread_mutex_t gmtx;


static void frameCallback(DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c)
{
   pthread_mutex_lock(&gmtx);
   if (qFrame.size() < FRAME_QUEUE_SZ) {
      if (c == DepthCamera::FRAME_DEPTH_FRAME) {
         const Voxel::DepthFrame *f = dynamic_cast<const Voxel::DepthFrame *>(&frame);
         Voxel::Frame *nf = dynamic_cast<const Voxel::Frame *>(new Voxel::DepthFrame(*f));                    
         qFrame.push_back(nf);
      }
      else if (c == DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME) {
	      const Voxel::XYZIPointCloudFrame *f = dynamic_cast<const Voxel::XYZIPointCloudFrame *>(&frame);
         Voxel::Frame *nf = dynamic_cast<const Voxel::Frame *>(new Voxel::XYZIPointCloudFrame(*f));                    
         qFrame.push_back(nf);
      }
   }
   pthread_mutex_unlock(&gmtx);
}


void TOFApp::lock()
{
   pthread_mutex_lock(&_mtx);
}


void TOFApp::unlock()
{
   pthread_mutex_unlock(&_mtx);
}


//=============================================================================
//  Class initialization
//=============================================================================
TOFApp::TOFApp()
{
   Init(TOF_WIDTH, TOF_HEIGHT);
}


TOFApp::TOFApp(int w, int h)
{
   Init(w, h);
}


void TOFApp::Init(int w, int h)
{
   _isRunning = false;
   _isConnected = false;
   _dimen.width = w;
   _dimen.height = h;
   _loopDelay = 33;
   _profile = "Calibrated Normal";
}

//=============================================================================
// Accessors
//=============================================================================

bool TOFApp::setDim(int w, int h) 
{
   _dimen.width=w; 
   _dimen.height=h;
   return true;
}


DepthCameraPtr TOFApp::getDepthCamera() 
{
   return _depthCamera;
}


FrameSize &TOFApp::getDim() 
{
   return _dimen;
}


bool TOFApp::setLoopDelay(int delay) 
{
   _loopDelay = delay;
   return true;
}


bool TOFApp::getLoopDelay(int &delay) 
{
   delay = _loopDelay;
   return true;
}


bool TOFApp::setIllumPower(int power) 
{
   uint p = (uint)power;
   return _depthCamera->set("illum_power_percentage", p);
}


bool TOFApp::getIllumPower(int &power) 
{ 
   uint p;
   if (_depthCamera->get("illum_power_percentage", p)) 
   {
      power = (int)p;
      return true;
   }
   else
      return false; 
}


bool TOFApp::setExposure(int exposure) 
{
   uint e = (uint)exposure;
   return _depthCamera->set("intg_duty_cycle", e);
}


bool TOFApp::getExposure(int &integ) 
{ 
   uint i;
   if (_depthCamera->get("intg_duty_cycle", i)) 
   {
      integ = (int)i;
      return true;
   }
   else
      return false;
}


bool TOFApp::setProfile(Voxel::String name)
{
   bool rc = false;
   const Map<int, Voxel::String> &profiles = _depthCamera->getCameraProfileNames();
   for (auto &p: profiles) {
      if (p.second == name) {
         int profile_id = p.first;
         ConfigurationFile *c = _depthCamera->configFile.getCameraProfile(p.first);
         if (c && c->getLocation() == ConfigurationFile::IN_CAMERA) {
            if (_depthCamera->setCameraProfile(profile_id)) {
              rc = true;
              break;
            }
         }
      }
   }
   return rc;
}


Voxel::String TOFApp::getProfile()
{
   return _profile;
}


DepthCamera::FrameType TOFApp::getFrameType()
{
   return _frameType;
}


//=============================================================================
// Connection
//=============================================================================
bool TOFApp::connect(DepthCamera::FrameType frmType)
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

   // Setup spatial median filter
   FilterPtr p = _sys.createFilter("Voxel::MedianFilter", DepthCamera::FRAME_RAW_FRAME_PROCESSED); 
   if (!p) 
   {
      logger(LOG_ERROR) << "Failed to get MedianFilter" << std::endl;
      return -1;
   }
   p->set("deadband", 0.0f);
   _depthCamera->addFilter(p, DepthCamera::FRAME_RAW_FRAME_PROCESSED);
  

   // Setup temporal median filter
   p = _sys.createFilter("Voxel::TemporalMedianFilter",  DepthCamera::FRAME_RAW_FRAME_PROCESSED);
   if (!p)
   {
      logger(LOG_ERROR) << "Failed to get TemporalMedianFilter" << std::endl;
      return -1;
   }
   p->set("order", 5U);
   _depthCamera->addFilter(p, DepthCamera::FRAME_RAW_FRAME_PROCESSED);


   // Setup frame callback profile and settings
   _frameType = frmType;
   _depthCamera->registerCallback(_frameType, frameCallback);
   _depthCamera->setFrameSize(_dimen);
   if (setProfile(_profile)) 
      cout << "Profile " << _profile << " found." << endl;
   else 
      cout << "Profile " << _profile << "not found." << endl;
   setIllumPower(DEFAULT_ILLUM_POWER);
   setExposure(DEFAULT_EXPOSURE);

   // Start the camera
   _depthCamera->start();
   _isConnected = true;

   return true;
}


void TOFApp::disconnect()
{
   _depthCamera->stop();
}

//=============================================================================
// Run control
//=============================================================================
bool TOFApp::isRunning() 
{
   return _isRunning;
}


bool TOFApp::isConnected() 
{
   return _isConnected;
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


void *TOFApp::eventLoop(void *p)
{
   bool done = false;
   bool empty;

   TOFApp *app = (TOFApp *)p;
   logger.setDefaultLogLevel(LOG_INFO);   

   if (!app->isConnected()) 
      goto err_exit;

   app->_isRunning = true;

   while (!done) {

      pthread_mutex_lock(&gmtx);
      empty = qFrame.empty();      
      pthread_mutex_unlock(&gmtx);
 
      if (!empty) {

         pthread_mutex_lock(&gmtx);
         Voxel::Frame *frm = qFrame.front(); 
         pthread_mutex_unlock(&gmtx);
    
         app->update(frm);
         delete frm;

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


#undef __JIVE_CPP__
/*! @} */
