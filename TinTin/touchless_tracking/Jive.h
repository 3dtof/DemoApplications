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
#include "FakeMouse.h"

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
   Mat _aMap, _aBkgMap, _aFgMap, _aPrevMap;
   Mat _zMap, _zBkgMap, _zFgMap, _zPrevMap;
   Mat _bMap, _drawing;

   bool _bkgUpdated;
   bool _bButtonDown;
   XYZIPointCloudFrame _prevFrame;
   float _zTrigger;
   float _zHighThresh;
   float _zLowThresh;
   float _aGain;
   float _minContourSize;
   float _minConvDefDepth;
   float _Xmin, _Xmax;
   float _Ymin, _Ymax;
   float _Xcur, _Ycur;
   cv::Point _palmCenter[2];
   float _palmRadius[2];
   cv::Point _handTip[2];
   int _leftHand, _rightHand;
   int _numHands;
   int _handContour[2];
   vector< vector<cv::Point> > _contours;

   // Parameter map:  <ptr, precision, max>
   map< std::string, std::tuple<float*, int, float> > _param;
   map< std::string, Mat *> _images;
   vector<std::string> _mapsToDisplay;

   // Display and controls
   vector<int> _sliderPos;

   // Mouse
   FakeMouse mouse;

private:
   void findForeground(float zLowThr, float aHighThr, Mat &fgMap);
   void updateMaps(Frame *frame);
   void morphClean(Mat &in, Mat &out);
   void initControls();
   void displayMaps();
   void cropMaps(Mat &m, int xmin, int xmax, int ymin, int ymax);
   bool findContourCenter(vector<cv::Point> &contour, cv::Point &center, float &radius);
   bool findHandTips(vector< vector<cv::Point> > &contours);
   
};

#endif // __JIVE_H__
/*! @} */

