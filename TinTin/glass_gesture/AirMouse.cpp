/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */
#define __AIRMOUSE_CPP__

#include <cmath>
#include "AirMouse.h"


static deque < DepthFrame > qFrame; 
static void frameCallback(DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c);

void frameCallback(DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c)
{
   if (qFrame.size() < 2) {
      const DepthFrame *f = dynamic_cast<const DepthFrame *>(&frame);
      qFrame.push_back(*f);
   }
}

AirMouse::AirMouse()
{
   _debug = false;
   _real.width = XDIM;
   _real.height = YDIM;
   _hand.size = _real;
   _ampGain = 10.0;
   _depthClip = 1.5;
   _ampClip = 0.01;
   _area = 0;
   _illum_power = 48U;
   _intg = 1U;
   _loopDelay = 20;
   _cmap.setAttr(0.8, 0.1, 3);
   _isRunning = false;   
   _lefthanded = false;
}

AirMouse::~AirMouse()
{
   if (_isRunning) stop();
}

void AirMouse::start()
{
   if (!_isRunning) 
      pthread_create(&_thread, NULL, &AirMouse::eventLoop, this);
}

void AirMouse::stop()
{      
   bool running;

   pthread_mutex_lock(&_mtx);
   running = _isRunning;
   pthread_mutex_unlock(&_mtx);

   if (running) {
      pthread_mutex_lock(&_mtx);
      _isRunning = false;
      pthread_mutex_unlock(&_mtx);
      pthread_join(_thread, NULL);
   }
}


Mat &AirMouse::clipBackground()
{
   _hand.depth.clear();          
   _hand.amplitude.clear();
   for (int i = 0; i < _frm->depth.size(); i++) {
      _hand.depth.push_back((_frm->depth[i] < _depthClip && _frm->amplitude[i] > _ampClip) ? _frm->depth[i] : 0.0);
      _hand.amplitude.push_back((_frm->depth[i] < _depthClip && _frm->amplitude[i] > _ampClip) ? _ampGain : 0.0);
   }  
   _ampImg = Mat(_hand.size.height, _hand.size.width, CV_32FC1, _hand.amplitude.data());  

   return _ampImg;
}


bool AirMouse::findKeyPoints()
{
   int id;
   float dist2, max_dist2 = 0.0;

   if (_cmap.largestCluster(_max_id)) {
      Cluster *c = &_cmap.getClusters()[_max_id];

      // Find tip
      for (int i=0; i < c->getPoints().size(); i++) {
         POINT p = c->getPoints()[i];
         if (_lefthanded) {
	    if ((p.y <= c->getMin().y) || (p.x >= c->getMax().x)) {
               dist2 = (p.x)*(p.x) + (_real.height-p.y)*(_real.height-p.y);
               if (dist2 > max_dist2) {
                  max_dist2 = dist2;
                  _tip = cv::Point((int)floor(p.x), (int)floor(p.y));
               }
            }
         }
         else {
	    if ((p.y <= c->getMin().y) || (p.x <= c->getMin().x)) {
               dist2 = (_real.width-p.x)*(_real.width-p.x) + (_real.height-p.y)*(_real.height-p.y);
               if (dist2 > max_dist2) {
                  max_dist2 = dist2;
                  _tip = cv::Point((int)floor(p.x), (int)floor(p.y));
               }
            }
         }	          
      }

      // Find palm
      POINT centroid = c->getCentroid();
      _palm = cv::Point((int)floor(centroid.x), (int)floor(centroid.y));

      return true;
   }
   return false;
}


cv::Point AirMouse::getPos()
{
   cv::Point m;
   int s_width, s_height;
   getDim(s_width, s_height);
   m.x = (_tip.x*s_width)/_real.width;
   m.y = (_tip.y*s_height)/_real.height;
   return m;
}


float AirMouse::clickDist()
{
   return _frm->depth[_frm->size.width*_tip.y+_tip.x]; 
}


int AirMouse::getButton()
{
   float click_dist = clickDist();
   if (debugging()) 
      cout << "click_dist = " << click_dist << " LB = " 
           << _buttonHyst.getLowerB() << "  UB =" << _buttonHyst.getUpperB() << endl; 
   bool buttonDown = _buttonHyst.update(click_dist);
   if (buttonDown) 
      return ButtonPress;
   else
      return ButtonRelease;
}


void AirMouse::action(cv::Point &mouse, int buttonState)
{
    if (debugging())
       cout << "Mouse at (" << mouse.x << "," << mouse.y << ")" << endl;
    else
       moveTo(mouse.x, mouse.y);
         
    if (buttonState == ButtonPress) {
       if (debugging())
          cout << "Button Down" << endl;
       else
          buttonDown(Button1);
    }
    else if (buttonState == ButtonRelease) {
       if (debugging())
          cout << "Button Up" << endl;
       else
          buttonUp(Button1);
    }
}

void AirMouse::displayDebugInfo()
{
   // Create image windows
   namedWindow( "DepthThresh", WINDOW_NORMAL );
   namedWindow( "Original", WINDOW_NORMAL );
   namedWindow( "Filtered", WINDOW_NORMAL );
   namedWindow( "AmpThresh", WINDOW_NORMAL );

   // Create depth image
   unsigned char *depth = FloatImageUtils::getVisualImage(_hand.depth.data(), 
                                                          _hand.size.width, _hand.size.height, 
                                                          0, MAX_AMPLITUDE,false);  
   _depthImg = Mat(_hand.size.height, _hand.size.width, CV_8UC3, depth);


   // Create orig image
   unsigned char *orig = FloatImageUtils::getVisualImage(_frm->amplitude.data(), 
                                                         _frm->size.width, _frm->size.height, 
                                                         0, MAX_AMPLITUDE,true);  
   _origImg  = Mat(_frm->size.height, _frm->size.width, CV_8UC3, orig);


   // Create filter_img which shows only the largest cluster
   _filterImg = Mat(_hand.size.height, _hand.size.width, CV_32FC1, Scalar(0));
   if (_cmap.getClusters().size() > 0) {

      Cluster *c = &_cmap.getClusters()[_max_id];
      
      // Create image of just the largest cluster
      for (int i=0; i < c->getPoints().size(); i++) {
         POINT p = c->getPoints()[i];
         _filterImg.at<float>(p.y, p.x) = p.z;
      }

      // Overlay bounding box and tip point in orig_img
      cv::Point Pmax = cv::Point(c->getMax().x, c->getMax().y);
      cv::Point Pmin = cv::Point(c->getMin().x, c->getMin().y);
      rectangle(_origImg, Pmin, Pmax, Scalar(255));
      circle(_origImg, _tip, 3, Scalar(255));
      circle(_origImg, _palm, 3, Scalar(255));

      // Output debug data
      cout << "tip=(" << _tip.x << "," << _tip.y << ") "
           << "tip depth=" << _frm->depth[_frm->size.width*_tip.y+_tip.x] << " "
           << "palm=(" << _palm.x << "," << _palm.y << ") "
           << "palm depth=" << _frm->depth[_frm->size.width*_palm.y+_palm.x] << " "
           << "click_dist=" << clickDist() << endl;
   }

   // Output depth and amplitude at image center
   int mid = (_frm->size.width*_frm->size.height + _frm->size.width)/2;
   cout << "center amp=" << _frm->amplitude[mid] << " "
        << "center depth=" << _frm->depth[mid] << endl;
       
   // Show the images
   imshow("Filtered", _filterImg);
   imshow("Original", _ampGain*_origImg);
   imshow("DepthThresh", _depthImg);
   imshow("AmpThresh", _ampImg);

   delete depth;  
   delete orig;   
}


bool AirMouse::connect()
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
   
   _depthCamera->registerCallback(DepthCamera::FRAME_DEPTH_FRAME, frameCallback);
   _depthCamera->setFrameSize(_real);  
   _depthCamera->set("illum_power_percentage", _illum_power);
   _depthCamera->set("intg_duty_cycle", _intg);
   _depthCamera->start();

   return true;
}


void AirMouse::disconnect()
{
   _depthCamera->stop();
}


void *AirMouse::eventLoop(void *p)
{
   AirMouse *m = (AirMouse *)p;
   int sample_count = REFERENCE_SAMPLES;
   int wait_count = WAIT_COUNT;
   float sum = 0, sum2 = 0;

   if (m->debugging()) 
      logger.setDefaultLogLevel(LOG_INFO);   

   if (!m->connect())
      goto err_exit2;

   m->_isRunning = true;
   m->_refSet = false;
   while (m->_isRunning) {
      if (!qFrame.empty()) {
         m->_frm = &qFrame.front(); 
         m->clipBackground();
         m->_cmap.scan(m->_ampImg);
         if (m->findKeyPoints()) {
            if (m->_refSet) {
               cv::Point mouse = m->getPos();
               int button = m->getButton();
               m->action(mouse, button);
            }
            else {
               if (wait_count == 0) {
                  if (sample_count-- == 0) { 
                     m->_referenceDepth = sum/REFERENCE_SAMPLES;
                     float sigma = sqrt(sum2/REFERENCE_SAMPLES - m->_referenceDepth*m->_referenceDepth);
                     float ub = m->_referenceDepth+4*sigma;
                     float lb = m->_referenceDepth;
                     m->_buttonHyst = Hyster(ub, lb, 1);
                     m->_refSet = true;
                     sum = sum2 = 0;
                     if (m->debugging()) {
                        cout << "Calibrated ===============================================" << endl;
                        cout << "Reference = " << m->_referenceDepth << "   Sigma = " << sigma << endl;
                        cout << "LB = " << lb << "  UB = " << ub << endl;
                     }
                  } 
                  else {
                     float x = m->_frm->depth[m->_frm->size.width*m->_tip.y+m->_tip.x];
                     sum += x;
                     sum2 += (x*x);
                  }
               }
               else
                  wait_count--;
            }
         }
         else {
            m->_refSet = false;
            sample_count = REFERENCE_SAMPLES;
            sum = sum2 = 0;
            wait_count = WAIT_COUNT;
         }

#if 1
         if (m->debugging())  
            m->displayDebugInfo();
#endif
         qFrame.pop_front();
      } 
      waitKey(m->_loopDelay);
   } 

err_exit1:
   m->disconnect();

err_exit2:
   pthread_exit(NULL);
}

#ifdef __UNIT_TEST__
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

int main(int argc, char *argv[])
{
   AirMouse am;
   
   am.start();

   while (!done) {
      int key = getkey();
      if (key == 'q') done = true;
      usleep(10);
   }
}
#undef __UNIT_TEST__
#endif // __UNIT_TEST__


#undef __AIRMOUSE_CPP__


