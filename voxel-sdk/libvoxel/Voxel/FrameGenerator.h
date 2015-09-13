/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */

#ifndef VOXEL_FRAME_GENERATOR_H
#define VOXEL_FRAME_GENERATOR_H

#include <Frame.h>
#include <Common.h>

#include <FrameStream.h>

namespace Voxel
{
  
class VOXEL_EXPORT FrameGenerator
{
protected:
  GeneratorIDType _id;
  int _frameType; // Generated frame type. Value as in Voxel::DepthCamera::FrameType
  FrameStreamWriterPtr _frameStreamWriter;
  
  uint8_t _majorVersion, _minorVersion; // Used for storing/reading configuration
  
  inline size_t _sizeOfVersion() { return sizeof(uint8_t)*2; }
  inline bool _writeVersion(SerializedObject &object);
  inline bool _readVersion(SerializedObject &object);
  
  virtual bool _writeConfiguration(SerializedObject &object) = 0; // Write configuration to serialized data object
  
public:
  FrameGenerator(GeneratorIDType id, int frameType, uint8_t majorVersion, uint8_t minorVersion): _id(id), _frameType(frameType),
  _majorVersion(majorVersion), _minorVersion(minorVersion) {}
  inline const GeneratorIDType &id() const { return _id; }
  
  inline void setFrameStreamWriter(FrameStreamWriterPtr &writer) { _frameStreamWriter = writer; }
  inline void removeFrameStreamWriter() { _frameStreamWriter = nullptr; }
  
  virtual bool readConfiguration(SerializedObject &object) = 0; // Read configuration from serialized data object
  inline bool writeConfiguration(); // Write configuration to FrameStreamWriter
  
  virtual bool generate(const FramePtr &in, FramePtr &out) = 0;
  
  virtual ~FrameGenerator() {}
};

bool FrameGenerator::writeConfiguration()
{
  if(!_frameStreamWriter)
    return true;
  
  auto &p = _frameStreamWriter->getConfigObject();
  
  if(!_writeConfiguration(p))
    return false;
  
  return _frameStreamWriter->writeGeneratorConfiguration(_frameType);
}

bool FrameGenerator::_readVersion(SerializedObject &object)
{
  if(!object.get((char *)&_majorVersion, sizeof(uint8_t)) ||
    !object.get((char *)&_minorVersion, sizeof(uint8_t)))
    return false;
  
  return true;
}

bool FrameGenerator::_writeVersion(SerializedObject &object)
{
  if(object.put((const char *)&_majorVersion, sizeof(uint8_t)) != sizeof(uint8_t) ||
    object.put((const char *)&_minorVersion, sizeof(uint8_t)) != sizeof(uint8_t))
    return false;
  
  return true;
}


typedef Ptr<FrameGenerator> FrameGeneratorPtr;


class VOXEL_EXPORT DepthFrameGenerator: public FrameGenerator
{
public:
  DepthFrameGenerator(GeneratorIDType id, int frameType, uint8_t majorVersion, uint8_t minorVersion): FrameGenerator(id, frameType, majorVersion, minorVersion) {}
  virtual bool setProcessedFrameGenerator(FrameGeneratorPtr &p) = 0;
  
  virtual ~DepthFrameGenerator() {}
};

typedef Ptr<DepthFrameGenerator> DepthFrameGeneratorPtr;
  
}

#endif