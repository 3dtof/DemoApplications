/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */

#include <PointCloudFrameGenerator.h>

#include <DepthCamera.h>

namespace Voxel
{

PointCloudFrameGenerator::PointCloudFrameGenerator():
  FrameGenerator(0, DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME, 0, 1) {}
  
bool PointCloudFrameGenerator::setParameters(uint32_t left, uint32_t top, uint32_t width, uint32_t height, 
                                             uint32_t rowsToMerge, uint32_t columnsToMerge,
                                             float fx, float fy, float cx, float cy, float k1, float k2, float k3, float p1, float p2)
{
  if(_pointCloudTransform)
  {
    if(_pointCloudTransform->left == left && _pointCloudTransform->top == top &&
      _pointCloudTransform->height == height && _pointCloudTransform->width == width &&
      _pointCloudTransform->rowsToMerge == rowsToMerge && _pointCloudTransform->columnsToMerge == columnsToMerge &&
      _pointCloudTransform->fx == fx && _pointCloudTransform->fy == fy &&
      _pointCloudTransform->cx == cx && _pointCloudTransform->cy == cy &&
      _pointCloudTransform->k1 == k1 && _pointCloudTransform->k2 == k2 && _pointCloudTransform->k3 == k3 &&
      _pointCloudTransform->p1 == p1 && _pointCloudTransform->p2 == p2)
      return true; // No need to change anything
  }
  
  if(left < 0 || top < 0 || height < 0 || width < 0 || rowsToMerge < 0 || columnsToMerge < 0)
  {
    logger(LOG_ERROR) << "PointCloudFrameGenerator: Parameters values are invalid. "
      << "left = " << left << ", "
      << "top = " << top << ", "
      << "width = " << width << ", "
      << "height = " << height << ", "
      << "rowsToMerge = " << rowsToMerge << ", "
      << "columnsToMerge = " << columnsToMerge << std::endl;
    return false;
  }
  
  _pointCloudTransform = Ptr<PointCloudTransform>(new PointCloudTransform(left, top, width, height, rowsToMerge, columnsToMerge, fx, fy, cx, cy, k1, k2, k3, p1, p2));
  return writeConfiguration();
}

bool PointCloudFrameGenerator::_writeConfiguration(SerializedObject &object)
{
  if(!_pointCloudTransform)
    return false;
  
  object.resize(9*sizeof(float) + 6*sizeof(uint32_t) + _sizeOfVersion());
  
  if(!_writeVersion(object))
    return false;
  
  object.put((const char *)&_pointCloudTransform->left, sizeof(uint32_t));
  object.put((const char *)&_pointCloudTransform->top, sizeof(uint32_t));
  object.put((const char *)&_pointCloudTransform->width, sizeof(uint32_t));
  object.put((const char *)&_pointCloudTransform->height, sizeof(uint32_t));
  object.put((const char *)&_pointCloudTransform->rowsToMerge, sizeof(uint32_t));
  object.put((const char *)&_pointCloudTransform->columnsToMerge, sizeof(uint32_t));
  
  object.put((const char *)&_pointCloudTransform->fx, sizeof(float));
  object.put((const char *)&_pointCloudTransform->fy, sizeof(float));
  object.put((const char *)&_pointCloudTransform->cx, sizeof(float));
  object.put((const char *)&_pointCloudTransform->cy, sizeof(float));
  object.put((const char *)&_pointCloudTransform->k1, sizeof(float));
  object.put((const char *)&_pointCloudTransform->k2, sizeof(float));
  object.put((const char *)&_pointCloudTransform->k3, sizeof(float));
  object.put((const char *)&_pointCloudTransform->p1, sizeof(float));
  object.put((const char *)&_pointCloudTransform->p2, sizeof(float));
  
  return true;
}

bool PointCloudFrameGenerator::readConfiguration(SerializedObject &object)
{
  if(object.getBytes().size() < 9*sizeof(float) + 6*sizeof(uint32_t) + _sizeOfVersion())
    return false;
  
  uint32_t left, top, width, height, rowsToMerge, columnsToMerge;
  float fx, fy, cx, cy, k1, k2, k3, p1, p2;
  
  if(!_readVersion(object))
    return false;
  
  if(!object.get((char *)&left, sizeof(uint32_t)) ||
  !object.get((char *)&top, sizeof(uint32_t)) ||
  !object.get((char *)&width, sizeof(uint32_t)) ||
  !object.get((char *)&height, sizeof(uint32_t)) ||
  !object.get((char *)&rowsToMerge, sizeof(uint32_t)) ||
  !object.get((char *)&columnsToMerge, sizeof(uint32_t)) ||
  !object.get((char *)&fx, sizeof(float)) ||
  !object.get((char *)&fy, sizeof(float)) ||
  !object.get((char *)&cx, sizeof(float)) ||
  !object.get((char *)&cy, sizeof(float)) ||
  !object.get((char *)&k1, sizeof(float)) ||
  !object.get((char *)&k2, sizeof(float)) ||
  !object.get((char *)&k3, sizeof(float)) ||
  !object.get((char *)&p1, sizeof(float)) ||
  !object.get((char *)&p2, sizeof(float)))
    return false;
  
  return setParameters(left, top, width, height, rowsToMerge, columnsToMerge, fx, fy, cx, cy, k1, k2, k3, p1, p2);
}

bool PointCloudFrameGenerator::generate(const FramePtr &in, FramePtr &out)
{
  const DepthFrame *depthFrame = dynamic_cast<const DepthFrame *>(in.get());
  
  if(!depthFrame)
  {
    logger(LOG_ERROR) << "PointCloudFrameGenerator: Only DepthFrame type is supported as input to generate() function." << std::endl;
    return false;
  }
  
  XYZIPointCloudFrame *f = dynamic_cast<XYZIPointCloudFrame *>(out.get());
  
  if(!f)
  {
    f = new XYZIPointCloudFrame();
    out = FramePtr(f);
  }
  
  f->id = depthFrame->id;
  f->timestamp = depthFrame->timestamp;
  f->points.resize(depthFrame->size.width*depthFrame->size.height);
  
  if(!_pointCloudTransform->depthToPointCloud(depthFrame->depth, *f))
  {
    logger(LOG_ERROR) << "DepthCamera: Could not convert depth frame to point cloud frame" << std::endl;
    return false;
  }
  
  // Setting amplitude as intensity
  auto index = 0;
  
  auto w = depthFrame->size.width;
  auto h = depthFrame->size.height;
  
  for(auto y = 0; y < h; y++)
    for(auto x = 0; x < w; x++, index++)
    {
      IntensityPoint &p = f->points[index];
      p.i = depthFrame->amplitude[index];
    }
    
  return true;
}

  
  
}