/*!
 *****************************************************************************
 *
 * @addtogroup          cluster2.h
 * @{
 *
 * @file                cluster.h
 * @version             1.0
 * @date                11/3/2015
 * @note                DBSCAN cluster algorithms
 *
 * Copyright© 2014-2015 Texas Instruments Corporation, All Rights Reserved.
 * TI makes NO WARRANTY as to software products, which are supplied "AS-IS"
 *
 *****************************************************************************
 */
#ifndef __CLUSTER_H__
#define __CLUSTER_H__

#include "CameraSystem.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define UNALLOCATED         	(-1)
#define IMAGE_AT(img, x, y) 	(img.at<float>(y, x))	
#define IMAGE_WIDTH(img)	(img.cols)
#define IMAGE_HEIGHT(img)      	(img.rows)
#define POINT			Voxel::Point

using namespace std;
using namespace cv;

class Cluster
{
public:
   Cluster();
   ~Cluster();
   void addPoint(POINT p);
   POINT getCG();
   POINT getCentroid();
   inline vector<POINT> getPoints() {return _points;}
   inline float getMass() {return _mass;}
   inline float getArea() {return _area;}
   inline POINT getMin() {return _rect_min;}
   inline POINT getMax() {return _rect_max;}

private:
   vector<POINT> _points;
   float _area, _mass, _xy_sum;
   POINT _moment;
   POINT _sum;
   POINT _rect_min;
   POINT _rect_max;
};


class ClusterMap
{
public:
    ClusterMap(float den, float thr, int sz);
    void scan(Mat &m);
    inline vector<Cluster> &getClusters() {return _clusters;}
    inline Mat &getLabelMap() {return _labelMap;}

private:
    bool qualify(Mat d, int x, int y);

private:
    float _density, _thresh;
    int _kernSz;
    Mat _labelMap;
    vector<Cluster> _clusters;
};

#endif /* __CLUSTER_H__ */
/*! @} */    

