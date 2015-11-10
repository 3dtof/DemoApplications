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
   points.clear();
   area = mass = 0;
   moment = Point(0, 0, 0);
   sum = Point(0, 0, 0);
   sq_sum = Point(0, 0, 0);
   rect_min = Point(10000, 10000, 10000);
   rect_max = Point(0, 0, 0);
   xy_sum = 0;
}

Cluster::~Cluster()
{
   points.clear();
}


void Cluster::AddPoint(Point p)
{
   points.push_back(p);
   area += 1;
   mass += p.z;
   moment.x += p.z * p.x;
   moment.y += p.z * p.y;
   sum.x += p.x;
   sum.y += p.y;
#if 0
   sq_sum.x += p.x * p.x;
   sq_sum.y += p.y * p.y;
   xy_sum += p.x * p.y;
#endif
   rect_min.x = (p.x < rect_min.x) ? p.x : rect_min.x;
   rect_min.y = (p.y < rect_min.y) ? p.y : rect_min.y;
   rect_min.z = (p.z < rect_min.z) ? p.z : rect_min.z;
   rect_max.x = (p.x > rect_max.x) ? p.x : rect_max.x;
   rect_max.y = (p.y > rect_max.y) ? p.y : rect_max.y;
   rect_max.z = (p.z > rect_max.z) ? p.z : rect_max.z;
}

Point Cluster::GetCG()
{
   if (mass > 0)
      return Point(moment.x/mass, moment.y/mass, 0);
   return Point(0, 0, 0);
}
    

Point Cluster::GetCentroid()
{
   if (area > 0)
      return Point(sum.x/area, sum.y/area, 0);
   return Point(0, 0, 0);
}


float Cluster::GetMass()
{
   return mass;
}

float Cluster::GetArea()
{
   return area;
}


Point Cluster::GetMin()
{
   return rect_min;
}

Point Cluster::GetMax()
{
   return rect_max;
}

#if 0
Point Cluster::GetMajorAxis()
{
   Point bar, var;
   float covar;

   if (area > 0) {
      bar.x = sum.x/area;
      bar.y = sum.y/area;
      var.x = sq_sum.x/area - bar.x*bar.x;
      var.y = sq_sum.y/area - bar.y*bar.y;
      covar = xy_sum/area - bar.x*bar.y;
      if (co
   }
   return Point(0, 0, 0);
}

Point Cluster::GetMinorAxis()
{
}
#endif 

//===================================================

ClusterMap::ClusterMap(float den, float thr, int sz)
{
   density = den; 
   thresh = thr;
   kern_sz = sz;
}


bool ClusterMap::qualify(DepthFrame d, int x, int y)
{
   float area = 0, count = 0;

   if (IMAGE_AT(d, x, y) > thresh) {
      for (int i = x-kern_sz; i <= x+kern_sz; i++) {
         if (i >= 0 && i < d.size.width) {
            for (int j = y-kern_sz; j <= y+kern_sz; j++) {
               if (j >= 0 && j < d.size.height)  {
                  area = area + 1;
                  if (IMAGE_AT(d, i, j) > thresh)
                     count = count + 1;
               }
            }
         }
      }
      if (count >= density*area)
         return true;
   }
   return false;
}

vector<Cluster> &ClusterMap::GetClusters()
{
   return clusters;
}

DepthFrame &ClusterMap::GetLabelMap()
{
   return c;
}

void ClusterMap::Scan(DepthFrame d)
{
   c.depth.clear();
   c.size = d.size;
   for (int i = 0; i < d.depth.size(); i++)
      c.depth.push_back(UNALLOCATED);

   clusters.clear();
   for (int x = 0; x < d.size.width; x++) {
      for (int y = 0; y < d.size.height; y++) {
         /* if (P > thresh && density_qualify(P) */
         if (qualify(d, x, y)) {
            /* if (!allocated(P) */
            if (IMAGE_AT(c, x, y) == UNALLOCATED) {
               /* iterate N := Neighbor(P) */
               for (int i = x-kern_sz; i <= x+kern_sz; i++) {
                  if (i >= 0 && i < d.size.height) {
                     for (int j = y-kern_sz; j <= y+kern_sz; j++) {
                        if (j >= 0 && j < d.size.height) {
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
               IMAGE_AT(c, x, y) = clusters.size();
               Cluster *cl = new Cluster;
               clusters.push_back(*cl);
               delete cl;
            } /* if (!allocated(P)) */

expand:
            /* iterate N := Neighbor(P) */
            for (int i = x-kern_sz; i <= x+kern_sz; i++) {
               if (i >= 0 && i < d.size.width) {
                  for (int j = y-kern_sz; j <= y+kern_sz; j++) {
                     if (j >= 0 && j < d.size.height) {
                        if (i != x || j != y) {
                           /* if (N > thresh) N.id = P.id */
                           if (IMAGE_AT(d, i, j) > thresh)
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
   for (int x = 0; x < d.size.width; x++) {
      for (int y = 0; y < d.size.height; y++) {
          int cid = (int)(IMAGE_AT(c, x, y));
          if (cid != UNALLOCATED) {
             clusters[cid].AddPoint(Point(x, y, IMAGE_AT(d, x, y)));
          }
      }
   }
}


#undef __CLUSTER_CPP__
/*! @} */





