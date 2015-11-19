/*!
 *****************************************************************************
 *
 * @addtogroup         cluster
 * @{
 *
 * @file                cluster.cpp
 * @version             1.0
 * @date                11/3/2015
 *
 * @note                DBSCAN cluster algorithm
 *
 * Copyright© 2000-2012 Texas Instruments Corporation, All Rights Reserved.
 * TI makes NO WARRANTY as to software products, which are supplied "AS-IS"
 *
 *****************************************************************************
 */
#define __CLUSTER_CPP__

#include "Cluster.h"

Cluster::Cluster()
{
   _points.clear();
   _area = _mass = 0;
   _moment = POINT(0, 0, 0);
   _sum = POINT(0, 0, 0);
   _rect_min = POINT(10000, 10000, 10000);
   _rect_max = POINT(0, 0, 0);
   _xy_sum = 0;
}

Cluster::~Cluster()
{
   _points.clear();
}


void Cluster::addPoint(POINT p)
{
   _points.push_back(p);
   _area += 1;
   _mass += p.z;
   _moment.x += p.z * p.x;
   _moment.y += p.z * p.y;
   _sum.x += p.x;
   _sum.y += p.y;
   _rect_min.x = (p.x < _rect_min.x) ? p.x : _rect_min.x;
   _rect_min.y = (p.y < _rect_min.y) ? p.y : _rect_min.y;
   _rect_min.z = (p.z < _rect_min.z) ? p.z : _rect_min.z;
   _rect_max.x = (p.x > _rect_max.x) ? p.x : _rect_max.x;
   _rect_max.y = (p.y > _rect_max.y) ? p.y : _rect_max.y;
   _rect_max.z = (p.z > _rect_max.z) ? p.z : _rect_max.z;
}

POINT Cluster::getCG()
{
   if (_mass > 0)
      return POINT(_moment.x/_mass, _moment.y/_mass, 0);
   return POINT(0, 0, 0);
}
    

POINT Cluster::getCentroid()
{
   if (_area > 0)
      return POINT(_sum.x/_area, _sum.y/_area, 0);
   return POINT(0, 0, 0);
}



//===================================================

ClusterMap::ClusterMap(float den, float thr, int sz)
{
   _density = den; 
   _thresh = thr;
   _kernSz = sz;
}


bool ClusterMap::qualify(Mat d, int x, int y)
{
   float area = 0, count = 0;

   if (IMAGE_AT(d, x, y) > _thresh) {
      for (int i = x-_kernSz; i <= x+_kernSz; i++) {
         if (i >= 0 && i < IMAGE_WIDTH(d)) {
            for (int j = y-_kernSz; j <= y+_kernSz; j++) {
               if (j >= 0 && j < IMAGE_HEIGHT(d))  {
                  area = area + 1;
                  if (IMAGE_AT(d, i, j) > _thresh)
                     count = count + 1;
               }
            }
         }
      }
      if (count >= _density*area)
         return true;
   }
   return false;
}


void ClusterMap::scan(Mat &d)
{
   Mat c = Mat(d.rows, d.cols, CV_32FC1);
   c = Scalar(UNALLOCATED);

   _clusters.clear();
   for (int x = 0; x < IMAGE_WIDTH(d); x++) {
      for (int y = 0; y < IMAGE_HEIGHT(d); y++) {
         /* if (P > thresh && density_qualify(P) */
         if (qualify(d, x, y)) {
            /* if (!allocated(P) */
            if (IMAGE_AT(c, x, y) == UNALLOCATED) {
               /* iterate N := Neighbor(P) */
               for (int i = x-_kernSz; i <= x+_kernSz; i++) {
                  if (i >= 0 && i < IMAGE_WIDTH(d)) {
                     for (int j = y-_kernSz; j <= y+_kernSz; j++) {
                        if (j >= 0 && j < IMAGE_HEIGHT(d)) {
                           if (i != x || j != y) {
                              /* if (allocated(N)) */
                              if (IMAGE_AT(c, i, j) != UNALLOCATED) {
                                 /* if density_qualify(N) */
                                 if (qualify(d, i, j)) {
                                    /* P.id = N.id */
                                    IMAGE_AT(d, x, y) = IMAGE_AT(c, i, j);
                                    goto expand;
                                 }
                              }
                           }
                        }
                     }
                  }
               }
               /* P.id = new(id) */
               IMAGE_AT(c, x, y) = _clusters.size();
               Cluster *cl = new Cluster;
               _clusters.push_back(*cl);
               delete cl;
            } /* if (!allocated(P)) */

expand:
            /* iterate N := Neighbor(P) */
            for (int i = x-_kernSz; i <= x+_kernSz; i++) {
               if (i >= 0 && i < IMAGE_WIDTH(d)) {
                  for (int j = y-_kernSz; j <= y+_kernSz; j++) {
                     if (j >= 0 && j < IMAGE_HEIGHT(d)) {
                        if (i != x || j != y) {
                           /* if (N > thresh) N.id = P.id */
                           if (IMAGE_AT(d, i, j) > _thresh)
                              IMAGE_AT(c, i, j) = IMAGE_AT(c, x, y);
                        }
                     }
                  }
               }
            }
         }  /* if (P > thresh && density_qualify(P) */
      }   /* iterate P := All Points */
   }

   /* Allocate points to clusters */
   for (int x = 0; x < IMAGE_WIDTH(d); x++) {
      for (int y = 0; y < IMAGE_HEIGHT(d); y++) {
          int cid = (int)(IMAGE_AT(c, x, y));
          if (cid != UNALLOCATED) {
             _clusters[cid].addPoint(POINT(x, y, IMAGE_AT(d, x, y)));
          }
      }
   }
}


#undef __CLUSTER_CPP__
/*! @} */





