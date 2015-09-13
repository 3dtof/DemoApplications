/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */

#ifndef VOXEL_TOF_DEPTH_FRAME_GENERATOR_H
#define VOXEL_TOF_DEPTH_FRAME_GENERATOR_H

#include <FrameGenerator.h>
#include <TI3DToFExports.h>
#include "ToFFrameGenerator.h"

namespace Voxel
{
  
namespace TI
{
  
class TI3DTOF_EXPORT ToFDepthFrameGenerator: public DepthFrameGenerator
{
  float _amplitudeScalingFactor, _depthScalingFactor;
  
  FramePtr _intermediate;
  
  ToFFrameGeneratorPtr _tofFrameGenerator;
  
protected:
  virtual bool _writeConfiguration(SerializedObject &object);
  
public:
  ToFDepthFrameGenerator();
  
  virtual bool generate(const FramePtr &in, FramePtr &out);
  
  bool setParameters(float amplitudeScalingFactor, float depthScalingFactor);
  
  virtual bool setProcessedFrameGenerator (FrameGeneratorPtr &p);
  
  virtual bool readConfiguration(SerializedObject &object);
  
  virtual ~ToFDepthFrameGenerator() {}
};
  
}
}

#endif