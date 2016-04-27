/*! 
 * ============================================================================
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
 * ============================================================================
 */
#define __JIVE_CPP__
#include "Jive.h"
#include <climits>
#include <algorithm>

/*!
 * @brief   Initialize control window based on Jive parameters
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
   _zDiffMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _aDiffMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _zHeatMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _aHeatMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _zPrevMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _aPrevMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _zBkgMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _aBkgMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _zFgMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _aFgMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _bMap = Mat::zeros(getDim().height, getDim().width, CV_8U);

   _bkgUpdated = false;
   _aHeatMapThresh = 0;
   _zHeatMapThresh = 0;
   _heatMapCoef = 0.5;
   _zLowThresh = 0;
   _zHighThresh = 4095;

   // Setup parameter map
   _param["aHeatMapThresh"] = std::make_tuple(&_aHeatMapThresh, 1, 4095);
   _param["zHeatMapThresh"] = std::make_tuple(&_zHeatMapThresh, 1, 4095);
   _param["heatMapCoef"]    = std::make_tuple(&_heatMapCoef, 100, 1);
   _param["zLowThresh"]     = std::make_tuple(&_zLowThresh, 1, 4095);
   _param["zHighThresh"]    = std::make_tuple(&_zHighThresh, 1, 4095);

   // Setup image map
   _images["aMap"] = &_aMap;
   _images["aBkgMap"] = &_aBkgMap;
   _images["aFgMap"] = &_aFgMap;
   _images["aHeatMap"] = &_aHeatMap;
   _images["aDiffMap"] = &_aDiffMap;
   _images["aPrevMap"] = &_aPrevMap;
   _images["zMap"] = &_zMap;
   _images["zBkgMap"] = &_zBkgMap;
   _images["zFgMap"] = &_zFgMap;
   _images["zHeatMap"] = &_zHeatMap;
   _images["zDiffMap"] = &_zDiffMap;
   _images["zPrevMap"] = &_zPrevMap;
   _images["bMap"] = &_bMap;

}


/*!
 * @brief   Initialize control window based on Jive parameters
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
 * @brief   Initialize control window based on Jive parameters
 */
void Jive::addMapToDisplay(std::string name)
{
   _mapsToDisplay.push_back(name);
}



/*!
 * @brief   Initialize control window based on Jive parameters
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
 * @brief   Initialize control window based on Jive parameters
 */
void Jive::displayMaps()
{
   for (int i=0; i < _mapsToDisplay.size(); i++) 
      imshow(_mapsToDisplay[i], *_images[_mapsToDisplay[i]]);
}


/*!
 * @brief  Get the list of prestored parameter strings
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
 * @brief  Get the list of prestored parameter strings
 */
map< std::string, std::tuple<float*, int, float> > &Jive::getParamMap()
{
   return _param;
}


/*!
 * @brief  Get the list of prestored parameter strings
 */
map< std::string, cv::Mat* > &Jive::getImageMap()
{
   return _images;
}


/*!
 * @brief  Find foreground based on thresholds.  
 */
void Jive::findForeground(float zLowThr, float zHighThr, Mat &fgMap)
{ 
   Mat zFgMap = _zBkgMap-_zMap;

   for (int i = 0; i < _zMap.rows; i++) 
   {
      for (int j = 0; j < _zMap.cols; j++) 
      {
         if (zFgMap.at<float>(i,j)<zHighThr && zFgMap.at<float>(i,j)>zLowThr) 
            fgMap.at<float>(i,j) = 255.0f;
         else 
            fgMap.at<float>(i,j) = 0.0f;
      }
   }
}


/*!
 * @brief   Update the various maps used to make decisions
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
         _zDiffMap.at<float>(i,j) = _zMap.at<float>(i,j)-_zPrevMap.at<float>(i,j);
         _aDiffMap.at<float>(i,j) = _aMap.at<float>(i,j)-_aPrevMap.at<float>(i,j);
         _zHeatMap.at<float>(i,j) = (1-_heatMapCoef)*_zHeatMap.at<float>(i,j)
                                  + _heatMapCoef*abs(_zDiffMap.at<float>(i,j));
         _aHeatMap.at<float>(i,j) = (1-_heatMapCoef)*_aHeatMap.at<float>(i,j)
                                  + _heatMapCoef*abs(_aDiffMap.at<float>(i,j));
         _zPrevMap.at<float>(i,j) = _zMap.at<float>(i,j);
         _aPrevMap.at<float>(i,j) = _aMap.at<float>(i,j);
      }
   }
}


/*!
 *  @brief   Perform morph 'open' to clean up the foregound image
 */
void Jive::morphClean(Mat &in, Mat &out)
{
   in.convertTo(out, CV_8U, 255.0);
   Mat morph = out.clone();
   Mat element = getStructuringElement( 0, Size(5,5), cv::Point(1,1) );
   morphologyEx(out, morph, 2, element);
}


#if 0
/*!
 *  @brief   Try recognize gesture of qualified contour
 */
bool Jive::findGestures(vector< vector<cv::Point> > &contours, enum Gesture &gesture, int[2] &values)
{
   bool found = false;
   int numHands = 0;
   int handContour[2];

   // Find number of qualified hands and remember their contour index
   for (int i=0; i<contours.size(); i++) 
   {
      if (contourArea(contours[i]) > _minContourArea) 
      {
         if (numHands < 2)
            handContour[numHands] = i;
         numHands++;
      }
   }

   // Only process for 1 or 2 hands
   if (numHands <= 2) 
   {
      for (int i=0; i < numHands; i++) 
      {
         // Find convex hull and defects
         vector<int> hulls, defects;
         vector<Vec4i> convDef;
         vector<cv::Point> contour = contours[handContours[i]];
         convexHull(Mat(contour, hulls, false ); 
         convexityDefects(contour, hulls, convDef);
         for (int k=1; k<convDef.size(); k++) 
         {  
            if (convDef[k][3] > _minConvDefDepth*256) 
            {
               int ind = convDef[k][2];
               defects.push_back(ind);
            } // if (convDef[k][3]) 
         } // for (k) 

         // Find average hull point distances
         double dist, distAvg=0, distMax=0, distMin=1e10;
         cv::Point start = contour[hulls[0]];
         for (int j=1; j<hulls.size(); j++)
         {
            dist = cv::norm(contour[hulls[j]]-start);
            distMax = (dist > distMax) ? dist : distMax;
            distMin = (dist < distMin) ? dist : distMin;
            distAvg += dist;
            start = contour[hulls[j]];
         }
         dist = cv::norm(contour[hulls[j]]-start);
         distMax = (dist > distMax) ? dist : distMax;
         distMin = (dist < distMin) ? dist : distMin;
         distAvg += dist;
         distAvg /= (double)hulls.size();

         for (int j=0; j< hulls.size(); j++) 
         {
            if (hull
         }

      } // for (i)
   } // if (numHands) 
   
   return found;
}
#endif

/*!
 *  @brief   Periodic called to update hand-tracking
 */
void Jive::update(Frame *frame)
{
   vector< vector<cv::Point> > contours;
   vector<Vec4i> hierarchy;
   RNG rng(12345);

   if (getFrameType() == DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME) 
   {
      // Update all maps
      updateMaps(frame);  

      // Check if background should update
      if (cv::sum(_zHeatMap)[0] < _zHeatMapThresh && cv::sum(_aHeatMap)[0] < _aHeatMapThresh)
      {
         _aMap.copyTo(_aBkgMap);
         _zMap.copyTo(_zBkgMap);
         _bkgUpdated = true;
      }

#if 0
      if (_bkgUpdated) 
      {
         // Find foregrounds based on amplitude and depth thresholds
         findForeground(_zLowThresh, _zHighThresh, _zFgMap);

         // Apply morph 'open' to clean up _fgMap; generates _bMap
         morphClean(_zFgMap, _bMap);

         // Find all contours
         findContours(_bMap, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0,0));

         // Find gestures
         enum Gestures gesture;
         int values[2];
         findGesture(hands, gesture, values);

         // Dispatch gesture actions
         switch (gesture) 
         {
            // One-hand
            case GESTURE_1H1F:
               cout << "1H1F" << " POS:" << value[0] << endl;
               break;

            case GESTURE_1H2F:
               cout << "1H2F" << " POS:" << value[0] << endl;
               break;

            case GESTURE_1H3F:
               cout << "1H3F" << " POS:" << value[0] << endl;
               break;

            case GESTURE_1H4F:
               cout << "1H4F" << " POS:" << value[0] << endl;
               break;

            case GESTURE_1H5F:
               cout << "1H5F" << " POS:" << value[0] << endl;
               break;

            // Two-hands
            case GESTURE_2H1F:
               cout << "2H1F" << " POS:" << value[0] << " ROT:" << value[1] << endl;
               break;

            case GESTURE_2H2F:
               cout << "2H2F" << " POS:" << value[0] << " ROT:" << value[1] << endl;
               break;

            case GESTURE_2H3F:
               cout << "2H3F" << " POS:" << value[0] << " ROT:" << value[1] << endl;
               break;

            case GESTURE_2H4F:
               cout << "2H4F" << " POS:" << value[0] << " ROT:" << value[1] << endl;
               break;

            case GESTURE_2H5F:
               cout << "2H5F" << " POS:" << value[0] << " ROT:" << value[1] << endl;
               break;

            // Default
            default:
               break;
         } // switch()

      } // if (_bkgUpdated)

#endif

      displayMaps();

   } // if (getFrameType() == DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME) 

}

#undef __JIVE_CPP__
/*! @} */
