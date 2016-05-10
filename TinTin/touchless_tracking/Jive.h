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
   enum Gesture
   {
      GESTURE_NULL = 0,
      GESTURE_1H1F,
      GESTURE_1H2F,
      GESTURE_1H3F,
      GESTURE_1H4F,
      GESTURE_1H5F,
      GESTURE_2H1F,
      GESTURE_2H2F,
      GESTURE_2H3F,
      GESTURE_2H4F,
      GESTURE_2H5F,
   };

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
   Mat _aMap, _aBkgMap, _aFgMap, _aHeatMap, _aDiffMap, _aPrevMap;
   Mat _zMap, _zBkgMap, _zFgMap, _zHeatMap, _zDiffMap, _zPrevMap;
   Mat _bMap, _drawing;

   bool _bkgUpdated;
   XYZIPointCloudFrame _prevFrame;
   float _aHeatMapThresh;
   float _zHeatMapThresh;
   float _aDiffMapThresh;
   float _zDiffMapThresh;
   float _heatMapCoef;
   float _zTrigger;
   float _zHighThresh;
   float _zLowThresh;
   float _aGain;
   float _minContourSize;
   float _minConvDefDepth;
   float _Xmin, _Xmax;
   float _Ymin, _Ymax;
   cv::Point _palmCenter[MAX_HANDS];
   float _palmRadius[MAX_HANDS];
   vector<int> _tips[MAX_HANDS];
   float _maxAngle;
   int _movementCount;
   float _separation;


   // Parameter map:  <ptr, precision, max>
   map< std::string, std::tuple<float*, int, float> > _param;
   map< std::string, Mat *> _images;
   vector<std::string> _mapsToDisplay;

   // Display and controls
   vector<int> _sliderPos;

private:
   void findForeground(float zLowThr, float aHighThr, Mat &fgMap);
   void updateMaps(Frame *frame);
   void morphClean(Mat &in, Mat &out);
   void initControls();
   void displayMaps();
   bool noMovement(int t);
   void cropMaps(Mat &m, int xmin, int xmax, int ymin, int ymax);
   bool findGesture(vector< vector<cv::Point> > &contours, enum Gesture &gesture, int *value);
   bool findPalmCenter(vector<cv::Point> &contour, cv::Point &center, float &radius);
   void findKCurv(vector<cv::Point> &contour, vector<int> &hull, int kmin, int kmax, double ang, vector<int> &tips);
   bool findTips(vector<cv::Point> &contour, vector<int> &hulls, vector<int> &tips, float maxDist);
   int findCenterPoint(vector<cv::Point> &contour, vector<int> &cluster);
   int adjPix(int pix);
};

#endif // __JIVE_H__
/*! @} */

