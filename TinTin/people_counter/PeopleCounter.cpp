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

#define XDIM		80 
#define YDIM		60	
#define SCALE		8
#define MAX_AMPLITUDE   5

deque < DepthFrame > qFrame;
DepthFrame *frm;
float ampGain = 200;
 
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

float bilinearInterpolation(float q11, float q12, float q21, float q22, float x1, float x2, float y1, float y2, float x, float y) 
{
    float x2x1, y2y1, x2x, y2y, yy1, xx1;
    x2x1 = x2 - x1;
    y2y1 = y2 - y1;
    x2x = x2 - x;
    y2y = y2 - y;
    yy1 = y - y1;
    xx1 = x - x1;
    return 1.0 / (x2x1 * y2y1) * (
        q11 * x2x * y2y +
        q21 * xx1 * y2y +
        q12 * x2x * yy1 +
        q22 * xx1 * yy1
    );
}

void scaleImage(DepthFrame *src, DepthFrame *dst, int scale)
{
   float q11, q12, q21, q22;
   dst->size.width = src->size.width * scale;
   dst->size.height = src->size.height * scale;
   dst->depth.clear();
   dst->amplitude.clear();
   for (int i = 0; i < dst->size.width*dst->size.height; i++) {
      dst->depth.push_back(0);
      dst->amplitude.push_back(0);
   }

   for (int j=0; j < dst->size.height; j++) {
      for (int i=0; i < dst->size.width; i++) {
         int ii = i/scale;
         int jj = j/scale;

         q11 = src->depth[jj*src->size.width+ii]; 
         q12 = src->depth[jj*src->size.width+(ii+1)]; 
         q22 = src->depth[(jj+1)*src->size.width+(ii+1)]; 
         q21 = src->depth[(jj+1)*src->size.width+ii]; 
         dst->depth[j*dst->size.width+i] = bilinearInterpolation(q11, q12, q21, q22, 
                                 ii*scale, (ii+1)*scale, jj*scale, (jj+1)*scale, i, j);
      
         q11 = src->amplitude[jj*src->size.width+ii]; 
         q12 = src->amplitude[jj*src->size.width+(ii+1)]; 
         q22 = src->amplitude[(jj+1)*src->size.width+(ii+1)]; 
         q21 = src->amplitude[(jj+1)*src->size.width+ii]; 
         dst->amplitude[j*dst->size.width+i] = ampGain*bilinearInterpolation(q11, q12, q21, q22, 
                                 ii*scale, (ii+1)*scale, jj*scale, (jj+1)*scale, i, j);
 
      }
   }
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
  ClusterMap cmap(0.8, 0.2, 3);
  DepthFrame d, bkgnd;
  int scale = SCALE;
  FrameSize scaled, real;

  real.width = XDIM;
  real.height = YDIM;
  scaled.width = XDIM*scale;
  scaled.height = YDIM*scale;

  d.size = bkgnd.size = real;
 
 
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
  iv->setSize(scaled.width, scaled.height); 

  depthCamera->setFrameSize(real);
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
            DepthFrame scaled_d;
            scaleImage(&bkgnd, &scaled_d, scale);
	    unsigned char *rgb = FloatImageUtils::getVisualImage(scaled_d.amplitude.data(), 
                                                                 scaled_d.size.width, scaled_d.size.height, 
                                                                 0, MAX_AMPLITUDE,true);
            iv->showRGBImage(rgb, scaled_d.size.width, scaled_d.size.height);     
            delete rgb;        
            iv->removeLayer("rectangles");
         }
         else {
            d.depth.clear();          
            d.amplitude = frm->amplitude;
            for (int i = 0; i < frm->depth.size(); i++) {
               float diff = bkgnd.depth[i] - frm->depth[i];
               d.depth.push_back((diff < 0) ? 0 : diff);
            }

            cmap.Scan(d);

            DepthFrame scaled_d;
            scaleImage(&d, &scaled_d, scale);
	    unsigned char *rgb = FloatImageUtils::getVisualImage(scaled_d.amplitude.data(), 
                                                                 scaled_d.size.width, scaled_d.size.height, 
                                                                 0, MAX_AMPLITUDE,true);
            iv->showRGBImage(rgb, scaled_d.size.width, scaled_d.size.height);     
            delete rgb;        

            iv->removeLayer("rectangles");

            for (int i = 0; i < cmap.GetClusters().size(); i++) {
               if (cmap.GetClusters()[i].GetArea() > 100) {
                  iv->addRectangle(cmap.GetClusters()[i].GetMin().x*scale, 
                                   cmap.GetClusters()[i].GetMax().x*scale,
                                   (real.height-cmap.GetClusters()[i].GetMin().y)*scale, 
                                   (real.height-cmap.GetClusters()[i].GetMax().y)*scale,
                                   1, 0, 0);
               }
            }
         }  

         qFrame.pop_front();
      }
  } // end while 

  delete iv;
  depthCamera->stop();
  return 0;
}
