/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */

#ifndef VOXEL_TI_HADDOCKCDKCAMERA_H
#define VOXEL_TI_HADDOCKCDKCAMERA_H

#include <ToFHaddockCamera.h>
#include <Downloader.h>

#include "TI3DToFExports.h"

#define HADDOCK_CDK_VENDOR_ID 0x0451U
#define HADDOCK_CDK_PRODUCT_ID1 0x9100U

#undef ILLUM_VOLTAGE
#define ILLUM_VOLTAGE "illum_power" // Illumination voltage
#define ILLUM_VOLTAGE2 "illum_power2" // Illumination voltage
#define MIX_VOLTAGE "mix_volt" // Mixing voltage
#define BLK_BLANK_SIZE "blk_blank_size"

namespace Voxel
{
  
namespace TI
{

class TI3DTOF_EXPORT HaddockCDKCamera: public ToFHaddockCamera
{
protected:
  Ptr<Downloader> _downloader;
  
  bool _init();
  
  virtual bool _getFieldOfView(float &fovHalfAngle) const;
  virtual bool _getSupportedVideoModes(Vector<SupportedVideoMode> &supportedVideoModes) const;
  virtual bool _setStreamerFrameSize(const FrameSize &s);
  
  virtual bool _getMaximumVideoMode(VideoMode &videoMode) const;
  virtual bool _getMaximumFrameRate ( FrameRate &frameRate, const FrameSize &forFrameSize ) const;
  
  virtual bool _initStartParams();
public:
  HaddockCDKCamera(DevicePtr device);
  
  virtual ~HaddockCDKCamera() {}
};

}
}

#endif // VOXEL_TI_HADDOCKCDKCAMERA_H
