/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */

#include "ToFHaddockCamera.h"
#include <Configuration.h>
#include <ParameterDMLParser.h>

namespace Voxel
{
  
namespace TI
{
  
/// Custom parameters
class HaddockVCOFrequency: public FloatParameter
{
  ToFHaddockCamera &_depthCamera;
public:
  HaddockVCOFrequency(ToFHaddockCamera &depthCamera, RegisterProgrammer &programmer):
  FloatParameter(programmer, VCO_FREQ, "MHz", 0, 0, 0, 1, 600, 1300, 864, "VCO frequency", 
                 "Frequency of the VCO used for generating modulation frequencies", IOType::IO_READ_WRITE, {MOD_M, MOD_N}), _depthCamera(depthCamera) {}
                 
  virtual bool get(float &value, bool refresh = false)
  {
    uint modM, modN, systemClockFrequency;
    if(!_depthCamera._get(MOD_M, modM, refresh) || !_depthCamera._get(MOD_N, modN, refresh) || !_depthCamera._get(SYS_CLK_FREQ, systemClockFrequency, refresh))
      return false;
    
    if(modN == 0)
      return false;
    
    float v = systemClockFrequency*modM/modN;
    
    if(!validate(v))
      return false;
    
    value = v;
    return true;
  }
  
  virtual bool set(const float &value)
  {
    if(!validate(value))
      return false;
    
    if(!_depthCamera._set(MOD_PLL_UPDATE, true))
      return false;
    
    ParameterPtr pllUpdate(nullptr, [this](Parameter *) { _depthCamera._set(MOD_PLL_UPDATE, false); }); // Set PLL update to false when going out of scope of this function
    
    uint modM, modN, systemClockFrequency;
    if(!_depthCamera._getSystemClockFrequency(systemClockFrequency))
      return false;
    
    modN = 18;
    
    if(systemClockFrequency == 0)
      return false;
    
    modM = value*modN/systemClockFrequency;
    
    if(!_depthCamera._set(MOD_M, modM) || !_depthCamera._set(MOD_N, modN))
      return false;
    
    _value = modM*systemClockFrequency/modN;
    
    return true;
  }
  
  virtual ~HaddockVCOFrequency() {}
};

class HaddockModulationFrequencyParameter: public FloatParameter
{
  ToFHaddockCamera &_depthCamera;
  String _psName;
public:
  HaddockModulationFrequencyParameter(ToFHaddockCamera &depthCamera, RegisterProgrammer &programmer, const String &name, const String &psName):
  FloatParameter(programmer, name, "MHz", 0, 0, 0, 1, 6.25f, 433.333f, 18, "Modulation frequency", "Frequency used for modulation of illumination", 
                 Parameter::IO_READ_WRITE, {psName}), _psName(psName), _depthCamera(depthCamera) {}
                 
  virtual bool get(float &value, bool refresh = false)
  {
    float vcoFrequency;
    
    uint modulationPS;
    
    if(!_depthCamera._get(VCO_FREQ, vcoFrequency, refresh) || !_depthCamera._get(_psName, modulationPS, refresh))
      return false;
    
    float v = vcoFrequency/3/modulationPS;
    
    if(!validate(v))
      return false;
    
    value = v;
    return true;
  }
  
  virtual bool set(const float &value)
  {
    if(!validate(value))
      return false;
    
    ParameterPtr p = _depthCamera.getParam(VCO_FREQ);
    
    if(!p)
      return false;
    
    HaddockVCOFrequency &v = (HaddockVCOFrequency &)*p;
    
    uint modulationPS = v.upperLimit()/3/value;
    
    if(!v.set(modulationPS*3*value))
      return false;
    
    if(!_depthCamera._set(MOD_PLL_UPDATE, true))
      return false;
    
    ParameterPtr pllUpdate(nullptr, [this](Parameter *) { _depthCamera._set(MOD_PLL_UPDATE, false); }); // Set PLL update to false when going out of scope of this function
    
    if(!_depthCamera._set(_psName, modulationPS))
      return false;
    
    float val;
    
    if(!v.get(val))
      return false;
    
    return true;
  }
  
  virtual ~HaddockModulationFrequencyParameter() {}
};

class HaddockToFFrameTypeParameter: public UnsignedIntegerParameter
{
public:
  HaddockToFFrameTypeParameter(RegisterProgrammer &programmer):
  UnsignedIntegerParameter(programmer, ToF_FRAME_TYPE, "", 0x5c25, 24, 11, 8, 0, 15, 0, "", "Type of ToF frame", Parameter::IO_READ_WRITE) {}
  
  virtual ~HaddockToFFrameTypeParameter() {}
};

ToFHaddockCamera::ToFHaddockCamera(const String &name, DevicePtr device): ToFCamera(name, device)
{
}

bool ToFHaddockCamera::_getSystemClockFrequency(uint &frequency) const
{
  return _get(SYS_CLK_FREQ, frequency);
}


bool ToFHaddockCamera::_getMaximumFrameSize(FrameSize &s) const
{
  s.width = 320;
  s.height = 240;
  return true;
}

  
bool ToFHaddockCamera::_init()
{
  Configuration c;
  
  String name = configFile.get("core", "dml");
  
  if(!name.size() || !c.getConfFile(name)) // true => name is now a proper path
  {
    logger(LOG_ERROR) << "ToFHaddockCamera: Failed to locate/read DML file '" << name << "'" << std::endl;
    return false;
  }
  
  ParameterDMLParser p(*_programmer, name);
  
  Vector<ParameterPtr> params;
  
  if(!p.getParameters(params))
  {
    logger(LOG_ERROR) << "ToFHaddockCamera: Could not read parameters from DML file '" << name << "'" << std::endl;
    return _parameterInit = false;
  }
  
  for(auto &p: params)
  {
    if((p->address() >> 8) == 0) // bankId == 0
      p->setAddress((0x58 << 8) + p->address());
    else if((p->address() >> 8) == 3) // bankId == 3
      p->setAddress((0x5C << 8) + (p->address() & 0xFF));
  }
  
  if(!_addParameters(params))
    return false;
  
  
  if(!_addParameters({
    ParameterPtr(new HaddockVCOFrequency(*this, *_programmer)),
    ParameterPtr(new HaddockModulationFrequencyParameter(*this, *_programmer, MOD_FREQ1, MOD_PS1)),
    ParameterPtr(new HaddockModulationFrequencyParameter(*this, *_programmer, MOD_FREQ2, MOD_PS2)),
    ParameterPtr(new HaddockToFFrameTypeParameter(*_programmer))
  }))
    return false;
  
  if(!ToFCamera::_init())
  {
    return false;
  }
  
  return true;
}

bool ToFHaddockCamera::_reset()
{
  if(!ToFCamera::_reset() || !set(PHASE_CORR_1, configFile.getInteger("calib", "phase_corr_1")))
    return false;
  return true;
}

bool ToFHaddockCamera::_initStartParams()
{
  if(!set(TG_EN, true) || 
     !set(BLK_SIZE, 1024U) ||
     !set(BLK_HEADER_EN, true) ||
     !set(OP_CS_POL, true) ||
     !set(FB_READY_EN, true) ||
     !set(CONFIDENCE_THRESHOLD, 1U) ||
     //!set(DEBUG_EN, true) || // Uncomment this for sample data
     !set(ILLUM_EN_POL, false))
    // || set(INTG_TIME, 40.0f);
    return false;
  
  return ToFCamera::_initStartParams();
}

bool ToFHaddockCamera::_allowedROI(String &message)
{
  message  = "Column start and end must be multiples of 16, both between 0 to 319. ";
  message += "Row start and end must be between 0 to 239.";
  return true;
}

bool ToFHaddockCamera::_getROI(RegionOfInterest &roi)
{
  uint rowStart, rowEnd, colStart, colEnd;
  
  if(!_get(ROW_START, rowStart) || !_get(ROW_END, rowEnd) || !_get(COL_START, colStart) || !_get(COL_END, colEnd))
  {
    logger(LOG_ERROR) << "ToFHaddockCamera: Could not get necessary parameters for ROI." << std::endl;
    return false;
  }
  
  colStart = colStart*2;
  colEnd = (colEnd + 1)*2 - 1;
  
  roi.x = colStart;
  roi.y = rowStart;
  roi.width = colEnd - colStart + 1;
  roi.height = rowEnd - rowStart + 1;
  return true;
}

bool ToFHaddockCamera::_setROI(const RegionOfInterest &roi)
{
  if(isRunning())
  {
    logger(LOG_ERROR) << "ToFHaddockCamera: Cannot set frame size while the camera is streaming" << std::endl;
    return false;
  }
  
  uint rowStart, rowEnd, colStart, colEnd;
  
  colStart = (roi.x/16)*8;
  colEnd = ((roi.x + roi.width)/16)*8 - 1;
  
  rowStart = roi.y;
  rowEnd = rowStart + roi.height - 1;
  
  if(!_set(ROW_START, rowStart) || !_set(ROW_END, rowEnd) || !_set(COL_START, colStart) || !_set(COL_END, colEnd))
  {
    logger(LOG_ERROR) << "ToFHaddockCamera: Could not get necessary parameters for ROI." << std::endl;
    return false;
  }
  
  if(!_setBinning(1, 1, roi))
  {
    logger(LOG_ERROR) << "ToFHaddockCamera: Could not reset binning while setting ROI." << std::endl;
    return false;
  }
  
  FrameSize s;
  
  if(!_getFrameSize(s) || !_setFrameSize(s, false)) // Get and set frame size to closest possible
  {
    logger(LOG_ERROR) << "ToFHaddockCamera: Could not update frame size to closest valid frame size" << std::endl;
    return false;
  }
  
  return true;
}

bool ToFHaddockCamera::_isHistogramEnabled() const
{
  bool histogramEnabled;
  return _get(HISTOGRAM_EN, histogramEnabled) && histogramEnabled;
}

bool ToFHaddockCamera::_getToFFrameType(ToFFrameType &frameType) const
{
  uint r;
  
  if(!get(ToF_FRAME_TYPE, r))
  {
    frameType = ToF_PHASE_AMPLITUDE;
    return true;
  }
  
  if(r == 1)
    frameType = ToF_I_Q;
  else
    frameType = ToF_PHASE_AMPLITUDE;
  
  return true;
}



bool ToFHaddockCamera::_getDepthScalingFactor(float &factor)
{
  float modulationFrequency1, modulationFrequency2;
  bool dealiasingEnabled;
  
  uint modulationPS1, modulationPS2;
  
  if(!get(DEALIAS_EN, dealiasingEnabled))
    return false;
  
  bool frequencyCorrectionPresent = false;
  float frequencyCorrection = 1.0f, frequencyCorrectionAt;
  
  if(configFile.isPresent("calib", "freq_corr"))
  {
    frequencyCorrection = configFile.getFloat("calib", "freq_corr");
    frequencyCorrectionAt = configFile.getFloat("calib", "freq_corr_at");
    
    if(frequencyCorrectionAt < 1E-5)
      frequencyCorrectionPresent = false;
    else
      frequencyCorrectionPresent = true;
  }
  
  if(dealiasingEnabled)
  {
    if(!get(MOD_FREQ1, modulationFrequency1) || !get(MOD_FREQ2, modulationFrequency2)) // ensure that these are valid
      return false;
    
    if(!get(MOD_PS1, modulationPS1) || !get(MOD_PS2, modulationPS2))
      return false;
    
    float freq = modulationFrequency1*gcd(modulationPS1, modulationPS2)/modulationPS2;
    
    if(frequencyCorrectionPresent)
      freq *= (frequencyCorrection*freq/frequencyCorrectionAt);
    
    factor = SPEED_OF_LIGHT/1E6f/(2*(1 << 12)*freq);
    
    return true;
  }
  else
  {
    if(!get(MOD_FREQ1, modulationFrequency1))
      return false;
    
    if(frequencyCorrectionPresent)
      modulationFrequency1 *= (frequencyCorrection*modulationFrequency1/frequencyCorrectionAt);
    
    factor = SPEED_OF_LIGHT/1E6f/2/modulationFrequency1/(1 << 12);
    return true;
  }
}
 
}
}
