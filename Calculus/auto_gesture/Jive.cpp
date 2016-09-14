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

#define MAJOR_AXIS      0
#define MINOR_AXIS      1

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

/*!
 *===========================================================================================
 * @brief   Initialize control window based on Jive parameters
 *===========================================================================================
 */
Jive::Jive(int w, int h) : TOFApp(w, h)
{
   _zMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _aMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _zFgMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _aFgMap = Mat::zeros(getDim().height, getDim().width, CV_32FC1);
   _bMap = Mat::zeros(getDim().height, getDim().width, CV_8U);
   _drawing = Mat::zeros(getDim().height, getDim().width, CV_8UC3 );

   _zThresh = 0.5;
   _aThresh = 0.005;
   _aGain = 50;
   _minContourSize = 100;
   _Xmin = 0;
   _Xmax = TOF_WIDTH-1;
   _Ymin = 0;
   _Ymax = TOF_HEIGHT-1;

   _avg[0] = _avg[1] = 0;
   

   // Setup parameter map

   _param["aThresh"] = std::make_tuple(&_aThresh, 1000, 2);
   _param["zThresh"] = std::make_tuple(&_zThresh, 1000, 2);
   _param["aGain"] = std::make_tuple(&_aGain, 10, 200);
   _param["minContourSize"] = std::make_tuple(&_minContourSize, 1, 10000);
   _param["Xmin"] = std::make_tuple(&_Xmin, 1, TOF_WIDTH-1);
   _param["Xmax"] = std::make_tuple(&_Xmax, 1, TOF_WIDTH-1);
   _param["Ymin"] = std::make_tuple(&_Ymin, 1, TOF_HEIGHT-1);
   _param["Ymax"] = std::make_tuple(&_Ymax, 1, TOF_HEIGHT-1);

   // Setup image map
   _images["aMap"] = &_aMap;
   _images["aFgMap"] = &_aFgMap;
   _images["zMap"] = &_zMap;
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
   if (_numHands > 0 && _numHands <= 2) 
   {
      for (int i=0; i < _numHands; i++) 
      {         
         vector<cv::Point> contour = _contours[_handContour[i]];

         // Draw hand
         cv::drawContours(_drawing, _contours, _handContour[i], Scalar(0, 0, 255), 0, 1, vector<Vec4i>(), 0, cv::Point() ); 

         // Draw palm
         cv::circle(_drawing, _palmCenter[i], _palmRadius[i], Scalar(0,255,0), 1);       

#if 1
         // Draw wrist points
         if (_wristStart[i] >= 0 && _wristEnd[i] >= 0)
         {
            cv::circle(_drawing, contour[_wristStart[i]], 2, Scalar(0, 255, 0), 1);
            cv::circle(_drawing, contour[_wristEnd[i]], 2, Scalar(255, 0, 0), 1);
            cv::line(_drawing, contour[_wristStart[i]], contour[_wristEnd[i]], CV_RGB(255, 0, 0));
         }
#endif

#if 1
         // Draw fingertips
         for (int k=0; k < _fingerTips[i].size(); k++)
            cv::circle(_drawing, _fingerTips[i][k], 1, Scalar(0, 255, 0), 1);
#endif

#if 1
         // Draw defects
         for (int k=0; k < _defects[i].size(); k++)
            cv::circle(_drawing, contour[_defects[i][k]], 1, Scalar(255, 0, 0), 1);
#endif

#if 1
         // Draw the principal components
         cv::line(_drawing, _palmCenter[i], _palmCenter[i] + 0.1*_major[i], CV_RGB(255, 255, 0));
         cv::line(_drawing, _palmCenter[i], _palmCenter[i] + 0.1*_minor[i], CV_RGB(0, 255, 255));
#endif

         cv::putText(_drawing, std::to_string(_fingerTips[i].size()), cv::Point(5, 55), FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(255,125,50));
      } // for (i)

   }


   // Display all registered maps
   for (int i=0; i < _mapsToDisplay.size(); i++) 
   {
      if (_mapsToDisplay[i] == "aMap" || _mapsToDisplay[i] == "aFgMap" || _mapsToDisplay[i] == "drawing" ) 
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
void Jive::findForeground(float zThr, float aThr, Mat &fgMap)
{ 
   for (int i = 0; i < _zMap.rows; i++) 
   {
      for (int j = 0; j < _zMap.cols; j++) 
      {
         if (_zMap.at<float>(i,j) < zThr && _aMap.at<float>(i,j) > aThr) 
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
bool Jive::findPalmCenter(vector<cv::Point> &contour, cv::Point &center, float &radius)
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
int Jive::adjPix(int pix)
{
   int out = pix*getDim().width/TOF_WIDTH;
   return (out<=0)?1:out;
}

/*!
 *===========================================================================================
 *  @brief   Get orientation of a collection of points
 *===========================================================================================
 */
double Jive::getOrientation(vector<cv::Point> &pts, cv::Point &pos, vector<cv::Point2d> &e_vec, vector<double> &e_val)
{
    //Construct a buffer used by the pca analysis
    Mat data_pts = Mat(pts.size(), 2, CV_64FC1);
    for (int i = 0; i < data_pts.rows; ++i)
    {
        data_pts.at<double>(i, 0) = pts[i].x;
        data_pts.at<double>(i, 1) = pts[i].y;
    }

    //Perform PCA analysis
    PCA pca_analysis(data_pts, Mat(), CV_PCA_DATA_AS_ROW);
 
    //Store the position of the object
    pos = cv::Point(pca_analysis.mean.at<double>(0, 0),
                    pca_analysis.mean.at<double>(0, 1));

    //Store the eigenvalues and eigenvectors
    e_vec.clear();
    e_val.clear();
    for (int i = 0; i < 2; ++i)
    {
        e_vec.push_back(cv::Point2d(pca_analysis.eigenvectors.at<double>(i, 0),
                                  pca_analysis.eigenvectors.at<double>(i, 1)));
        e_val.push_back(pca_analysis.eigenvalues.at<double>(0, i));
    }
 
    return atan2(e_vec[0].y, e_vec[0].x);
}


/*!
 *===========================================================================================
 *  @brief   Find convex hulls and convexity defects of a contour
 *===========================================================================================
 */
void Jive::findKeyPoints(vector<cv::Point> &contour, vector<int> &hulls, vector<int> &defects, int depth)
{
   vector<Vec4i> convDef;

   hulls.clear();
   defects.clear();
   convexHull(Mat(contour), hulls, false );
   if (hulls.size() > 3)
   {
      convexityDefects(contour, hulls, convDef);
      for (int k=1; k<convDef.size(); k++) {  // first defect is false
         if (convDef[k][3] > depth*256) {
            int ind = convDef[k][2];
            defects.push_back(ind);
         }
      }
   }
}

/*!
 *===========================================================================================
 *  @brief   Find the hull point in the middle of a cluster of convex hulls associated 
 *           with one fingertip
 *===========================================================================================
 */
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


/*!
 *===========================================================================================
 *  @brief   Reduce cluster of convex hulls of a tip to just one point
 *===========================================================================================
 */
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

/*!
 *===========================================================================================
 *  @brief   Remove border hull points
 *===========================================================================================
 */
void Jive::removeBorderPoints(vector<cv::Point> &contour, vector<int> &hull)
{
   vector<int> out;
   out.clear();

   if (hull.size() > 2)
   {
      for (int k=0; k < hull.size(); k++) 
      {
         cv::Point p = contour[hull[k]];
         if (p.x > _Xmin+1 && p.x < _Xmax-1 && p.y > _Ymin+1 && p.y < _Ymax-1)
            out.push_back(hull[k]);
      }
   }
   hull = out;
}

/*!
 *===========================================================================================
 *  @brief   Find angle among three points
 *===========================================================================================
 */
double Jive::findAngle(cv::Point p, cv::Point p1, cv::Point p2)
{
   double dval = ((p1.x-p.x)*(p2.x-p.x)+(p1.y-p.y)*(p2.y-p.y))
                 / (norm(Mat(p1), cv::Mat(p))*norm(Mat(p2), Mat(p)));
   double a = acos(dval)*180.0/3.1415926;
   return a;
}


/*!
 *===========================================================================================
 *  @brief   Quality hull points as fingertips based on k curvature (sharpness)
 *===========================================================================================
 */
void Jive::kCurvature(vector<cv::Point> &contour, vector<int> &hull, int kmin, int kmax,
                      double ang, vector<int> &tips)
{
   tips.clear();
   for (int i=0; i < hull.size(); i++) {
      int n = hull[i];
      cv::Point p = contour[n];
      bool done = false;
      for (int k = 1; k < kmax && !done; k++) {
         double a = findAngle(p, p, p);
         if (a < ang && k >= kmin) {
            tips.push_back(n);
            done = true;
         }
      }
   }
}

/*!
 *===========================================================================================
 *  @brief   Quality hull points as fingertips based on k curvature (sharpness)
 *===========================================================================================
 */
double Jive::distPoint2Line(cv::Point p1, cv::Point p2, cv::Point p)
{
   double y2y1 = p2.y-p1.y;
   double x2x1 = p2.x-p1.x;

   double num = abs(y2y1*p.x - x2x1*p.y + p2.x*p1.y - p2.y*p1.x);
   double den = sqrt(y2y1*y2y1 + x2x1*x2x1);

   return num/den;
}


/*!
 *===========================================================================================
 *  @brief  Find wrists
 *===========================================================================================
 */
void Jive::findWrists(vector<cv::Point> &contour, cv::Point palm, double radius, int &start, int &end)
{
   // Find two points where contour intersects the minor we call that 'wrist'
   cv::Point p1 = palm - cv::Point(0, radius);
   //cv::Point p2 = p1 + _minor[i];
   cv::Point p2 = p1 + cv::Point(1, 0);

   start = end = -1;
   for (int k=0; k < contour.size(); k++)
   {
      cv::Point p = contour[k];
      double d = distPoint2Line(p1, p2, p);
      if (start < 0 && d < 3.0f) 
      {
         start = k;
         break;
      }
   }
   for (int k=contour.size()-1; k >= 0; k--)
   {
      cv::Point p = contour[k];
      double d = distPoint2Line(p1, p2, p);
      if (end < 0 && d < 3.0f) 
      {
         end = k;
         break;
      }
   }
}


/*!
 *===========================================================================================
 *  @brief   Find hand tips of qualified contour
 *===========================================================================================
 */
bool Jive::findFingertips(vector< vector<cv::Point> > &contours)
{
   bool found = false;

   _leftHand = 0;
   _rightHand = 1;

   _palmCenter[_leftHand] = cv::Point(0,0);
   _palmCenter[_rightHand] = cv::Point(0,0);
   _palmRadius[_leftHand] = 0;
   _palmRadius[_rightHand] = 0;
   _palmDepth[_leftHand] = 0;
   _palmDepth[_rightHand] = 0;

   // Find number of qualified hands and remember their contour index
   _numHands = 0;
   for (int i=0; i<contours.size(); i++) 
      if (cv::contourArea(contours[i]) > (int)_minContourSize) 
         if (_numHands < 2)
            _handContour[_numHands++] = i;


   // Find palms
   if (_numHands > 0 && _numHands <= 2) 
   {
      for (int i=0; i < _numHands; i++) 
      {
         vector<int> hulls;
         vector<cv::Point> contour = contours[_handContour[i]];

         _fingerTips[i].clear();

         // Find hand orientation 
         getOrientation(contour, _palmCenter[i], _eigenVec[i], _eigenVal[i]);
         _major[i] = cv::Point(_eigenVec[i][MAJOR_AXIS].x * _eigenVal[i][MAJOR_AXIS], 
                                     _eigenVec[i][MAJOR_AXIS].y * _eigenVal[i][MAJOR_AXIS]);
         _minor[i] = cv::Point(_eigenVec[i][MINOR_AXIS].x * _eigenVal[i][MINOR_AXIS], 
                                     _eigenVec[i][MINOR_AXIS].y * _eigenVal[i][MINOR_AXIS]);

         // Find palm center and depth
         findPalmCenter(contour, _palmCenter[i], _palmRadius[i]);
         _palmDepth[i] = _zMap.at<float>(_palmCenter[i].x, _palmCenter[i].y);


         // Find convex hulls and defects
         findKeyPoints(contour, _hulls[i], _defects[i], 4);



         // Find two points where contour intersects the minor we call that 'wrist'
         findWrists(contour, _palmCenter[i], _palmRadius[i], _wristStart[i], _wristEnd[i]);


         // Find longest point in each segment along the contour starting
         // from wristStart and ends with wristEnd.  Segments along the way
         // are delineated by convexity defects.
         //if (_wristStart[i] >= 0 && _wristEnd[i] >= 0) 
         if (_wristStart[i] >= 0 && _wristEnd[i] >= 0 && cv::norm(_wristStart[i]-_wristEnd[i]) > 5)
         {
            int start = _wristStart[i];
            int next = 0;
            int maxIdx = -1;
            double maxDist = 0;
            cv::Point p1, p2;

            // Find longest point between starting point and end point
            _fingerTips[i].clear();
            for (int k=0; k < _defects[i].size(); k++)
            {
               maxDist = 0;
               maxIdx = -1;           
               next = _defects[i][k];
               
               p1 = contour[start];
               p2 = contour[next];
               for (int j=start+1; j < next; j++)
               {
                  cv::Point p = contour[j];
                  double d = cv::norm(p-p1) + cv::norm(p-p2);
                  if (d > maxDist) 
                  {
                     maxDist = d;
                     maxIdx = j;
                  }
               }
               // qualify tip by angle
               if (maxIdx >= 0 && findAngle(contour[maxIdx], p1, p2) < 60)
                     _fingerTips[i].push_back(contour[maxIdx]);

               start = next;
            }

            // One more segment between last defect start and _wristEnd[i]
            maxDist = 0;
            maxIdx = -1;           
            next = _wristEnd[i];
               
            p1 = contour[start];
            p2 = contour[next];
            for (int j=start; j < next; j++)
            {
               cv::Point p = contour[j];
               double d = cv::norm(p-p1) + cv::norm(p-p2);
               if (d > maxDist) 
               {
                  maxDist = d;
                  maxIdx = j;
               }
            }
            // qualify tip
            if (maxIdx >= 0 && _defects[i].size() > 0 && findAngle(contour[maxIdx], p1, p2) < 60)
               _fingerTips[i].push_back(contour[maxIdx]);
         }
      } // for (i)


      // Straighten out left vs right hand
      if (_numHands == 2 && _palmCenter[0].x < _palmCenter[1].x)
      {
         _rightHand = 0; 
         _leftHand = 1;
      }
      else
      {
         _rightHand = 1; 
         _leftHand = 0;
      }
      
   } // if (numHand)

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
      // Update aMap and zMap 
      updateMaps(frame);  

  //    cout << "A";

      // Initialize drawing canvas
      _drawing = Mat::zeros(getDim().height, getDim().width, CV_8UC3 );
      cvtColor(_aMap, _drawing, CV_GRAY2RGB);

    //  cout << "B";

      // Find foregrounds based on amplitude and depth thresholds
      findForeground(_zThresh, _aThresh, _zFgMap);

    //  cout << "C";

      // Apply morph 'open' to clean up _fgMap; generates _bMap
      morphClean(_zFgMap, _bMap);

    //  cout << "D";

      // Crop unused sections
      cropMaps(_bMap, (int)_Xmin, (int)_Xmax, (int)_Ymin, (int)_Ymax);

    //  cout << "E";

      // Find all contours
      Mat canny = _bMap.clone();
      findContours(canny, _contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0,0));

     // cout << "F";

      // Find hand parameters
      findFingertips(_contours);

     // cout << "G";


      // Update map
      displayMaps();

     // cout << "H";

   } // if (getFrameType() == DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME) 

}

#undef __JIVE_CPP__
/*! @} */
