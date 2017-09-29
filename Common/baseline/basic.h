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
    void update(ToFRawFrame *frm);
    bool addMat(std::string name, cv::Mat *mat);
    cv::Mat* getMat(std::string);
    void rawFrameToMat(Voxel::ToFRawFrame *frame, cv::Mat &phase, cv::Mat &amplitude);
    void depthFrameToMat(Voxel::DepthFrame *frame, cv::Mat &depth, cv::Mat &amplitude);

protected:
    void _init();

private:
    std::map< std::string, cv::Mat* > _imageMap;
    int _width, _height;
    float _ampGain;
};

#endif // __BASIC_H__
/*! @} */

