/*! 
 * ===================================================================
 *
 *  @addtogroup	   	baseline	
 *  @{
 *
 *  @file 	basic.h
 *
 *  @brief	Basic example class
 *
 *  Copyright (c) 2017 Texas Instruments Inc.
 *
 *====================================================================
 */
#include <math.h>
#include <map>
#include <tuple>
#include <string>
#include "grabber.h"
#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <Eigen/StdVector>

#ifndef __BASIC_H__
#define __BASIC_H__

class Basic : public Grabber
{
public:
    Basic(DepthCameraPtr depthCamera, FrameFlag flag, CameraSystem &sys);
    ~Basic();
    void update(DepthFrame *frm);
    cv::Mat* getAmpMat() { return &_ampMat; }
    cv::Mat* getPhaseMat() { return &_phaseMat; }
    void rawFrameToMat(Voxel::ToFRawFrame *frame, cv::Mat &phase, cv::Mat &amplitude);
    bool depthFrameToMat(Voxel::DepthFrame *frame, cv::Mat &depth, cv::Mat &amplitude);

protected:
    void _init();

private:
    cv::Mat _ampMat;
    cv::Mat _phaseMat;
    int _width, _height;
    float _ampGain;
};

#endif // __BASIC_H__
/*! @} */

