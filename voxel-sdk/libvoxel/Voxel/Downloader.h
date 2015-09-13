/*
 * TI Voxel Lib component.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */

#ifndef VOXEL_DOWNLOADER_H
#define VOXEL_DOWNLOADER_H

#include "Device.h"
#include <fstream>
#include <USBIO.h>
#include "Logger.h"

namespace Voxel
{
  
/**
 * \defgroup IO I/O classes
 * @{
 */

class VOXEL_EXPORT Downloader
{
protected:
  DevicePtr _device;
  LoggerOutStream _outStream;
  
  virtual bool _locateFile(String &file);
  
public:
  Downloader(DevicePtr device): _device(device) {}
  
  virtual bool download(const String &file) = 0;
  
  inline void setLogCallback(LoggerOutStream::LoggerOutStreamFunctionType f) { _outStream.setOutputFunction(f); }
  
  virtual ~Downloader() {}
};

typedef Ptr<Downloader> DownloaderPtr;

class VOXEL_EXPORT USBDownloader: public Downloader
{
protected:
  USBIOPtr _usbIO;
  
  virtual bool _configureForDownload();
  virtual bool _download(InputFileStream &file, long unsigned int filesize);
  
public:
  USBDownloader(DevicePtr device);
  
  virtual bool download(const String &file);
  
  virtual ~USBDownloader() {}
};

/**
 * @}
 */

}

#endif // VOXEL_DOWNLOADER_H
