/*! 
 * ===================================================================
 *
 *  @addtogroup	   	baseline	
 *  @{
 *
 *  @file 	basic.cpp
 *
 *  @brief	Basic example class
 *
 *  Copyright (c) 2017 Texas Instruments Inc.
 *
 *====================================================================
 */
#define __BASIC_CPP__
#include "basic.h"

Basic::Basic(DepthCameraPtr dc, FrameFlag flag, CameraSystem &sys) : Grabber(dc, flag, sys)
{
    _init();
}

Basic::~Basic()
{
}

void Basic::update(DepthFrame *frm)
{
    if (frm)
        depthFrameToMat(frm, _phaseMat, _ampMat); 
}


void Basic::_init()
{
    getFrameSize(_height, _width);  
    _ampMat = cv::Mat::zeros(_height, _width, CV_32FC1);
    _phaseMat = cv::Mat::zeros(_height, _width, CV_32FC1);
}


void Basic::rawFrameToMat(ToFRawFrame *frame, cv::Mat &phase, cv::Mat &amplitude)
{
    for (auto i=0; i<_height; i++ )
    {
        for (auto j=0; j<_width; j++ )
        {
            int idx = i * _width +j;
            if (frame->phaseWordWidth() == 2)
                //phase.at<float>(i,j) = (float)(((uint16_t *)frame->phase())[idx])/4095.0;
                phase.at<float>(i,j) = (float)(((uint16_t *)frame->phase())[idx])/8192.0;
            else if (frame->phaseWordWidth() == 1)
                phase.at<float>(i,j) = (float)(((uint8_t *)frame->phase())[idx])/255.0;

            if (frame->amplitudeWordWidth() == 2)
                //amplitude.at<float>(i,j) = (float)(((uint16_t *)frame->amplitude())[idx])/4095.0;
                amplitude.at<float>(i,j) = (float)(((uint16_t *)frame->amplitude())[idx])/8192.0;
            else if (frame->amplitudeWordWidth() == 1)
                amplitude.at<float>(i,j) = (float)(((uint8_t *)frame->amplitude())[idx])/255.0;

            for (int k = 0; k < 8; k++)
                for (int m = 0; m < 8; m++)
                    amplitude.at<float>(k, m) = 0;
        }
    }
}


bool Basic::depthFrameToMat(Voxel::DepthFrame *frame, cv::Mat &depth, cv::Mat &amplitude)
{
    bool ret = false;

    for (auto i=0; i<_height; i++ )
    {
        for (auto j=0; j<_width; j++ )
        {
            int idx = i * _width +j;
            depth.at<float>(i,j) = frame->depth[idx];
            amplitude.at<float>(i,j) = frame->amplitude[idx];
        }
    }
    return ret;
}

#undef __BASIC_CPP__
/*! @} */
