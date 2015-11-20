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
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "Cluster.h"


using namespace std;
using namespace Voxel;
using namespace cv;
using namespace pcl::visualization;

#define XDIM		80	
#define YDIM		60	
#define MAX_AMPLITUDE   2.0

deque < DepthFrame > qFrame;


int morph_elem = 0;
int morph_size = 4;
int morph_operator = 0;
int const max_operator = 1;
int const max_elem = 2;
int const max_kernel_size = 21;



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
  logger.setDefaultLogLevel(LOG_INFO);
  CameraSystem sys;
  DepthCameraPtr depthCamera;
  FrameSize real;
  DepthFrame *frm, hand;
  float ampGain = 200.0;
  float proximity = 0.5;
  float area = 0;
  uint illum_power = 48U;
  ClusterMap cmap(0.8, 0.1, 3);


  real.width = XDIM;
  real.height = YDIM;

  hand.size = real;

  if (argc != 5) {
     cout << "Usage: glass_gesture <ampGain> <proximity> <illum power> <area> " << endl;
     return -1;
  }
  
  ampGain = atof(argv[1]);
  proximity = atof(argv[2]);
  illum_power = atoi(argv[3]);
  area = atof(argv[4]);

  cout << "ampGain= " << ampGain << " "
       << "proximity= " << proximity << " "
       << "illum_power = " << illum_power 
       << "area = " << area << endl;
 
  const Vector<DevicePtr> &devices = sys.scan();
  
  if(devices.size() > 0) {
    depthCamera = sys.connect(devices[0]);  // Connect to first available device
  }
  else
  {
    cerr << "Error: Could not find a compatible device." << endl;
    return -1;
  }
  
  if(!depthCamera)
  {
    cerr << "Error: Could not open a depth camera." << endl;
    return -1;
  }
 
  if (!depthCamera->isInitialized())   
  {
     cerr << "Error: Depth camera not initialized " << endl;
     return -1;
  }
 
  cout << "Successfully loaded depth camera " << endl; 
 
 
  depthCamera->registerCallback(DepthCamera::FRAME_DEPTH_FRAME, frameCallback);
  depthCamera->setFrameSize(real);  
  cout << "set intg = " << depthCamera->set("intg_duty_cycle", 1U) << endl;
  depthCamera->start();

  bool done = false;
  
  namedWindow( "AmpThresh", WINDOW_NORMAL );
  namedWindow( "DepthThresh", WINDOW_NORMAL );
  namedWindow( "Original", WINDOW_NORMAL );
  namedWindow( "Filtered", WINDOW_NORMAL );

  while (!done) {

      char key = getkey();
      
      if ('q' == key) {
         done = true;
         cout << "Exit." << endl;
      }

      if (!qFrame.empty()) {

         frm = &qFrame.front(); 

         // Clip background (objects > proximity) 
         //
         hand.depth.clear();          
         hand.amplitude.clear();
         for (int i = 0; i < frm->depth.size(); i++) {
            hand.depth.push_back((frm->depth[i] < proximity && frm->amplitude[i] > 0.01) ? frm->depth[i] : 0.0);
            hand.amplitude.push_back((frm->depth[i] < proximity && frm->amplitude[i] > 0.01) ? ampGain : 0.0);
         }      

         // Create viewable images
         //
         unsigned char *depth = FloatImageUtils::getVisualImage(hand.depth.data(), 
                                                              hand.size.width, hand.size.height, 
                                                              0, MAX_AMPLITUDE,false);  
         unsigned char *orig = FloatImageUtils::getVisualImage(frm->amplitude.data(), 
                                                              frm->size.width, frm->size.height, 
                                                              0, MAX_AMPLITUDE,true);  
         Mat orig_img  = Mat(frm->size.height, frm->size.width, CV_8UC3, orig);
         Mat depth_img = Mat(hand.size.height, hand.size.width, CV_8UC3, depth);
         Mat amp_img   = Mat(hand.size.height, hand.size.width, CV_32FC1, hand.amplitude.data());
         Mat filter_img = Mat(hand.size.height, hand.size.width, CV_32FC1, Scalar(0));

         // Perform clustering
         //
         cmap.scan(amp_img);

         // Find the largest cluster, which should be the hand
         //
         int max_cluster_id = 0;
         float max_area = 0;
         for (int i=0; i < cmap.getClusters().size(); i++) {
            if (cmap.getClusters()[i].getArea() > max_area) {
               max_area = cmap.getClusters()[i].getArea();
               max_cluster_id = i;
            }
         }

         // Find the finger tip
         //
         POINT tip = POINT(amp_img.cols,amp_img.rows,0);
	 float dist2, max_dist2 = 0.0;

         if (cmap.getClusters().size() > 0) {

            // Find tip which point in cluster farthest from bottom-right corner
            //
	    for (int i=0; i < cmap.getClusters()[max_cluster_id].getPoints().size(); i++) {
	       POINT p = cmap.getClusters()[max_cluster_id].getPoints()[i];
               filter_img.at<float>(p.y, p.x) = p.z;
	       if ((p.y <= cmap.getClusters()[max_cluster_id].getMin().y) ||
                   (p.x <= cmap.getClusters()[max_cluster_id].getMin().x)) {
                  dist2 = (filter_img.cols-p.x)*(filter_img.cols-p.x)
                        + (filter_img.rows-p.y)*(filter_img.rows-p.y);
                  if (dist2 > max_dist2) {
                     max_dist2 = dist2;
                     tip = p;
                  }
               }	          
	    }

            POINT p = cmap.getClusters()[max_cluster_id].getCentroid();
            cv::Point palm = cv::Point(p.x, p.y);
            cv::Point finger_tip = cv::Point(tip.x, tip.y);

            float click_dist = depth_img.at<float>(finger_tip) - depth_img.at<float>(palm);
            

            // Draw bounding box and tip
            if (max_area > area) {
          	cv::Point Pmax = cv::Point(cmap.getClusters()[max_cluster_id].getMax().x, 
                                    cmap.getClusters()[max_cluster_id].getMax().y);
         	cv::Point Pmin = cv::Point(cmap.getClusters()[max_cluster_id].getMin().x, 
                                    cmap.getClusters()[max_cluster_id].getMin().y);
         	rectangle(orig_img, Pmin, Pmax, Scalar(255));
                circle(orig_img, finger_tip, 3, Scalar(255));
                circle(orig_img, palm, 3, Scalar(255));
	    }

            cout << "tip = (" << tip.x << "," << tip.y << ") "
                 << "tip depth = " << frm->depth[frm->size.width*tip.y+tip.x] 
                 << "click_dist2 = " << max_dist2 << endl;
         
            if (max_dist2 < 2000) 
                cout << " ======== mouse down ========" << endl;
         }

         int mid = (frm->size.width*frm->size.height + frm->size.width)/2;
         cout << "center amp =" << frm->amplitude[mid] << " "
              << "center depth =" << frm->depth[mid] 
              << "depth*amp = " << frm->depth[mid]*frm->amplitude[mid] << endl;

         imshow("Original", ampGain*orig_img);
         imshow("AmpThresh", amp_img); 
         imshow("DepthThresh", depth_img);     
         imshow("Filtered", filter_img);

         delete depth;  
         delete orig;      

         qFrame.pop_front();
      }
  
      if (waitKey(30) >= 0) done = true;
      
  } // end while 

  depthCamera->stop();
  return 0;
}
