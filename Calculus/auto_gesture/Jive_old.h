/*! 
 * ============================================================================
 *
 * @addtogroup		jive
 * @{
 *
 * @file		jive.h
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
#include "TOFApp.h"
#include <math.h>

#ifndef __JIVE_H__
#define __JIVE_H__

class Jive : public TOFApp
{
public:
   Jive();
   Jive(int w, int h);
   ~Jive();
   void init();
   void update(DepthFrame *frm);

private:
   Mat _binaryMat;
   float _ampGain;
   float _depthClip;
   float _ampClip;
   uint _illum_power;
   uint _intg;

private:
   void clipBackground(DepthFrame &in, DepthFrame &out);
   bool findPalmCenter(vector<cv::Point> &contour, cv::Point &center, float &radius);
   void findKeyPoints(vector<cv::Point> &contour, vector<int> &hull, vector<int> &defects, int depth);
   void distillHullPoints(vector<cv::Point> &contour, vector<int> &hull, vector<int>&rhull, float maxDist);
   int  findMedianHull(vector<cv::Point> &contour, vector<int> hull);
   void kCurvature(vector<cv::Point> &contour, vector<int> &hull, int kmin, int kmax, double ang, vector<int> &tips);
   float depthAt(DepthFrame frm, cv::Point p);
   int  adjPix(int pix);
};

#endif // __JIVE_H__
/*! @} */

