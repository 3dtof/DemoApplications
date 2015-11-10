/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */

#ifndef VOXEL_TOF_FRAME_GENERATOR_H
#define VOXEL_TOF_FRAME_GENERATOR_H

#include <TI3DToFExports.h>
#include "ToFCrossTalkFilter.h"
#include <FrameGenerator.h>

namespace Voxel
{

namespace TI
{
  
class TI3DTOF_EXPORT ToFFrameGenerator: public FrameGenerator
{
  uint32_t _bytesPerPixel, _dataArrangeMode;
  
  RegionOfInterest _roi;
  FrameSize _maxFrameSize, _size;
  uint32_t _rowsToMerge, _columnsToMerge;
  
  uint8_t _histogramEnabled;
  
  String _phaseOffsetFileName;
  Vector<int16_t> _phaseOffsetCorrectionData;
  
  ToFFrameType _frameType;
  
  ToFCrossTalkFilterPtr _crossTalkFilter;
  String _crossTalkCoefficients;
  FramePtr _filterInputFrame;
  
protected:
  virtual bool _writeConfiguration(SerializedObject &object);
  
  virtual bool _createCrossTalkFilter();
  virtual bool _applyCrossTalkFilter(FramePtr &out);
  virtual bool _readPhaseOffsetCorrection();
  virtual bool _applyPhaseOffsetCorrection(Vector<uint16_t> &phaseData);
  
  bool _generateToFRawFrame(const FramePtr &in, FramePtr &out);
  bool _generateToFRawIQFrame(const FramePtr &in, FramePtr &out);
  
public:
  ToFFrameGenerator();
  
  virtual bool generate(const ToFRawIQFramePtr &in, FramePtr &out); // Convert IQ to amplitude-phase
  virtual bool generate(const FramePtr &in, FramePtr &out);
  
  bool setParameters(const String &phaseOffsetFileName, uint32_t bytesPerPixel, 
                     uint32_t dataArrangeMode,
                     const RegionOfInterest &roi, const FrameSize &maxFrameSize, 
                     uint rowsToMerge, uint columnsToMerge,
                     uint8_t histogramEnabled, 
                     const String &crossTalkCoefficients, ToFFrameType type);
  
  virtual bool readConfiguration(SerializedObject &object);
  
  virtual ~ToFFrameGenerator() {}
};

typedef Ptr<ToFFrameGenerator> ToFFrameGeneratorPtr;
  
}
}

#endif