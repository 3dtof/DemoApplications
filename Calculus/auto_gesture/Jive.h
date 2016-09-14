/*! 
 * ============================================================================
 *
 * @addtogroup		Jive
 * @{
 *
 * @file		Jive.h
 * @version		2.0
 * @date		4/22/2016
 *
 * @note		Touchless hand-tracking class
 * 
 * Copyright(c) 2015-2016 Texas Instruments Corporation, All Rights Reserved.
 * TI makes NO WARRANTY as to software products, which are supplied "AS-IS"
 *
 * ============================================================================
 */
#include "TOFApp.h"
#include <math.h>
#include <map>
#include <tuple>
#include <string>

#ifndef __JIVE_H__
#define __JIVE_H__

#define MAX_HANDS	2

class Jive : public TOFApp
{
public:
   Jive(int w, int h);
   void update(Frame *frm);
   bool getParamList(vector<std::string> &s);
   map< std::string, std::tuple<float*, int, float> > &getParamMap();
   map< std::string, cv::Mat *> &getImageMap();
   void addMapToDisplay(std::string name);
   void initDisplays();
   void sampleBackground();

private:
   Mat _aMap, _aFgMap;
   Mat _zMap, _zFgMap;
   Mat _bMap, _drawing;

   XYZIPointCloudFrame _prevFrame;
   float _aThresh;
   float _zThresh;
   float _aGain;
   float _minContourSize;
   float _minConvDefDepth;
   float _Xmin, _Xmax;
   float _Ymin, _Ymax;
   cv::Point _palmCenter[2];
   float _palmDepth[2];
   float _palmRadius[2];
   vector<cv::Point> _fingerTips[2];
   vector<int> _hulls[2];
   vector<int> _defects[2];
   int _leftHand, _rightHand;
   int _numHands;
   int _handContour[2];
   vector< vector<cv::Point> > _contours;
   float _avg[2];
   vector<cv::Point2d> _eigenVec[2];
   vector<double> _eigenVal[2];
   int _wristStart[2], _wristEnd[2];
   cv::Point _major[2], _minor[2];

   // Parameter map:  <ptr, precision, max>
   map< std::string, std::tuple<float*, int, float> > _param;
   map< std::string, Mat *> _images;
   vector<std::string> _mapsToDisplay;

   // Display and controls
   vector<int> _sliderPos;

private:
   int  adjPix(int pix);
   void findForeground(float zThr, float aThr, Mat &fgMap);
   void updateMaps(Frame *frame);
   void morphClean(Mat &in, Mat &out);
   void initControls();
   void displayMaps();
   void cropMaps(Mat &m, int xmin, int xmax, int ymin, int ymax);
   bool findPalmCenter(vector<cv::Point> &contour, cv::Point &center, float &radius);
   bool findFingertips(vector< vector<cv::Point> > &contours);
   void findKeyPoints(vector<cv::Point> &contour, vector<int> &hulls, vector<int> &defects, int depth);
   int  findMedianHull(vector<cv::Point> &contour, vector<int> hull);
   void distillHullPoints(vector<cv::Point> &contour, vector<int> &hull, vector<int>&rhull, float maxDist);
   void removeBorderPoints(vector<cv::Point> &contour, vector<int> &hull);
   void kCurvature(vector<cv::Point> &contour, vector<int> &hull, int kmin, int kmax, double ang, vector<int> &tips);
   double findAngle(cv::Point p, cv::Point p1, cv::Point p2);
   double getOrientation(vector<cv::Point> &pts, cv::Point &center, vector<cv::Point2d> &e_vec, vector<double> &e_val);
   double distPoint2Line(cv::Point p1, cv::Point p2, cv::Point p);
   void findWrists(vector<cv::Point> &contour, cv::Point palm, double radius, int &start, int &end);
};

#endif // __JIVE_H__
/*! @} */

