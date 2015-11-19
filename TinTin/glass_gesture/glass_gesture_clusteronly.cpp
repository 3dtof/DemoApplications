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

#define XDIM		160	
#define YDIM		120	
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
  //logger.setDefaultLogLevel(LOG_INFO);
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
  depthCamera->setFrameSize(real);  
  depthCamera->start();

  bool done = false;
  
  namedWindow( "Amplitude", WINDOW_NORMAL );
  namedWindow( "Color", WINDOW_NORMAL );
  namedWindow( "Morph", WINDOW_NORMAL );
  namedWindow( "BW_Image", WINDOW_NORMAL );


  /// Create Trackbar to select Morphology operation
  createTrackbar("Operator:\n 0: Opening - 1: Closing \n 2: Gradient - 3: Top Hat \n 4: Black Hat", 
                 "Morph", &morph_operator, max_operator, NULL );

  /// Create Trackbar to select kernel type
  createTrackbar( "Element:\n 0: Rect - 1: Cross - 2: Ellipse", "Morph",
                 &morph_elem, max_elem, NULL);

  /// Create Trackbar to choose kernel size
  createTrackbar( "Kernel size:\n 2n +1", "Morph",
                 &morph_size, max_kernel_size, NULL);

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
            hand.depth.push_back((frm->depth[i] < proximity) ? frm->depth[i] : 0.0);
            hand.amplitude.push_back((frm->depth[i] < proximity) ? ampGain : 0.0);
         }      

         // Create viewable images
         //
         unsigned char *color = FloatImageUtils::getVisualImage(hand.depth.data(), 
                                                              hand.size.width, hand.size.height, 
                                                              0, MAX_AMPLITUDE,false);  
         unsigned char *grey = FloatImageUtils::getVisualImage(frm->amplitude.data(), 
                                                              frm->size.width, frm->size.height, 
                                                              0, MAX_AMPLITUDE,true);  
         Mat grey_img  = Mat(frm->size.height, frm->size.width, CV_8UC3, grey);
         Mat color_img = Mat(hand.size.height, hand.size.width, CV_8UC3, color);
         Mat amp_img   = Mat(hand.size.height, hand.size.width, CV_32FC1, hand.amplitude.data());
         Mat morph_img = Mat(hand.size.height, hand.size.width, CV_32FC1, Scalar(0.0));

#if 0
         // Apply morphological operators
         //
         int operation = morph_operator + 2;
         Mat element = getStructuringElement( morph_elem, Size( 2*morph_size + 1, 2*morph_size+1 ), 
                                       cv::Point( morph_size, morph_size ) );
         morphologyEx( amp_img, morph_img, operation, element );

         // Apply clustering
         //
         cmap.scan(morph_img);
#else
         cmap.scan(amp_img);
#endif
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

         // Find the finger tip, which should be the most up-left point
         // in the cluster and draw the bounding box and circle around the tip
         //
         POINT tip;
         if (cmap.getClusters().size() > 0) {
	    for (int i=0; i < cmap.getClusters()[max_cluster_id].getPoints().size(); i++) {
	       tip = cmap.getClusters()[max_cluster_id].getPoints()[i];
	       if (tip.y <= cmap.getClusters()[max_cluster_id].getMin().y)
	          break;
	    }

            // Draw bounding box and tip
            if (max_area > area) {
          	cv::Point Pmax = cv::Point(cmap.getClusters()[max_cluster_id].getMax().x, 
                                    cmap.getClusters()[max_cluster_id].getMax().y);
         	cv::Point Pmin = cv::Point(cmap.getClusters()[max_cluster_id].getMin().x, 
                                    cmap.getClusters()[max_cluster_id].getMin().y);
         	rectangle(grey_img, Pmin, Pmax, Scalar(255));
                cv::Point center = cv::Point(tip.x, tip.y);
                circle(grey_img, center, 10, Scalar(255));
	    }
         }
         imshow("BW_Image", grey_img);
         imshow("Amplitude", amp_img); 
         imshow("Color", color_img);     
         imshow("Morph", morph_img);

         delete color;  
         delete grey;      

         cout << "center amp =" << frm->amplitude[frm->size.width*(frm->size.height)/2]
              << " center depth =" << frm->depth[frm->size.width*(frm->size.height)/2]<< endl;

         qFrame.pop_front();
      }
  
      if (waitKey(30) >= 0) done = true;
      
  } // end while 

  depthCamera->stop();
  return 0;
}
