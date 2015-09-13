/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */

#include <ToFFrameGenerator.h>

#include <ToFCamera.h>

#define _MATH_DEFINES
#include <math.h>

#define MAX_PHASE_VALUE 0x0FFF
#define MAX_PHASE_RANGE 0x1000
#define TOF_ANGLE_TO_PHASE_FACTOR 4096/(2*M_PI)
#define IQ_SIGN_BIT 0x0800
#define IQ_SIGN_EXTEND 0xF000

namespace Voxel
{
  
namespace TI
{
  
ToFFrameGenerator::ToFFrameGenerator(): 
  FrameGenerator((TI_VENDOR_ID << 16) | DepthCamera::FRAME_RAW_FRAME_PROCESSED, DepthCamera::FRAME_RAW_FRAME_PROCESSED, 0, 1),
_bytesPerPixel(-1), _dataArrangeMode(-1), _histogramEnabled(false)
{
}

bool ToFFrameGenerator::_writeConfiguration(SerializedObject &object)
{
  if(_bytesPerPixel == -1)
    return false;
  
  uint16_t size = _phaseOffsetFileName.size(), size2 = _crossTalkCoefficients.size();
  
  object.resize(sizeof(uint32_t)*2 
    + sizeof(_maxFrameSize.width)*2 + sizeof(_roi.x)*4 + sizeof(_rowsToMerge)*2
    + sizeof(_histogramEnabled) + sizeof(size) + size 
    + sizeof(size2) + size2 + sizeof(uint8_t) + _sizeOfVersion());
  
  if(!_writeVersion(object))
    return false;
  
  object.put((const char *)&size, sizeof(size));
  
  if(size)
    object.put(_phaseOffsetFileName.c_str(), size);
  
  object.put((const char *)&size2, sizeof(size2));
  
  if(size2)
    object.put(_crossTalkCoefficients.c_str(), size2);
  
  object.put((const char *)&_bytesPerPixel, sizeof(uint32_t));
  object.put((const char *)&_dataArrangeMode, sizeof(uint32_t));
  object.put((const char *)&_maxFrameSize.width, sizeof(_maxFrameSize.width));
  object.put((const char *)&_maxFrameSize.height, sizeof(_maxFrameSize.height));
  object.put((const char *)&_roi.x, sizeof(_roi.x));
  object.put((const char *)&_roi.y, sizeof(_roi.y));
  object.put((const char *)&_roi.width, sizeof(_roi.width));
  object.put((const char *)&_roi.height, sizeof(_roi.height));
  object.put((const char *)&_rowsToMerge, sizeof(_rowsToMerge));
  object.put((const char *)&_columnsToMerge, sizeof(_columnsToMerge));
  object.put((const char *)&_histogramEnabled, sizeof(_histogramEnabled));
  
  uint8_t x = _frameType;
  object.put((const char *)&x, sizeof(x));
  
  return true;
}

bool ToFFrameGenerator::readConfiguration(SerializedObject &object)
{
  uint16_t size;
  
  if(object.size() < sizeof(uint32_t)*2 + 
    sizeof(_maxFrameSize.width)*2 + sizeof(_roi.x)*4 + sizeof(_rowsToMerge)*2 + 
    sizeof(_histogramEnabled) + sizeof(size)*2 + sizeof(uint8_t) + _sizeOfVersion())
    return false;
  
  if(!_readVersion(object))
    return false;
  
  size = _phaseOffsetFileName.size();
  
  if(!object.get((char *)&size, sizeof(size)))
    return false;
  
  if(size)
  {
    _phaseOffsetFileName.resize(size);
    
    if(!object.get((char *)_phaseOffsetFileName.data(), size))
      return false;
    
    bool ret = _readPhaseOffsetCorrection();
    
    if(!ret)
      return false;
  }
  
  size = _crossTalkCoefficients.size();
  
  if(!object.get((char *)&size, sizeof(size)))
    return false;
  
  if(size)
  {
    _crossTalkCoefficients.resize(size);
    
    if(!object.get((char *)_crossTalkCoefficients.data(), size))
      return false;
    
    if(!_createCrossTalkFilter())
      return false;
  }
  
  uint8_t x;
  
  if(!object.get((char *)&_bytesPerPixel, sizeof(uint32_t)) ||
  !object.get((char *)&_dataArrangeMode, sizeof(uint32_t)) ||
  !object.get((char *)&_maxFrameSize.width, sizeof(_maxFrameSize.width)) ||
  !object.get((char *)&_maxFrameSize.height, sizeof(_maxFrameSize.height)) ||
  !object.get((char *)&_roi.x, sizeof(_roi.x)) ||
  !object.get((char *)&_roi.y, sizeof(_roi.y)) ||
  !object.get((char *)&_roi.width, sizeof(_roi.width)) ||
  !object.get((char *)&_roi.height, sizeof(_roi.height)) ||
  !object.get((char *)&_rowsToMerge, sizeof(_rowsToMerge)) ||
  !object.get((char *)&_columnsToMerge, sizeof(_columnsToMerge)) ||
  !object.get((char *)&_histogramEnabled, sizeof(_histogramEnabled)) ||
  !object.get((char *)&x, sizeof(x)))
    return false;
    
  _frameType = (ToFFrameType)x;
  
  return true;
}
bool ToFFrameGenerator::setParameters(const String &phaseOffsetFileName, uint32_t bytesPerPixel, 
                                      uint32_t dataArrangeMode, 
                                      const RegionOfInterest &roi, const FrameSize &maxFrameSize, 
                                      uint rowsToMerge, uint columnsToMerge,
                                      uint8_t histogramEnabled, const String &crossTalkCoefficients,
                                      ToFFrameType type)
{
  if(_phaseOffsetFileName == phaseOffsetFileName && bytesPerPixel == _bytesPerPixel && 
    _dataArrangeMode == dataArrangeMode && 
    _maxFrameSize == maxFrameSize && _roi == roi && _rowsToMerge == rowsToMerge && _columnsToMerge == columnsToMerge
    && _histogramEnabled == histogramEnabled &&
    _crossTalkCoefficients == crossTalkCoefficients && _frameType == type)
    return true;
  
  _phaseOffsetFileName = phaseOffsetFileName;
  
  if(!_readPhaseOffsetCorrection())
    return false;
  
  _bytesPerPixel = bytesPerPixel;
  _dataArrangeMode = dataArrangeMode;
  _maxFrameSize = maxFrameSize;
  _roi = roi;
  _rowsToMerge = rowsToMerge;
  _columnsToMerge = columnsToMerge;
  
  if(_roi.x < 0 || _roi.y < 0 || _roi.width < 0 || _roi.height < 0 || _maxFrameSize.width < 0
    || _maxFrameSize.height < 0 ||
    _roi.x + _roi.width > _maxFrameSize.width || _roi.y + _roi.height > _maxFrameSize.height ||
    _rowsToMerge < 1 || _columnsToMerge < 1)
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Incorrect ROI or maxFrameSize or rowsToMerge or columnsToMerge" 
      << "ROI = (" << _roi.x << ", " << _roi.y << ", " << _roi.width << ", " << _roi.height << "), "
      << "maxFrameSize = (" << _maxFrameSize.width << ", " << _maxFrameSize.height << "), "
      << "rowsToMerge = " << _rowsToMerge << ", columnsToMerge = " << _columnsToMerge << std::endl;
    return false;
  }
  
  _size.width = _roi.width/_columnsToMerge;
  _size.height = _roi.height/_rowsToMerge;
  
  _histogramEnabled = histogramEnabled;
  _crossTalkCoefficients = crossTalkCoefficients;
  _frameType = type;
  
  if(!_createCrossTalkFilter())
    return false;
  
  return writeConfiguration();
}

bool ToFFrameGenerator::_createCrossTalkFilter()
{
  if(_crossTalkCoefficients.size())
  {
    _crossTalkFilter = ToFCrossTalkFilterPtr(new ToFCrossTalkFilter());
    
    if(!_crossTalkFilter->readCoefficients(_crossTalkCoefficients))
    {
      _crossTalkFilter = nullptr;
      
      logger(LOG_ERROR) << "ToFFrameGenerator: Failed to read cross talk filter coefficients. Coefficients = "
      << _crossTalkCoefficients << std::endl;
      
      return false;
    }
    
    if(!_crossTalkFilter->setMaxPhaseRange(MAX_PHASE_RANGE))
      return false;
  }
  else
    _crossTalkFilter = nullptr;
  
  return true;
}


bool ToFFrameGenerator::_readPhaseOffsetCorrection()
{
  if(!_phaseOffsetFileName.size())
    return true;
  
  Configuration c;
  
  String file = _phaseOffsetFileName;
  
  if(!c.getConfFile(file))
    return false;
  
  InputFileStream f(file, std::ios::binary | std::ios::ate);
  
  if(!f.good())
    return false;
  
  size_t size = f.tellg();
  
  _phaseOffsetCorrectionData.resize(size/2); // uint16_t data
  
  f.seekg(0, std::ios::beg);
  f.clear();
  
  if(!f.good())
    return false;
  
  f.read((char *)_phaseOffsetCorrectionData.data(), size);
  return true;
}

bool ToFFrameGenerator::_applyPhaseOffsetCorrection(Vector<uint16_t> &phaseData)
{
  if(!_phaseOffsetCorrectionData.size())
    return true; // Nothing to do
    
  if(_phaseOffsetCorrectionData.size() != _maxFrameSize.height*_maxFrameSize.width ||
    phaseData.size() != (_roi.width/_columnsToMerge)*(_roi.height/_rowsToMerge))
    return false;
  
  int i = _roi.height/_rowsToMerge, j;
  
  int16_t v;
  uint16_t *d = phaseData.data();
  int16_t *o = _phaseOffsetCorrectionData.data() + _roi.x + _roi.y*_maxFrameSize.width, *o1;
  o1 = o;
  
  
  while(i--)
  {
    j = _roi.width/_columnsToMerge;
    while(j--)
    {
      v = *d - *o;
      if(v < 0)
        *d = 0;
      else if(v >= MAX_PHASE_VALUE + 1)
        *d = MAX_PHASE_VALUE;
      else
        *d = v;
      d++;
      o += _columnsToMerge;
    }
    o1 += _rowsToMerge*_maxFrameSize.width;
    o = o1;
  }
  
  return true;
}

bool ToFFrameGenerator::generate(const FramePtr &in, FramePtr &out)
{
  if(_frameType == ToF_I_Q)
    return _generateToFRawIQFrame(in, out);
  else
    return _generateToFRawFrame(in, out);
}

bool ToFFrameGenerator::_generateToFRawFrame(const FramePtr &in, FramePtr &out)
{
  RawDataFramePtr rawDataFrame = std::dynamic_pointer_cast<RawDataFrame>(in);
  
  if(!rawDataFrame)
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Input data frame is not of raw data type." << std::endl;
    return false;
  }
  
  ToFRawFrameTemplate<uint16_t, uint8_t> *t;
  if(!_crossTalkFilter)
  {
    t = dynamic_cast<ToFRawFrameTemplate<uint16_t, uint8_t> *>(out.get());
    
    if(!t)
    {
      t = new ToFRawFrameTemplate<uint16_t, uint8_t>();
      out = FramePtr(t);
    }
  }
  else
  {
    t = dynamic_cast<ToFRawFrameTemplate<uint16_t, uint8_t> *>(_filterInputFrame.get());
    
    if(!t)
    {
      t = new ToFRawFrameTemplate<uint16_t, uint8_t>();
      _filterInputFrame = FramePtr(t);
    }
  }
  
  t->size = _size;
  t->id = rawDataFrame->id;
  t->timestamp = rawDataFrame->timestamp;
  
  if(_bytesPerPixel == 4)
  {
    if(rawDataFrame->data.size() < _size.height*_size.width*4)
    {
      logger(LOG_ERROR) << "ToFFrameGenerator: Incomplete raw data size = " << rawDataFrame->data.size() << ". Required size = " << _size.height*_size.width*4 << std::endl;
      return false;
    }
    
    if(_dataArrangeMode != 2 && _dataArrangeMode != 0)
    {
      logger(LOG_ERROR) << "ToFFrameGenerator: Invalid op_data_arrange_mode = " << _dataArrangeMode << " for pixel_data_size = " << _bytesPerPixel << std::endl;
      return false;
    }
    
    t->_ambient.resize(_size.width*_size.height);
    t->_amplitude.resize(_size.width*_size.height);
    t->_phase.resize(_size.width*_size.height);
    t->_flags.resize(_size.width*_size.height);
    
    if(_dataArrangeMode == 2)
    {
      auto index1 = 0, index2 = 0;
      
      uint16_t *data = (uint16_t *)rawDataFrame->data.data();
      
      for (auto i = 0; i < _size.height; i++) 
      {
        for (auto j = 0; j < _size.width/8; j++) 
        {
          index1 = i*_size.width*2 + j*16;
          index2 = i*_size.width + j*8;
          
          //logger(INFO) << "i = " << i << ", j = " << j << ", index1 = " << index1 << ", index2 = " << index2 << std::endl;
          
          for (auto k = 0; k < 8; k++) 
          {
            t->_amplitude[index2 + k] = data[index1 + k] & MAX_PHASE_VALUE;
            t->_ambient[index2 + k] = (data[index1 + k] & 0xF000) >> 12;
            
            t->_phase[index2 + k] = data[index1 + k + 8] & MAX_PHASE_VALUE;
            t->_flags[index2 + k] = (data[index1 + k + 8] & 0xF000) >> 12;
          }
        }
      }
    }
    else // dataArrangeMode == 0
    {
      auto index1 = 0, index2 = 0;
      
      uint16_t *data = (uint16_t *)rawDataFrame->data.data();
      
      for (auto i = 0; i < _size.height; i++) 
      {
        for (auto j = 0; j < _size.width; j++) 
        {
          index1 = i*_size.width*2 + j*2;
          index2 = i*_size.width + j;
          t->_amplitude[index2] = data[index1] & MAX_PHASE_VALUE;
          t->_ambient[index2] = (data[index1] & 0xF000) >> 12;
          
          t->_phase[index2] = data[index1 + 1] & MAX_PHASE_VALUE;
          t->_flags[index2] = (data[index1 + 1] & 0xF000) >> 12;
        }
      }
    }
  }
  else if(_bytesPerPixel == 2)
  {
    if(_dataArrangeMode != 0)
    {
      logger(LOG_ERROR) << "ToFFrameGenerator: " << OP_DATA_ARRANGE_MODE << " is expected to be zero, but got = " << _dataArrangeMode << " for " << PIXEL_DATA_SIZE << " = " << _bytesPerPixel << std::endl;
      return false;
    }
    
    if(rawDataFrame->data.size() < _size.height*_size.width*2)
    {
      logger(LOG_ERROR) << "ToFFrameGenerator: Incomplete raw data size = " << rawDataFrame->data.size() << ". Required size = " << _size.height*_size.width*2 << std::endl;
      return false;
    }
    
    t->_ambient.resize(_size.width*_size.height);
    t->_amplitude.resize(_size.width*_size.height);
    t->_phase.resize(_size.width*_size.height);
    t->_flags.resize(_size.width*_size.height);
    
    auto index = 0;
    
    uint16_t *data = (uint16_t *)rawDataFrame->data.data();
    
    for (auto i = 0; i < _size.height; i++) 
    {
      for (auto j = 0; j < _size.width; j++) 
      {
        index = i*_size.width + j;
        t->_phase[index] = data[index] & MAX_PHASE_VALUE;
        t->_amplitude[index] = (data[index] & 0xF000) >> 4; // Amplitude information is MS 4-bits
        
        t->_ambient[index] = 0;
        t->_flags[index] = 0;
      }
    }
  }
  else
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Don't know to handle " << PIXEL_DATA_SIZE << " = " << _bytesPerPixel << std::endl;
    return false;
  }
  
  if(!_applyCrossTalkFilter(out))
    return false;
  
  
  t = dynamic_cast<ToFRawFrameTemplate<uint16_t, uint8_t> *>(out.get());
  
  if(!t)
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Output frame is not a ToF frame!" << std::endl;
    return false;
  }
  
  if(!_applyPhaseOffsetCorrection(t->_phase))
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Failed to apply phase offset correction" << std::endl;
    return false;
  }
  
  if(!_histogramEnabled)
    return true; // No histogram data
    
  if(rawDataFrame->data.size() < _size.height*_size.width*_bytesPerPixel + 96)
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Histogram is enabled but raw data has less than 96 bytes at the end. Raw data size = " << rawDataFrame->data.size() 
    << ", bytes in frame = " << _size.height*_size.width*_bytesPerPixel << std::endl;
    return false;
  }
  
  uint8_t *data = rawDataFrame->data.data() + _size.height*_size.width*_bytesPerPixel;
  
  t->_histogram.resize(48); // 48 elements of 16-bits each
  
  memcpy((uint8_t *)t->_histogram.data(), data, 96);
  
  return true;
}

bool ToFFrameGenerator::generate(const ToFRawIQFramePtr &in, FramePtr &out)
{
  ToFRawIQFrameTemplate<int16_t> *input = dynamic_cast<ToFRawIQFrameTemplate<int16_t> *>(in.get());
  
  if(!input)
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Input data frame is not of raw ToF IQ type." << std::endl;
    return false;
  }
  
  if(input->size != _size)
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Input data size differs from expected size. Input size = " 
      << input->size.width << "x" << input->size.height 
      << ", expected size = " << _size.width << "x" << _size.height
      << std::endl;
    return false;
  }
  
  ToFRawFrameTemplate<uint16_t, uint8_t> *t;
  if(!_crossTalkFilter)
  {
    t = dynamic_cast<ToFRawFrameTemplate<uint16_t, uint8_t> *>(out.get());
    
    if(!t)
    {
      t = new ToFRawFrameTemplate<uint16_t, uint8_t>();
      out = FramePtr(t);
    }
  }
  else
  {
    t = dynamic_cast<ToFRawFrameTemplate<uint16_t, uint8_t> *>(_filterInputFrame.get());
    
    if(!t)
    {
      t = new ToFRawFrameTemplate<uint16_t, uint8_t>();
      _filterInputFrame = FramePtr(t);
    }
  }
  
  t->size = _size;
  t->id = input->id;
  t->timestamp = input->timestamp;
  
  t->_ambient.resize(_size.width*_size.height);
  t->_amplitude.resize(_size.width*_size.height);
  t->_phase.resize(_size.width*_size.height);
  t->_flags.resize(_size.width*_size.height);
  
  auto index = 0;
  
  Complex c;
  float phase;
  
  for (auto l = 0; l < _size.height; l++) 
  {
    for (auto j = 0; j < _size.width; j++) 
    {
      index = l*_size.width + j;
      
      c.real(input->_i[index]);
      c.imag(input->_q[index]);
      
      phase = std::arg(c);
      
      t->_phase[index] = ((phase < 0)?(2*M_PI + phase):phase)*TOF_ANGLE_TO_PHASE_FACTOR;
      t->_amplitude[index] = std::abs(c);
      
      t->_ambient[index] = 0;
      t->_flags[index] = 0;
    }
  }
  
  if(!_applyCrossTalkFilter(out))
    return false;
  
  
  t = dynamic_cast<ToFRawFrameTemplate<uint16_t, uint8_t> *>(out.get());
  
  if(!t)
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Output frame is not a ToF frame!" << std::endl;
    return false;
  }
  
  if(!_applyPhaseOffsetCorrection(t->_phase))
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Failed to apply phase offset correction" << std::endl;
    return false;
  }
  
  t->_histogram.clear(); // No histogram
  
  return true;
}


bool ToFFrameGenerator::_generateToFRawIQFrame(const FramePtr &in, FramePtr &out)
{
  if(_dataArrangeMode != 0)
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Data arrange mode is expected to be zero, but got " << _dataArrangeMode << std::endl;
    return false;
  }
  
  RawDataFramePtr rawDataFrame = std::dynamic_pointer_cast<RawDataFrame>(in);
  
  if(!rawDataFrame)
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Input data frame is not of raw data type." << std::endl;
    return false;
  }
  
  ToFRawIQFrameTemplate<int16_t> *t;
  
  t = dynamic_cast<ToFRawIQFrameTemplate<int16_t> *>(out.get());
    
  if(!t)
  {
    t = new ToFRawIQFrameTemplate<int16_t>();
    out = FramePtr(t);
  }
  
  t->size = _size;
  t->id = rawDataFrame->id;
  t->timestamp = rawDataFrame->timestamp;
  
  if(rawDataFrame->data.size() < _size.height*_size.width*2)
  {
    logger(LOG_ERROR) << "ToFFrameGenerator: Incomplete raw data size = " << rawDataFrame->data.size() << ". Required size = " << _size.height*_size.width*2 << std::endl;
    return false;
  }
  
  t->_i.resize(_size.width*_size.height);
  t->_q.resize(_size.width*_size.height);
  
  auto index1 = 0, index2 = 0;
      
  uint16_t *data = (uint16_t *)rawDataFrame->data.data();
  
  for (auto i = 0; i < _size.height; i++) 
  {
    for (auto j = 0; j < _size.width; j++) 
    {
      index1 = i*_size.width*2 + j*2;
      index2 = i*_size.width + j;
      
      t->_i[index2] = (int16_t)data[index1];
      t->_q[index2] = (int16_t)data[index1 + 1];
    }
  }
  
  return true;
}

bool ToFFrameGenerator::_applyCrossTalkFilter(FramePtr &out)
{
  if(!_crossTalkFilter)
    return true;
  
  return _crossTalkFilter->filter(_filterInputFrame, out);
}


}
}
