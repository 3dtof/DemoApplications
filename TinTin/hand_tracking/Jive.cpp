/*! 
 * ============================================================================
 *
 * @addtogroup		jive
 * @{
 *
 * @file		jive.cpp
 * @version		1.0
 * @date		12/14/2015
 *
 * @note		Hand tracking class
 * 
 * Copyright(c) 2007-2012 Texas Instruments Corporation, All Rights Reserved.
 * TI makes NO WARRANTY as to software products, which are supplied "AS-IS"
 *
 * ============================================================================
 */
#define __JIVE_CPP__
#include "Jive.h"
#include <climits>
#include <algorithm>

Jive::Jive()
{
   init();
}

Jive::Jive(int w, int h) : TOFApp(w, h)
{
   init();
}

Jive::~Jive()
{
}

void Jive::init()
{
   _ampGain = 10.0;
   _depthClip = 0.6;
   _ampClip = 0.01;
   _illum_power = 100U;
   _intg = 20U;
   setLoopDelay(33);
   namedWindow( "Binary", WINDOW_NORMAL );
   namedWindow( "Contours", WINDOW_NORMAL );
}


void Jive::clipBackground(DepthFrame &in, DepthFrame &out)
{
   out.size = in.size;
   out.depth.clear();          
   out.amplitude.clear();
   for (int i = 0; i < in.depth.size(); i++) {
      out.depth.push_back((in.depth[i] < _depthClip && in.amplitude[i] > _ampClip) ? in.depth[i] : 0.0);
      out.amplitude.push_back((in.depth[i] < _depthClip && in.amplitude[i] > _ampClip) ? 1.0 : 0.0);
   }
}

bool Jive::findPalmCenter(vector<cv::Point> &contour, cv::Point &center, float &radius)
{
   bool rc = false;
   float m10, m01, m00;

   m10 = m01 = m00 = 0;
   for (int x=0; x < _binaryMat.cols; x++) {
      for (int y=0; y < _binaryMat.rows; y++) {
         cv::Point p = cv::Point(x, y);
         if (pointPolygonTest(contour, p, false) >= 0) {
            float val = _binaryMat.at<float>(y, x);
            m00 += val;
            m10 += x*val;
            m01 += y*val;
         }
      }
   }
   if (m00 > 0.0) { 
      center = cv::Point((int)(m10/m00), (int)(m01/m00));
      float r2 = 2*getDim().width*getDim().width;
      for (int i=0; i<contour.size(); i++) {
         float x2 = (float)contour[i].x - center.x;
         x2 *= x2;
         float y2 = (float)contour[i].y - center.y;
         y2 *= y2;
         if (r2 > x2+y2)
            r2 = x2+y2;    
      }
      radius = sqrt(r2);
      rc = true;
   }
   return rc;
}


void Jive::findKeyPoints(vector<cv::Point> &contour, vector<int> &hulls, vector<int> &defects, int depth)
{  
   vector<Vec4i> convDef;

   hulls.clear();
   defects.clear();   
   convexHull(Mat(contour), hulls, false ); 
   convexityDefects(contour, hulls, convDef);
   for (int k=1; k<convDef.size(); k++) {  // first defect is false  
      if (convDef[k][3] > depth*256) {
         int ind = convDef[k][2];
         defects.push_back(ind);
      }
   }
}

int Jive::findMedianHull(vector<cv::Point> &contour, vector<int> hull) 
{
   cv::Point centroid(0,0);
   for (int i = 0; i < hull.size(); i++) {
      centroid = centroid + contour[hull[i]];
   }
   centroid.x = centroid.x / hull.size();
   centroid.y = centroid.y / hull.size();

   int rc = 0;
   double minDist = 1e6;
   for (int i = 0; i < hull.size(); i++) {
      double dist = cv::norm(contour[hull[i]]-centroid);
      minDist = (dist < minDist) ? dist : minDist;
      rc = hull[i];
   } 
   return rc;
}


void Jive::distillHullPoints(vector<cv::Point> &contour, vector<int> &hull, vector<int>&rhull, float maxDist)
{
   int start = 0;
   cv::Point p1, p2;
   vector<int> temp;

   rhull.clear();
   if (hull.size() > 2) {
      // Find the starting point
      for (int k=0; k < hull.size(); k++) {
         int n = (k+1 >= hull.size()) ? k+1-hull.size() : k+1;
         p1 = contour[hull[k]]; 
         p2 = contour[hull[n]];
         if (cv::norm(p1-p2) > (double)maxDist) {
            start = n;
            temp.clear();
            temp.push_back(hull[n]);
            break;
         }
      }
   
      // Walk around the entire hull points once from the starting point
      if (temp.size() > 0) {
         for (int i=start; i < start+hull.size(); i++) {
            int k = (i >= hull.size()) ? i-hull.size() : i;
            int n = (i+1 >= hull.size()) ? i+1-hull.size() : i+1;
            p1 = contour[hull[k]];
            p2 = contour[hull[n]];
            if (cv::norm(p1-p2) > (double)maxDist) {
               int m = findMedianHull(contour, temp);
               rhull.push_back(m);
               temp.clear();
            }
            temp.push_back(hull[n]);
         }
      }
   }
}


void Jive::kCurvature(vector<cv::Point> &contour, vector<int> &hull, int kmin, int kmax, 
                      double ang, vector<int> &tips)
{
   tips.clear();
   for (int i=0; i < hull.size(); i++) {
      int n = hull[i];
      cv::Point p = contour[n];
      bool done = false;
      for (int k = 1; k < kmax && !done; k++) {
         cv::Point p1 = (n-k < 0) ? contour[contour.size()+(n-k)] : contour[n-k];
         cv::Point p2 = (n+k > contour.size()) ? contour[n+k-contour.size()] : contour[n+k];
         double dval = ((p1.x-p.x)*(p2.x-p.x)+(p1.y-p.y)*(p2.y-p.y)) 
                     / (norm(Mat(p1), cv::Mat(p))*norm(Mat(p2), Mat(p)));
         double a = acos(dval)*180.0/3.1415926;
         if (a < ang && k >= kmin) {
            tips.push_back(n);
            done = true;
         } 
      } 
   }
}

float Jive::depthAt(DepthFrame frm, cv::Point p)
{
   return frm.depth[p.y*frm.size.height+p.x];
}


int Jive::adjPix(int pix)
{
   int out = pix*getDim().width/TOF_WIDTH;
   return (out<=0)?1:out;
}


void Jive::update(DepthFrame *frm)
{
   DepthFrame hand;
   vector< vector<cv::Point> > contours;
   vector<Vec4i> hierarchy;
   RNG rng(12345);   
   Mat gray;

   // Create silhouette
   clipBackground(*frm, hand);
   _binaryMat = Mat(hand.size.height, hand.size.width, CV_32FC1, hand.amplitude.data());  
   _binaryMat.convertTo(gray, CV_8U, 255.0);
   threshold(gray, gray, 200, 255, THRESH_BINARY);
   findContours(gray, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0,0));
   Mat drawing = Mat::zeros( gray.size(), CV_8UC3 );

   int hands = 0;
   if (contours.size() > 0) {

      vector<int> hull, rhull;
      vector<int> defect;
      vector<int> tips;
      cv::Point center = cv::Point(0,0);
      float radius = 0;

      for( int i = 0; i < contours.size(); i++ ) {  
         if (contourArea(contours[i]) > adjPix(1000)) {
            hands++;
            findPalmCenter(contours[i], center, radius);
            float palm_depth = depthAt(*frm, center);
            cv::circle(drawing, center, (int)radius, Scalar(0, 0, 255), 1);

            findKeyPoints(contours[i], hull, defect, adjPix(15));
            distillHullPoints(contours[i], hull, rhull, adjPix(10));
            for (int k=0; k < rhull.size(); k++) {
               if (k > 0)
                  cv::line(drawing, contours[i][rhull[k-1]], contours[i][rhull[k]], Scalar(255, 0, 0), 1);
            }
#if 1
            for (int k=0; k < defect.size(); k++) 
               cv::circle(drawing, contours[i][defect[k]], adjPix(3), Scalar(255, 0, 255), -1);
#endif
            vector<int> temp;
            vector<cv::Point> tips;
            kCurvature(contours[i], rhull, adjPix(5), adjPix(25), 60.0, temp);
            for (int k=0; k < temp.size(); k++) {
          //     if (depthAt(*frm, contours[i][temp[k]]) < palm_depth) 
                  tips.push_back(contours[i][temp[k]]);
            }
            for (int k=0; k < tips.size(); k++) {
               cv::circle(drawing, tips[k], adjPix(4), Scalar(0, 0, 255), -1);
            }

#if 0
            cout << "palm_size= " << palm_depth << " ";
            for (int k=0; k < temp.size(); k++) 
               cout << "temp[" << k << "]=" << depthAt(*frm, contours[i][temp[k]]) << " ";
            cout << endl;
#endif
         }
         drawContours( drawing, contours, i, Scalar(0, 255, 0), 1, 8, vector<Vec4i>(), 0, cv::Point() );  
      }
   }

   imshow("Binary", _binaryMat);
   imshow("Contours", drawing);
}




#undef __JIVE_CPP__
/*! @} */
