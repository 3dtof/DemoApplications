/*!
 *****************************************************************************
 *
 * @addtogroup         cluster
 * @{
 *
 * @file                cluster.c
 * @version             1.0
 * @date                8/22/2012
 *
 * @note                DBSCAN cluster algorithm
 *
 * Copyright© 2000-2012 Texas Instruments Corporation, All Rights Reserved.
 * TI makes NO WARRANTY as to software products, which are supplied "AS-IS"
 *
 *****************************************************************************
 */
#define __CLUSTER_C__

#include "sdk_types.h"
#include "touch.h"
#include "matrix.h"
#include "cluster.h"
#include "panel.h"


/*!
 *****************************************************************************
 *
 * @fn          cluster_t *cluster_new(UINT8_t num)
 *
 * @brief       Create a cluster algorithm component
 *
 * @param       num     Number of clusters to create
 *
 * @return      Pointer to the cluster object
 *
 *****************************************************************************
 */
cluster_t *cluster_new(UINT8_t num)
{
   cluster_t *cl = (cluster_t *)calloc(num, sizeof(cluster_t));
   return cl;
}

/*!
 *****************************************************************************
 *
 * @fn          errcode_t cluster_del(cluster_t *cl)
 *
 * @brief       Delete a cluster algorithm component
 *
 * @return      SDK_ERR or SDK_OK
 *
 *****************************************************************************
 */
errcode_t cluster_del(cluster_t *cl)
{
   SDK_ASSERT(cl == NULL, SDK_ERR);
   free(cl);

   return SDK_OK;
}

/*!
 *****************************************************************************
 *
 * @fn          errcode_t cluster_init(cluster_t *cl, UINT8_t num)
 *
 * @brief       Init an instance of cluster component
 *
 * @param       cl      Cluster instance
 * @param       num     Number of clusters
 *
 * @return      SDK_OK or SDK_ERR
 *
 *****************************************************************************
 */
errcode_t cluster_init(cluster_t *cl, UINT8_t num)
{
   UINT8_t n;

   SDK_ASSERT(cl == NULL, SDK_ERR);

   for (n = 0; n < num; n++) {
        cl[n].id = n;
        cl[n].area = 0;
        cl[n].mass = 0;
        cl[n].x_moment = 0;
        cl[n].y_moment = 0;
        cl[n].x_sum = 0;
        cl[n].y_sum = 0;
        cl[n].xsq_sum = 0;
        cl[n].ysq_sum = 0;
        cl[n].xy_sum = 0;
        cl[n].npeak = 0;
        cl[n].angle = 0;
   }
   return SDK_OK;
}

/*!
 *=============================================================================
 *
 *  @fn         errcode_t cluster_get_cg(cluster_t *cl, qmath_t *x_cg, qmath_t *y_cg)
 *
 *  @brief      Calc the cluster's Center of Gravity
 *
 *  @param      cl          Cluster pointer
 *  @param      disp        Display dimensions
 *  @param      sensor      Sensor dimensions
 *  @param      x_cg        CG point X
 *  @param      y_cg        CG point Y
 *
 *  @return     SDK_OK or SDK_ERR
 *
 *=============================================================================
 */
errcode_t cluster_get_cg(cluster_t *cl, qmath_t *x_cg, qmath_t *y_cg)
{
   SDK_ASSERT(cl == NULL || x_cg == NULL || y_cg == NULL, SDK_ERR);

   *x_cg = qmath_div(QMATH_INT2Q(cl->x_moment), QMATH_INT2Q(cl->mass));
   *y_cg = qmath_div(QMATH_INT2Q(cl->y_moment), QMATH_INT2Q(cl->mass));

   return SDK_OK;
}

/*!
 *=============================================================================
 *
 *  @fn         errcode_t cluster_get_centroid(cluster_t *cl,
 *                                      qmath_t *x_c, qmath_t *y_c)
 *
 *  @brief      Calc the cluster's Centroid
 *
 *  @param      cl          Cluster pointer
 *  @param      disp        Display dimensions
 *  @param      sensor      Sensor dimensions
 *  @param      x_c         Centroid point X
 *  @param      y_c         Centroid point Y
 *
 *  @return     SDK_OK or SDK_ERR
 *
 *=============================================================================
 */
errcode_t cluster_get_centroid(cluster_t *cl, qmath_t *x_c, qmath_t *y_c)
{
   SDK_ASSERT(cl == NULL || x_c == NULL || y_c == NULL, SDK_ERR);

   *x_c = qmath_div(QMATH_INT2Q(cl->x_sum), QMATH_INT2Q(cl->area));
   *y_c = qmath_div(QMATH_INT2Q(cl->y_sum), QMATH_INT2Q(cl->area));

   return SDK_OK;
}

/*!
 *=============================================================================
 *
 * @fn        errcode_t cluster_add_point(cluster_t *cl,
 *                           matrix_t *mi, matrix_t *mc,
 *                           INT16_t row, INT16_t col)
 *
 *  @brief      Add point(row,col) to cluster 'cl', only if the point
 *              is unallocated.
 *
 *  @param      cl          Pointer to cluster
 *  @param      mi          Input z-value array
 *  @param      mc          Input cid array
 *  @param      row         Row of the point
 *  @param      col         Col of the point
 *
 *  @return     SDK_OK or SDK_ERR
 *
 *=============================================================================
 */
errcode_t cluster_add_point(cluster_t *cl, matrix_t *mi, matrix_t *mc,
                                           INT16_t row, INT16_t col)
{
    INT16_t zval;

    SDK_ASSERT(cl == NULL, SDK_ERR);

    cl->area++;
    zval = MATRIX(mi,row,col);
    cl->mass += zval;
    cl->x_moment += zval*(row);
    cl->y_moment += zval*(col);
    cl->x_sum += row;
    cl->y_sum += col;
    cl->xsq_sum += row*row;
    cl->ysq_sum += col*col;
    cl->xy_sum += row*col;

    return SDK_OK;
}

/*!
 *=============================================================================
 *
 *  @fn         UINT8_t cluster_qualify(matrix_t *mi, INT16_t row,
 *                      INT16_t col, INT16_t density, INT16_t thresh)
 *
 *  @brief      Check if current point meets density requirement
 *
 *  @param      min         Input touchmap
 *  @param      row         Node row index
 *  @param      col         Node col index
 *  @param      density     Density to qualify (0 - 100)
 *  @param      thresh      Threshold to qualify for consideration
 *
 *  @return     1 is qualified; 0 otherwise
 *
 *=============================================================================
 */
UINT8_t cluster_qualify(matrix_t *mi, INT16_t row, INT16_t col,
                        INT16_t density, INT16_t thresh)
{
   INT16_t i, j;
   INT16_t area, count;

   area = count = 0;

   if (MATRIX(mi,row,col) > thresh) {
      for (i = row-EPS; i <= row+EPS; i++) {
         if (i >= 0 && i < mi->nrow) {
            for (j = col-EPS; j <= col+EPS; j++) {
               if (j >= 0 && j < mi->ncol)  {
                  area++;
                  if (MATRIX(mi,i,j) > thresh)
                     count++;
               }
            }
         }
      }
      if (count*100 >= density*area)
         return 1;
   }
   return 0;
}


/*!
 *=============================================================================
 *
 *  @fn         errcode_t cluster_process(cluster_t *cl,
 *                          matrix_t *mi, matrix_t *mc,
 *                          INT16_t density,
 *                          INT16_t cluster_thresh,
 *                          INT16_t peak_thresh,
 *                          UINT8_t max_cluster,
 *                          touch_t *peak,
 *                          UINT8_t *npeak)
 *
 *  @brief      Perform cluster processing
 *
 *  @param      cl          Cluster pointer
 *              mi          Input touch map
 *              mc          scratch cluster map
 *  `           density     Density (0 - 100 percent)
 *              thresh      Z threshold to qualify
 *              peak        Raw local peaks array pointer)
 *              npeak       Number of raw peaks found
 *
 *  @return     SDK_OK or SDK_ERR
 *
 *=============================================================================
 */
errcode_t cluster_process(cluster_t *cl, matrix_t *mi, matrix_t *mc,
                          INT16_t density, INT16_t cluster_thresh, INT16_t peak_thresh,
                          UINT8_t *num_cluster, UINT8_t max_cluster,
                          touch_t *peak, UINT8_t *npeak)
{
   INT16_t row, col;
   INT16_t i, j, is_local_max;
   UINT8_t cid=1, n;
   qmath_t xbar,ybar,xvar,yvar,covar,varavg,disc_half,root_half;

   SDK_ASSERT(cl == NULL || mi == NULL || mc == NULL || peak == NULL || npeak == NULL, SDK_ERR);

   *npeak = 0;

   /* Iterate P:= All Points */
   for (row = 0; row < mi->nrow; row++)  {
      for (col = 0; col < mi->ncol; col++)  {

         /* if (P > thresh && density_qualify(P) */
         if (cluster_qualify(mi, row, col, density, cluster_thresh)) {
            /* if (!allocated(P) */
            if (MATRIX(mc,row,col) == UNALLOCATED) {
               /* iterate N := Neighbor(P) */
               for (i = row-EPS; i <= row+EPS; i++) {
                  if (i >= 0 && i < mi->nrow) {
                     for (j = col-EPS; j <= col+EPS; j++) {
                        if (j >= 0 && j < mi->ncol) {
                           if (i != row || j != col) {
                              /* if (allocated(N)) */
                              if (MATRIX(mc,i,j) != UNALLOCATED) {
                                 /* if density_qualify(N) */
                                 if (cluster_qualify(mi, i, j, density, cluster_thresh)) {
                                    /* P.id = N.id */
                                    MATRIX(mc, row, col) = MATRIX(mc, i, j);
                                    goto expand;
                                 }
                              }
                           }
                        }
                     }
                  }
               }
               /* P.id = new(id) */
               if (cid < max_cluster)
                    MATRIX(mc,row,col) = cid++;
            } /* if (!allocated(P)) */

expand:
            /* Setup for local max detection */
            is_local_max = 1;

            /* iterate N := Neighbor(P) */
            for (i = row-EPS; i <= row+EPS; i++) {
               if (i >= 0 && i < mi->nrow) {
                  for (j = col-EPS; j <= col+EPS; j++) {
                     if (j >= 0 && j < mi->ncol) {
                        if (i != row || j != col) {
                           /* if (N > P) then is local_max = 0 */
                           if (MATRIX(mi, row, col) < MATRIX(mi, i ,j))
                              is_local_max = 0;

                           /* Force local max if neighbor is same height */
                           if (MATRIX(mi, row, col) == MATRIX(mi, i, j))
                                MATRIX(mi, i, j) = MATRIX(mi, i, j) - 1;

                           /* if (N > thresh) N.id = P.id */
                           if (MATRIX(mi, i, j) > cluster_thresh)
                              MATRIX(mc, i, j) = MATRIX(mc, row, col);
                        }
                     }
                  }
               }
            }

            if (is_local_max) {
                if ((*npeak < MAX_TOUCHES) && (MATRIX(mi,row,col) > peak_thresh)) {
                    peak[*npeak].point.x = row;
                    peak[*npeak].point.y = col;
                    peak[*npeak].contact.zval = MATRIX(mi, row, col);
                    *npeak = *npeak+1;
                }
            }

         }  /* if (P > thresh && density_qualify(P) */
      }   /* iterate P := All Points */
    }

    /* Compute cluster statistics */
    *num_cluster = 0;
    for (row = 0; row < mi->nrow; row++) {
        for (col = 0; col < mi->ncol; col++) {
            cid = MATRIX(mc,row,col);
            if (cid != UNALLOCATED && cid < max_cluster) {
                if (cl[cid].area == 0)
                    *num_cluster = *num_cluster + 1;
                cluster_add_point(&cl[cid],mi,mc,row,col);
            }
        }
    }

    /* Compute directions */
    for (cid = 1; cid <= *num_cluster; cid++){
        /* It is meaningless if area < 2 or if peaks in a cluster != 1 */
        if (cl[cid].area > 2){
            xbar = qmath_div(QMATH_INT2Q(cl[cid].x_sum),QMATH_INT2Q(cl[cid].area));
            ybar = qmath_div(QMATH_INT2Q(cl[cid].y_sum),QMATH_INT2Q(cl[cid].area));
            xvar = qmath_sub(qmath_div(QMATH_INT2Q(cl[cid].xsq_sum),QMATH_INT2Q(cl[cid].area)),qmath_mul(xbar,xbar));
            yvar = qmath_sub(qmath_div(QMATH_INT2Q(cl[cid].ysq_sum),QMATH_INT2Q(cl[cid].area)),qmath_mul(ybar,ybar));
            covar = qmath_sub(qmath_div(QMATH_INT2Q(cl[cid].xy_sum),QMATH_INT2Q(cl[cid].area)),qmath_mul(xbar,ybar));
            if (covar == 0){
                if (yvar > xvar){
                    //cl[cid].angle = (UINT8_t)(RIGHT_ANGLE<<QMATH_FRACBITS); /* TODO: this is a bug -- see vyass */
                    cl[cid].angle = (UINT8_t)RIGHT_ANGLE;
                }
            }
            else{
                //varavg = qmath_div(xvar+yvar,QMATH_INT2Q(2));
                varavg = (xvar+yvar)>>1;
                disc_half = qmath_sub(qmath_mul(varavg,varavg),qmath_mul(xvar,yvar) - qmath_mul(covar,covar));
                root_half = qmath_sqrt(disc_half);
                cl[cid].angle = QMATH_IPART(qmath_arctan(qmath_div(varavg + root_half - xvar, covar)));
            }
        }
    }

    /* Fill peak data */
    for (n = 0; n < *npeak; n++) {
        cid = MATRIX(mc, peak[n].point.x, peak[n].point.y);
        peak[n].contact.clid = cid;
        peak[n].contact.area = cl[cid].area;
        peak[n].contact.mass = cl[cid].mass;
        peak[n].contact.angle = cl[cid].angle;
        cl[cid].npeak++;
    }

    return SDK_OK;
}

#undef __CLUSTER_C__
/*! @} */
