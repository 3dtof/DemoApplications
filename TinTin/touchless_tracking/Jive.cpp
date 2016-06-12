/*! 
 * ==========================================================================================
 *
 * @addtogroup		Jive	
 * @{
 *
 * @file		Jive.cpp
 * @version		1.0
 * @date		1/7/2016
 *
 * @note		People tracking class
 * 
 * Copyright(c) 20015-2016 Texas Instruments Corporation, All Rights Reserved.q
 * TI makes NO WARRANTY as to software products, which are supplied "AS-IS"
 *
 * ==========================================================================================
 */
#define __JIVE_CPP__
#include "Jive.h"
#include <climits>
#include <algorithm>

/*!
 *===========================================================================================
 * @brief   Initialize control window based on Jive parameters
 *===========================================================================================
 */
static void onTrackBar(int barVal, void *p)
{
   std::tuple<float*,int, float> *t = (std::tuple<float*,int, float> *)p;
   float *fval = std::get<0>(*t);
   *fval = (float)barVal/(float)std::get<1>(*t);
}


Jive::Jive(int w, int h) : TOFApp(w, h)
{
   _zMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _aMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _zBkgMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _aBkgMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _zFgMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _aFgMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _bMap = Mat::zeros(getDim().height, getDim().width, CV_8U);
   _drawing = Mat::zeros(getDim().height, getDim().width, CV_8UC3 );

   _bkgUpdated = false;
   _zTrigger = 0.03;
   _zLowThresh = 0.01;
   _zHighThresh = 1;
   _aGain = 10;
   _minContourSize = 100;
   _Xmin = 66;
   _Xmax = 279;
   _Ymin = 51;
   _Ymax = 171;
   _Xcur = 0;
   _Ycur = 0;
   _bButtonDown = false;
   

   // Setup parameter map
   _param["zTrigger"] = std::make_tuple(&_zTrigger, 1000, 2);
   _param["zLowThresh"] = std::make_tuple(&_zLowThresh, 1000, 2);
   _param["zHighThresh"] = std::make_tuple(&_zHighThresh, 1000, 2);
   _param["aGain"] = std::make_tuple(&_aGain, 10, 200);
   _param["minContourSize"] = std::make_tuple(&_minContourSize, 1, 10000);
   _param["Xmin"] = std::make_tuple(&_Xmin, 1, 319);
   _param["Xmax"] = std::make_tuple(&_Xmax, 1, 319);
   _param["Ymin"] = std::make_tuple(&_Ymin, 1, 239);
   _param["Ymax"] = std::make_tuple(&_Ymax, 1, 239);
   _param["Xcur"] = std::make_tuple(&_Xcur, 1, 300);
   _param["Ycur"] = std::make_tuple(&_Ycur, 1, 300);

   // Setup image map
   _images["aMap"] = &_aMap;
   _images["aBkgMap"] = &_aBkgMap;
   _images["aFgMap"] = &_aFgMap;
   _images["zMap"] = &_zMap;
   _images["zBkgMap"] = &_zBkgMap;
   _images["zFgMap"] = &_zFgMap;
   _images["bMap"] = &_bMap;
   _images["drawing"] = &_drawing;
}


/*!
 *===========================================================================================
 * @brief   Initialize control window based on Jive parameters
 *===========================================================================================
 */
void Jive::initControls()
{
   int i=0;
   _sliderPos.clear();
   for (map <std::string, std::tuple<float*,int,float> >::iterator it = _param.begin(); it != _param.end(); it++) 
   {
      std::string s = it->first;
      std::tuple<float*,int,float> *t = &_param[s];
      int maxVal = (int)floor(std::get<2>(*t)*std::get<1>(*t));
      _sliderPos.push_back((int)floor(*std::get<0>(*t)*std::get<1>(*t)));
      createTrackbar(s, "Controls", &_sliderPos.data()[i++], maxVal, onTrackBar, (void*)t);
   }
}


/*!
 *===========================================================================================
 * @brief   Initialize control window based on Jive parameters
 *===========================================================================================
 */
void Jive::addMapToDisplay(std::string name)
{
   _mapsToDisplay.push_back(name);
}



/*!
 *===========================================================================================
 * @brief   Initialize control window based on Jive parameters
 *===========================================================================================
 */
void Jive::initDisplays()
{
   if (_mapsToDisplay.size() > 0) 
   {
      namedWindow("Controls", WINDOW_NORMAL);
      initControls();
      for (int i=0; i < _mapsToDisplay.size(); i++) 
         namedWindow( _mapsToDisplay[i], WINDOW_NORMAL );
   }
}


/*!
 *===========================================================================================
 * @brief   Initialize control window based on Jive parameters
 *===========================================================================================
 */
void Jive::displayMaps()
{
   // Draw crop lines
   cv::line(_drawing, cv::Point(_Xmin, _Ymin), cv::Point(_Xmin, _Ymax), Scalar(0,0,255), 1);
   cv::line(_drawing, cv::Point(_Xmin, _Ymax), cv::Point(_Xmax, _Ymax), Scalar(0,0,255), 1);
   cv::line(_drawing, cv::Point(_Xmax, _Ymax), cv::Point(_Xmax, _Ymin), Scalar(0,0,255), 1);
   cv::line(_drawing, cv::Point(_Xmax, _Ymin), cv::Point(_Xmin, _Ymin), Scalar(0,0,255), 1);

   // Draw hand features
   if (_bkgUpdated && _numHands > 0) 
   {
      for (int i=0; i < _numHands; i++) 
      { 
         // Draw tip of hand
         Scalar color = Scalar(0,255,255);
         int sz = 4;
         if (_zBkgMap.at<float>(_handTip[i]) - _zMap.at<float>(_handTip[i]) < _zTrigger) 
         {
            sz = 10;    
         }
         cv::circle(_drawing, _handTip[i], sz, color, 1);

         // Draw palm
         cv::circle(_drawing, _palmCenter[i], (int)_palmRadius[i], Scalar(0,255,0), 1);
         
         // Draw hand
         cv::drawContours(_drawing, _contours, _handContour[i], Scalar(0, 0, 255), 0, 1, vector<Vec4i>(), 0, cv::Point() ); 

         // Draw text
         if (_numHands == 2) 
         {
            std::string s = (i == _leftHand) ? "LEFT" : "RIGHT";
            cv::putText(_drawing, s, _handTip[i], FONT_HERSHEY_COMPLEX_SMALL, 0.4, Scalar(255,255,255));
         }  
      } // for (i)

   }

   // Display all registered maps
   for (int i=0; i < _mapsToDisplay.size(); i++) 
   {
      if (_mapsToDisplay[i] == "aMap" || _mapsToDisplay[i] == "aBkgMap" 
       || _mapsToDisplay[i] == "aFgMap" || _mapsToDisplay[i] == "drawing" ) 
          imshow(_mapsToDisplay[i], *_images[_mapsToDisplay[i]]*_aGain );
      else
          imshow(_mapsToDisplay[i], *_images[_mapsToDisplay[i]] );
   }
}
 

/*!
 *===========================================================================================
 * @brief  Get the list of prestored parameter strings
 *===========================================================================================
 */
bool Jive::getParamList(vector<std::string> &s)
{
   s.clear();
   for (map <std::string, std::tuple<float*,int,float> >::iterator it = _param.begin(); it != _param.end(); it++) 
   {
      s.push_back(it->first);
   }
}


/*!
 *===========================================================================================
 * @brief  Get the list of prestored parameter strings
 *===========================================================================================
 */
map< std::string, std::tuple<float*, int, float> > &Jive::getParamMap()
{
   return _param;
}


/*!
 *===========================================================================================
 * @brief  Get the list of prestored parameter strings
 *===========================================================================================
 */
map< std::string, cv::Mat* > &Jive::getImageMap()
{
   return _images;
}

/*!
 *===========================================================================================
 * @brief   Initialize control window based on Jive parameters
 *===========================================================================================
 */
void Jive::cropMaps(Mat &m, int xmin, int xmax, int ymin, int ymax)
{
   for (int i=0; i <  getDim().height; i++)  
   {
      for (int j=0; j< getDim().width; j++) 
      {
          if ((i < ymin || i > ymax) || (j<xmin || j>xmax))
          {
             m.at<uint8_t>(i,j) = 0;
          }
      }
   }
}

/*!
 *===========================================================================================
 * @brief  Find foreground based on thresholds.  
 *===========================================================================================
 */
void Jive::sampleBackground()
{
   _zMap.copyTo(_zBkgMap);
   _aMap.copyTo(_aBkgMap);
   _bkgUpdated = true;
}


/*!
 *===========================================================================================
 * @brief  Find foreground based on thresholds. 
 *=========================================================================================== 
 */
void Jive::findForeground(float zLowThr, float zHighThr, Mat &fgMap)
{ 
   fgMap = _zBkgMap-_zMap;
   for (int i = 0; i < _zMap.rows; i++) 
   {
      for (int j = 0; j < _zMap.cols; j++) 
      {
         if (fgMap.at<float>(i,j)<zHighThr && fgMap.at<float>(i,j)>zLowThr) 
            fgMap.at<float>(i,j) = 255.0f;
         else 
            fgMap.at<float>(i,j) = 0.0f;
      }
   }
}


/*!
 *===========================================================================================
 * @brief   Update the various maps used to make decisions
 *===========================================================================================
 */
void Jive::updateMaps(Frame *frame)
{
   XYZIPointCloudFrame *frm = dynamic_cast<XYZIPointCloudFrame *>(frame);
    
   for (int i=0; i <  getDim().height; i++)  
   {
      for (int j=0; j< getDim().width; j++) 
      {
         int idx = i*getDim().width+j;
         _zMap.at<float>(i,j) = frm->points[idx].z;
         _aMap.at<float>(i,j) = frm->points[idx].i;
      }
   }
}


/*!
 *===========================================================================================
 *  @brief   Perform morph 'open' to clean up the foregound image
 *===========================================================================================
 */
void Jive::morphClean(Mat &in, Mat &out)
{
   in.convertTo(out, CV_8U, 255.0);
   Mat a = out.clone();
   Mat element = getStructuringElement( 0, Size(3,3), cv::Point(1,1) );
   morphologyEx(a, out, 2, element);
}


/*!
 *===========================================================================================
 *  @brief   Find palm center
 *===========================================================================================
 */
bool Jive::findContourCenter(vector<cv::Point> &contour, cv::Point &center, float &radius)
{
   bool rc = false;
   float m10, m01, m00;

   m10 = m01 = m00 = 0;
   for (int x=0; x < _bMap.cols; x++) {
      for (int y=0; y < _bMap.rows; y++) {
         cv::Point p = cv::Point(x, y);
         if (pointPolygonTest(contour, p, false) >= 0) {
            float val = (float)_bMap.at<uint8_t>(y, x);
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



/*!
 *===========================================================================================
 *  @brief   Find hand tips of qualified contour
 *===========================================================================================
 */
bool Jive::findHandTips(vector< vector<cv::Point> > &contours)
{
   bool found = false;

   _leftHand = 0;
   _rightHand = 1;

   _palmCenter[_leftHand] = cv::Point(0,0);
   _palmCenter[_rightHand] = cv::Point(0,0);
   _palmRadius[_leftHand] = 0;
   _palmRadius[_rightHand] = 0;

   // Find number of qualified hands and remember their contour index
   _numHands = 0;
   for (int i=0; i<contours.size(); i++) 
   {
      if (cv::contourArea(contours[i]) > (int)_minContourSize) 
      {
         if (_numHands < 2)
            _handContour[_numHands] = i;
         _numHands++;
      }
   }

   // Find palms
   if (_numHands > 0 && _numHands <= 2) 
   {
      for (int i=0; i < _numHands; i++) 
      {
         // Find convex hull and defects
         vector<int> hulls, defects;
         vector<Vec4i> convDef;
         vector<cv::Point> contour = contours[_handContour[i]];

         // Find contour center
         cv::Point contourCenter;
         findContourCenter(contour, contourCenter, _palmRadius[i]);
 
         // Find hand tip
         int tip_k = 0;
         int maxY = 0;
         for (int k=0; k<contour.size(); k++)
         { 
            if (contour[k].y > maxY) 
            {
               maxY = contour[k].y;
               tip_k = k;
            }
         }

         // Find palm Center
         cv::Point tip = contour[tip_k];
         _palmCenter[i] = cv::Point((tip.x+contourCenter.x)/2, (tip.y+contourCenter.y)/2);
         _handTip[i] = cv::Point((tip.x+_palmCenter[i].x)/2,(tip.y+_palmCenter[i].y)/2);

      } // for (i)

      if (_numHands == 2 && _palmCenter[_leftHand].x < _palmCenter[_rightHand].x)
      {
         int temp = _leftHand;
         _rightHand = _leftHand;
         _leftHand = temp;
      }
   
   } // if (numHands) 

   return found;
}



/*!
 *===========================================================================================
 *  @brief   Periodic called to update hand-tracking
 *===========================================================================================
 */
void Jive::update(Frame *frame)
{
   vector<Vec4i> hierarchy;
   RNG rng(12345);

   if (getFrameType() == DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME) 
   {
      // Update all maps
      updateMaps(frame);  

      _drawing = Mat::zeros(getDim().height, getDim().width, CV_8UC3 );
      cvtColor(_aMap, _drawing, CV_GRAY2RGB);

      if (_bkgUpdated) 
      {
         // Find foregrounds based on amplitude and depth thresholds
         findForeground(_zLowThresh, _zHighThresh, _zFgMap);

         // Apply morph 'open' to clean up _fgMap; generates _bMap
         morphClean(_zFgMap, _bMap);

         // Crop unused sections
         cropMaps(_bMap, (int)_Xmin, (int)_Xmax, (int)_Ymin, (int)_Ymax);
         Mat canny = _bMap.clone();

         // Find all contours
         findContours(canny, _contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0,0));

         // Find hand tips
         findHandTips(_contours);

         // Move mouse
         if (_numHands > 0)
         {
            int screen_width, screen_height;
            int sensor_width, sensor_hqeight;
            int scaled_x, scaled_y;
         
            mouse.getDim(screen_width, screen_height);
            scaled_x = screen_width - screen_width * (_handTip[0].x-(int)_Xmin) / (int)(_Xmax-_Xmin);
            scaled_y = screen_height - screen_height * (_handTip[0].y-(int)_Ymin) / (int)(_Ymax-_Ymin);
            mouse.moveTo(scaled_x-(int)_Xcur, scaled_y-(int)_Ycur);

            if ( (_zBkgMap.at<float>(_handTip[0]) - _zMap.at<float>(_handTip[0])) < _zTrigger)
            {
               mouse.buttonDown(Button1);
               _bButtonDown = true;
               cout << "buttonDown" << endl;
            }
            else if (_bButtonDown && (_zBkgMap.at<float>(_handTip[0])-_zMap.at<float>(_handTip[0])) > _zTrigger+0.01)
            {
               mouse.buttonUp(Button1);
               _bButtonDown = false;
               cout << "buttonUp" << endl;
            }
         }

      } // if (_bkgUpdated)

      displayMaps();

   } // if (getFrameType() == DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME) 

}

#undef __JIVE_CPP__
/*! @} */
