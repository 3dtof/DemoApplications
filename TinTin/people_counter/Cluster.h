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

#include <CameraSystem.h>

#define UNALLOCATED         	(-1)
#define IMAGE_AT(img, x, y) 	img.depth.data()[y*img.size.width+x]

using namespace std;
using namespace Voxel;

class Cluster
{
public:
   Cluster();
   ~Cluster();
   void AddPoint(Point p);
   Point GetCG();
   Point GetCentroid();
   float GetMass();
   float GetArea();
   Point GetMin();
   Point GetMax();
#if 0
   Point GetMajor();
   Point GetMinor();
#endif

private:
   vector<Point> points;
   float area, mass, xy_sum;
   Point moment;
   Point sum;
   Point sq_sum;
   Point rect_min;
   Point rect_max;
};


class ClusterMap
{
public:
    ClusterMap(float den, float thr, int sz);
    void Scan(DepthFrame d);
    vector<Cluster> &GetClusters();
    DepthFrame &GetLabelMap();

private:
    bool qualify(DepthFrame d, int x, int y);

private:
    float density, thresh;
    int kern_sz;
    DepthFrame c;
    vector<Cluster> clusters;
};

#endif /* __CLUSTER_H__ */
/*! @} */    

