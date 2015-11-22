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


AirMouse::AirMouse()
{
   _debug = false;
   _real.width = XDIM;
   _real.height = YDIM;
   _ampGain = 10.0;
   _proximity = 1.5;
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


void frameCallback(DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c)
{
   if (qFrame.size() < 2) {
      const DepthFrame *f = dynamic_cast<const DepthFrame *>(&frame);
      qFrame.push_back(*f);
   }
}
  
void *AirMouse::eventLoop(void *p)
{
   int rc = 0;
   AirMouse *m = (AirMouse *)p;
   DepthFrame *frm, hand;
   POINT palm;
   cv::Point palm_center, tip_center;
   float click_dist = 0;

   hand.size = m->_real;

   // If debug enable error logging and display
   //
   if (m->debugging()) 
      logger.setDefaultLogLevel(LOG_INFO);   hand.size = m->_real;

   // Connect to TOF camera
   //
   const Vector<DevicePtr> &devices = m->_sys.scan();
   if (devices.size() > 0) {
     m->_depthCamera = m->_sys.connect(devices[0]);
     if (!m->_depthCamera) 
        goto err_exit2; 
     if (!m->_depthCamera->isInitialized()) 
        goto err_exit2;
   }
   else 
      goto err_exit2;
   
   m->_depthCamera->registerCallback(DepthCamera::FRAME_DEPTH_FRAME, frameCallback);
   m->_depthCamera->setFrameSize(m->_real);  
   cout << "power = " << m->_depthCamera->set("illum_power_percentage", m->_illum_power) << endl;
   cout << "duty = " << m->_depthCamera->set("intg_duty_cycle", m->_intg) << endl;
   m->_depthCamera->start();

 
   // Thread loop
   //
   m->_isRunning = true;
   while (m->_isRunning) {

      if (!qFrame.empty()) {

         frm = &qFrame.front(); 

         // Clip background (objects > proximity) 
         //
         hand.depth.clear();          
         hand.amplitude.clear();
         for (int i = 0; i < frm->depth.size(); i++) {
            hand.depth.push_back((frm->depth[i] < m->_proximity && frm->amplitude[i] > 0.01) ? frm->depth[i] : 0.0);
            hand.amplitude.push_back((frm->depth[i] < m->_proximity && frm->amplitude[i] > 0.01) ? m->_ampGain : 0.0);
         }    

         // Find the largest cluster, which should be the hand
         //
         Mat amp_img = Mat(hand.size.height, hand.size.width, CV_32FC1, hand.amplitude.data());
         m->_cmap.scan(amp_img);
         int max_cluster_id = 0;
         float max_area = 0;
         for (int i=0; i < m->_cmap.getClusters().size(); i++) {
            if (m->_cmap.getClusters()[i].getArea() > max_area) {
               max_area = m->_cmap.getClusters()[i].getArea();
               max_cluster_id = i;
            }
         }

         // Find the finger tip which is the cluster point farthest from 
         // bottom-right corner (if rigthhanded), or bottom-left corner 
         // (if lefthanded)
         //
         POINT tip = POINT(amp_img.cols,amp_img.rows,0);
	 float dist2, max_dist2 = 0.0;

         if (m->_cmap.getClusters().size() > 0) {
            
            for (int i=0; i < m->_cmap.getClusters()[max_cluster_id].getPoints().size(); i++) {
	       POINT p = m->_cmap.getClusters()[max_cluster_id].getPoints()[i];
              
               if (m->_lefthanded) {
	          if ((p.y <= m->_cmap.getClusters()[max_cluster_id].getMin().y) ||
                             (p.x >= m->_cmap.getClusters()[max_cluster_id].getMax().x)) {
                     dist2 = (p.x)*(p.x)
                           + (amp_img.rows-p.y)*(amp_img.rows-p.y);
                     if (dist2 > max_dist2) {
                        max_dist2 = dist2;
                        tip = p;
                     }
                  }
               }
               else {
	          if ((p.y <= m->_cmap.getClusters()[max_cluster_id].getMin().y) ||
                             (p.x <= m->_cmap.getClusters()[max_cluster_id].getMin().x)) {
                     dist2 = (amp_img.cols-p.x)*(amp_img.cols-p.x)
                           + (amp_img.rows-p.y)*(amp_img.rows-p.y);
                     if (dist2 > max_dist2) {
                        max_dist2 = dist2;
                        tip = p;
                     }
                  }
               }	          
  	    }
 
            palm = m->_cmap.getClusters()[max_cluster_id].getCentroid();
            palm_center = cv::Point((int)floor(palm.x), (int)floor(palm.y));
            tip_center = cv::Point((int)floor(tip.x), (int)floor(tip.y));
            int s_width, s_height;
            m->getDim(s_width, s_height);
            int mouse_x = (tip_center.x*s_width)/amp_img.cols;
            int mouse_y = (tip_center.y*s_height)/amp_img.rows;

            if (m->debugging())
               cout << "Mouse at (" << mouse_x << "," << mouse_y << ")" << endl;
            else
               m->moveTo(mouse_x, mouse_y);
         
            click_dist = hand.depth[hand.size.width*tip.y+tip.x] - hand.depth[hand.size.width*palm.y+palm.x];

            if (m->debugging())
               cout << "Click dist = " << click_dist << endl;

            if (click_dist > 1) {
               if (m->debugging())
                  cout << "Button Down" << endl;
               else
                  m->buttonDown(Button1);
            }
            else {
               if (m->debugging())
                  cout << "Button Up" << endl;
               else
                  m->buttonUp(Button1);
            }

         } // if (cmap.getClusters().size() > 0)
         
 
         // If in debug mode, display images & debug data
         //
         if (m->debugging()) {

            namedWindow( "DepthThresh", WINDOW_NORMAL );
            namedWindow( "Original", WINDOW_NORMAL );
            namedWindow( "Filtered", WINDOW_NORMAL );
            namedWindow( "AmpThresh", WINDOW_NORMAL );

            // Create depth image
            //
            unsigned char *depth = FloatImageUtils::getVisualImage(hand.depth.data(), 
                                                                 hand.size.width, hand.size.height, 
                                                                 0, MAX_AMPLITUDE,false);  
            Mat depth_img = Mat(hand.size.height, hand.size.width, CV_8UC3, depth);


            // Create orig image
            //
            unsigned char *orig = FloatImageUtils::getVisualImage(frm->amplitude.data(), 
                                                                 frm->size.width, frm->size.height, 
                                                                 0, MAX_AMPLITUDE,true);  
            Mat orig_img  = Mat(frm->size.height, frm->size.width, CV_8UC3, orig);


            // Create filter_img which shows only the largest cluster
            //
            Mat filter_img = Mat(hand.size.height, hand.size.width, CV_32FC1, Scalar(0));

            if (m->_cmap.getClusters().size() > 0) {
               for (int i=0; i < m->_cmap.getClusters()[max_cluster_id].getPoints().size(); i++) {
	          POINT p = m->_cmap.getClusters()[max_cluster_id].getPoints()[i];
                  filter_img.at<float>(p.y, p.x) = p.z;
               }

               // Overlay bounding box and tip point in orig_img
               if (max_area > m->_area) {
          	   cv::Point Pmax = cv::Point(m->_cmap.getClusters()[max_cluster_id].getMax().x, 
                                    m->_cmap.getClusters()[max_cluster_id].getMax().y);
         	   cv::Point Pmin = cv::Point(m->_cmap.getClusters()[max_cluster_id].getMin().x, 
                                    m->_cmap.getClusters()[max_cluster_id].getMin().y);
         	   rectangle(orig_img, Pmin, Pmax, Scalar(255));
                   circle(orig_img, tip_center, 3, Scalar(255));
                   circle(orig_img, palm_center, 3, Scalar(255));
	       }

               // Output debug numbers
               //
               cout << "tip=(" << tip_center.x << "," << tip_center.y << ") "
                    << "tip depth=" << frm->depth[frm->size.width*tip.y+tip.x] << " "
                    << "palm=(" << palm_center.x << "," << palm_center.y << ") "
                    << "palm depth=" << frm->depth[frm->size.width*palm.y+palm.x] << " "
                    << "click_dist=" << click_dist << endl;
            }

            int mid = (frm->size.width*frm->size.height + frm->size.width)/2;
            cout << "center amp=" << frm->amplitude[mid] << " "
                 << "center depth=" << frm->depth[mid] << endl;
       
            // Show the images
            //
            imshow("Filtered", filter_img);
            imshow("Original", m->_ampGain*orig_img);
            imshow("DepthThresh", depth_img);
            imshow("AmpThresh", amp_img);

            delete depth;  
            delete orig;   
         }  // if (debugging())

         qFrame.pop_front();
      } // frame not empty
  
      waitKey(m->_loopDelay);
      
   } // while (m->_isRunning)

err_exit1:
   cout << "Exit 1 ... " << endl;
   m->_depthCamera->stop();

err_exit2:
   cout << "Exit 2..." << endl;
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


