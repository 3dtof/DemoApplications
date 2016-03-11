/*! 
 * ============================================================================
 *
 * @addtogroup		Horus	
 * @{
 *
 * @file		Horus.cpp
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
#define __HORUS_CPP__
#include "Horus.h"
#include <climits>
#include <algorithm>


Horus::Horus(int w, int h) : TOFApp(w, h)
{
   _setBackground = false;
   initDisplay();
}

void Horus::initDisplay()
{
   namedWindow( "Amplitude", WINDOW_NORMAL );
   namedWindow( "Binary", WINDOW_NORMAL );
   namedWindow( "Morph", WINDOW_NORMAL );
   namedWindow( "Draw", WINDOW_NORMAL );
   namedWindow( "Controls", WINDOW_NORMAL);

   _ampGain = 100;
   _ampThresh = 3;
   _depthThresh = 2;
   _minContourArea = 500;
   _aspectRatio = 100;

   createTrackbar("Amplitude Gain",   "Controls", &_ampGain, 100);
   createTrackbar("Amplitude Thresh", "Controls", &_ampThresh, 100);
   createTrackbar("Depth Threshold",  "Controls", &_depthThresh, 500);
   createTrackbar("MinContour Area",  "Controls", &_minContourArea, 5000);
   createTrackbar("Aspect Ratio (Y/X)%",  "Controls", &_aspectRatio, 500);
}


Mat Horus::clipBackground(float dThr, float iThr)
{
   Mat dMat = Mat::zeros( _iMat.size(), CV_32FC1 );
   Mat fMat = _bkgndMat-_dMat;
   for (int i = 0; i < _dMat.rows; i++) {
      for (int j = 0; j < _dMat.cols; j++) {
         if (fMat.at<float>(i,j) < 0.0f) fMat.at<float>(i,j) = 0.0f;
         dMat.at<float>(i,j) = (_iMat.at<float>(i,j) > iThr &&  fMat.at<float>(i,j) > dThr) ? 255.0 : 0.0;
      }
   }
   return dMat;
}


void Horus::resetBackground()
{
   _setBackground = false;
}


void Horus::getPCA(const vector<cv::Point> &contour, float &center, float &angle)
{
    //Construct a buffer used by the pca analysis
    int sz = static_cast<int>(contour.size());
    Mat data_pts = Mat(sz, 2, CV_32FC1);
    for (int i = 0; i < data_pts.rows; ++i) {
        data_pts.at<float>(i, 0) = contour[i].x;
        data_pts.at<float>(i, 1) = contour[i].y;
    }

    //Perform PCA analysis
    PCA pca_analysis(data_pts, Mat(), CV_PCA_DATA_AS_ROW);

    //Store the center of the object
    cv::Point cntr = cv::Point(static_cast<int>(pca_analysis.mean.at<float>(0, 0)),
                               static_cast<int>(pca_analysis.mean.at<float>(0, 1)));

    //Store the eigenvalues and eigenvectors
    vector<cv::Point2d> eigen_vecs(2);
    vector<float> eigen_val(2);
    for (int i = 0; i < 2; ++i) {
        eigen_vecs[i] = Point2d(pca_analysis.eigenvectors.at<float>(i, 0), 
                                pca_analysis.eigenvectors.at<float>(i, 1));
        eigen_val[i] = pca_analysis.eigenvalues.at<float>(0, i);
    }

    angle = atan2(eigen_vecs[0].y, eigen_vecs[0].x); // orientation in radians
}


// A contour is a person if:
// Aspect ration is largely vertical
// All points are within the same depth interval
// 
bool Horus::isPerson(vector<cv::Point> &contour, Mat dMat)
{
   bool rc = false;
   int area = 0;
   long sumX=0, sumY=0;
   int minX=INT_MAX, minY=INT_MAX;
   int maxX=0, maxY=0;
   int dx, dy;

   // Find biometric statistics
   for (int i=0; i< contour.size(); i++) {
      minX = std::min(minX, contour[i].x);
      minY = std::min(minY, contour[i].y);
      maxX = std::max(maxX, contour[i].x);
      maxY = std::max(maxY, contour[i].y);
      sumX += contour[i].x; 
      sumY += contour[i].y;
   }
   dx = maxX - minX;
   dy = maxY - minY;

   if (contourArea(contour) > _minContourArea) {
      if (dx > 0) {
         float ratio = (float)dy/(float)dx;
         cout << "ratio = " << ratio << endl;
         if (ratio > (float)_aspectRatio/100.0) {
            rc = true;
         }
      }
   } 
   
   return rc;
}


void Horus::update(Frame *frame)
{
   vector< vector<cv::Point> > contours;
   vector<Vec4i> hierarchy;
   RNG rng(12345);

   if (getFrameType() == DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME) {

      // Create amplitude and depth Mat
      vector<float> zMap, iMap;
      XYZIPointCloudFrame *frm = dynamic_cast<XYZIPointCloudFrame *>(frame);
      for (int i=0; i< frm->points.size(); i++) {
         zMap.push_back(frm->points[i].z);
         iMap.push_back(frm->points[i].i);
      }
      _iMat = Mat(getDim().height, getDim().width, CV_32FC1, iMap.data());
      _dMat = Mat(getDim().height, getDim().width, CV_32FC1, zMap.data()); 

      // Apply amplitude gain
      _iMat = (float)_ampGain*_iMat;

      // Update background as required
      if (!_setBackground) {
         _dMat.copyTo(_bkgndMat);
         _setBackground = true;
         cout << endl << "Updated background" << endl;
      }

      // Find foreground by subtraction and convert to binary 
      // image based on amplitude and depth thresholds
      Mat fMat = clipBackground((float)_depthThresh/100.0, (float)_ampThresh/100.0);

      // Apply morphological open to clean up image
      fMat.convertTo(_bMat, CV_8U, 255.0);
      Mat morphMat = _bMat.clone();
      Mat element = getStructuringElement( 0, Size(5,5), cv::Point(1,1) );
      morphologyEx(_bMat, morphMat, 2, element);

      // Draw contours that meet a "person" requirement
      Mat drawing = Mat::zeros( _iMat.size(), CV_8UC3 );
      Mat im_with_keypoints = Mat::zeros( _iMat.size(), CV_8UC3 );
      cvtColor(_iMat, drawing, CV_GRAY2RGB);

      int peopleCount = 0;

#if 1
      // Find all contours
      findContours(morphMat, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0,0));
      for ( int i = 0; i < contours.size(); i++ ) { 
         if (isPerson(contours[i], _dMat)) {  
            peopleCount++;
            drawContours( drawing, contours, i, Scalar(0, 0, 255), 2, 8, vector<Vec4i>(), 0, cv::Point() ); 
         }
      }
#else
      // Find blobs
      std::vector<KeyPoint> keypoints;
      SimpleBlobDetector::Params params;


      // Filter by color
      params.filterByColor = true;
      params.blobColor = 255;

      // Change thresholds - depth
      params.minThreshold = 0;
      params.maxThreshold = 1000;

      // Filter by Area.
      params.filterByArea = true;
      params.minArea = 100;
      params.maxArea = 100000;

      // Filter by Circularity
      params.filterByCircularity = false;
      params.minCircularity = 0.1;
 
      // Filter by Convexity
      params.filterByConvexity = false;
      params.minConvexity = 0.87;
 
      // Filter by Inertia
      params.filterByInertia = false;
      params.minInertiaRatio = 0.01;


      cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params); 
      detector->detect( morphMat, keypoints );

      cout << "Keypoints # " << keypoints.size() << endl;

      for ( int i = 0; i < keypoints.size(); i++ ) { 
	 cv::circle( drawing, cv::Point(keypoints[i].pt.x, keypoints[i].pt.y), 10, Scalar(0,0,255), 4 );
      }
      peopleCount = keypoints.size();
#endif

      putText(drawing, "Count = "+to_string(peopleCount), cv::Point(200, 50), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 255));

      imshow("Binary", _bMat);
      imshow("Amplitude", _iMat); 
      imshow("Draw", drawing);
      imshow("Morph", morphMat);
   }
}

#undef __HORUS_CPP__
/*! @} */
