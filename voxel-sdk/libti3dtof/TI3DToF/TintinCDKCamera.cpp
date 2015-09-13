/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */

#include "TintinCDKCamera.h"
#include "VoxelXUProgrammer.h"
#include <Logger.h>
#include <UVCStreamer.h>

#include <Parameter.h>
#define _USE_MATH_DEFINES
#include <math.h>
namespace Voxel
{
  
namespace TI
{
  
TintinCDKCamera::TintinCDKCamera(Voxel::DevicePtr device): ToFTinTinCamera("TintinCDKCamera", device)
{
  _init();
}

class TintinCDKMixVoltageParameter: public UnsignedIntegerParameter
{
protected:
  virtual uint _fromRawValue(uint32_t value) const
  {
    logger(LOG_ERROR) << "from raw value = " << value << std::endl;
    if(value > 0x80U)
      return (value - 0x80U)*50 + 500;
    else
      return 500;
  }
  
  virtual uint32_t _toRawValue(uint value) const
  {
    logger(LOG_ERROR) << "to raw value = " << value << std::endl;
    if(value > 500)
      return (value - 500)/50 + 0x80U;
    else
      return 0x80U;
  }
  
public:
  TintinCDKMixVoltageParameter(RegisterProgrammer &programmer):
  UnsignedIntegerParameter(programmer, MIX_VOLTAGE, "mV", 0x2D05, 8, 7, 0, 1200, 2000, 1500, "Mixing voltage", 
                           "Mixing voltage?", Parameter::IO_READ_WRITE, {})
  {}
  
  virtual ~TintinCDKMixVoltageParameter() {}
};


class TintinCDKPVDDParameter: public UnsignedIntegerParameter
{
protected:
  virtual uint _fromRawValue(uint32_t value) const
  {
    if(value > 0x80U)
      return (value - 0x80U)*50 + 500;
    else
      return 500;
  }
  
  virtual uint32_t _toRawValue(uint value) const
  {
    if(value > 500)
      return (value - 500)/50 + 0x80U;
    else
      return 0x80U;
  }
  
public:
  TintinCDKPVDDParameter(RegisterProgrammer &programmer):
  UnsignedIntegerParameter(programmer, PVDD, "mV", 0x2D0E, 8, 7, 0, 2000, 3600, 3300, "Pixel VDD", 
                           "Reset voltage level of pixel.", Parameter::IO_READ_WRITE, {})
  {}
  
  virtual ~TintinCDKPVDDParameter() {}
};


bool TintinCDKCamera::_init()
{
  USBDevice &d = (USBDevice &)*_device;
  
  DevicePtr controlDevice = _device;
  
  _programmer = Ptr<RegisterProgrammer>(new VoxelXUProgrammer(
    { {0x2D, 1}, {0x52, 1}, {0x54, 1}, {0x4B, 2}, {0x4E, 2}, {0x58, 3}, {0x5C, 3} },
    controlDevice));
  _streamer = Ptr<Streamer>(new UVCStreamer(controlDevice));
  
  if(!_programmer->isInitialized() || !_streamer->isInitialized())
    return false;
  
  if(!_addParameters({
    ParameterPtr(new TintinCDKMixVoltageParameter(*_programmer)),
    ParameterPtr(new TintinCDKPVDDParameter(*_programmer)),
    }))
  {
    return false;
  }
  
  // Settings for mix voltage and PVDD
  if(!_programmer->writeRegister(0x2D06, 0xFF) || 
    !_programmer->writeRegister(0x2D04, 0x40) ||
    !_programmer->writeRegister(0x2D05, 0x94) ||
    !_programmer->writeRegister(0x2D0D, 0x40) || 
    !_programmer->writeRegister(0x2D0E, 0xB8) ||
    !_programmer->writeRegister(0x2D0F, 0xFF)
  )
    return false;
    
  if(!ToFTinTinCamera::_init())
    return false;
  
  return true;
}

bool TintinCDKCamera::_initStartParams()
{
  return 
  //set(MIX_VOLTAGE, 1500U) &&
  ToFTinTinCamera::_initStartParams();
}



bool TintinCDKCamera::_getFieldOfView(float &fovHalfAngle) const
{
  fovHalfAngle = (87/2.0f)*(M_PI/180.0f);
  return true;
}

bool TintinCDKCamera::_setStreamerFrameSize(const FrameSize &s)
{
  UVCStreamer *streamer = dynamic_cast<UVCStreamer *>(&*_streamer);
  
  if(!streamer)
  {
    logger(LOG_ERROR) << "TintinCDKCamera: Streamer is not of type UVC" << std::endl;
    return false;
  }
  
  VideoMode m;
  m.frameSize = s;
  
  int bytesPerPixel;
  
  if(!_get(PIXEL_DATA_SIZE, bytesPerPixel))
  {
    logger(LOG_ERROR) << "TintinCDKCamera: Could not get current bytes per pixel" << std::endl;
    return false;
  }
  
  if(bytesPerPixel == 4)
    m.frameSize.width *= 2;
  
  if(!_getFrameRate(m.frameRate))
  {
    logger(LOG_ERROR) << "TintinCDKCamera: Could not get current frame rate" << std::endl;
    return false;
  }
  
  if(!streamer->setVideoMode(m))
  {
    logger(LOG_ERROR) << "TintinCDKCamera: Could not set video mode for UVC" << std::endl;
    return false;
  }
  
  return true;
}

bool TintinCDKCamera::_getSupportedVideoModes(Vector<SupportedVideoMode> &supportedVideoModes) const
{
  supportedVideoModes = Vector<SupportedVideoMode> {
    SupportedVideoMode(320,240,25,1,4),
    SupportedVideoMode(160,240,50,1,4),
    SupportedVideoMode(160,120,100,1,4),
    SupportedVideoMode(80,120,200,1,4),
    SupportedVideoMode(80,60,400,1,4),
    SupportedVideoMode(320,240,50,1,2),
    SupportedVideoMode(320,120,100,1,2),
    SupportedVideoMode(160,120,200,1,2),
    SupportedVideoMode(160,60,400,1,2),
    SupportedVideoMode(80,60,400,1,2),
  };
  return true;
}

bool TintinCDKCamera::_getMaximumVideoMode(VideoMode &videoMode) const
{
  int bytesPerPixel;
  if(!_get(PIXEL_DATA_SIZE, bytesPerPixel))
  {
    logger(LOG_ERROR) << "TintinCDKCamera: Could not get current bytes per pixel" << std::endl;
    return false;
  }
  
  videoMode.frameSize.width = 320;
  videoMode.frameSize.height = 240;
  videoMode.frameRate.denominator = 1;
  videoMode.frameRate.numerator = (bytesPerPixel == 4)?25:50;
  return true;
}


  
}
}