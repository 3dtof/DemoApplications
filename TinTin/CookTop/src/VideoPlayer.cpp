#include "VideoPlayer.h"

#define BUTTON_WIDTH    100
#define BUTTON_HEIGHT   100
#define BORDER_WIDTH    10

VideoPlayer::VideoPlayer(int w, int h) : AirApp("VideoPlayer")
{
   _width = w;
   _height = h;
   _bDirValid = false;
   _currIdx = 0;
   _videoIdx = 0;
   _title.load("arial.ttf", 14);
   _orig = ofPoint(0,0);
   
   _stopButton = new AirButton("Stop", "images/StopButton.png");
   _playButton = new AirButton("Play", "images/PlayButton.png");
   _pauseButton = new AirButton("Pause", "images/PauseButton.png");
   _buttons.push_back(_stopButton);
   _buttons.push_back(_playButton);
   _buttons.push_back(_pauseButton);
   for (int i=0; i<_buttons.size(); i++)
      _buttons[i]->resize(BUTTON_WIDTH, BUTTON_HEIGHT);
}


VideoPlayer::~VideoPlayer()
{
}


bool VideoPlayer::isFocused(ofPoint p)
{
   return (p.x > _orig.x && p.x < _orig.x + _width)
          && (p.y > _orig.y && p.y < _orig.y + _height);
}


bool VideoPlayer::loadDirectory(std::string path)
{
   bool rc = _dir.doesDirectoryExist(path);
   if (rc)
   {
      _path = path;
      _dir.listDir(_path);
      _files = _dir.getFiles();
      _videos.clear();
      for (int i=0; i<_dir.getFiles().size(); i++)
      {
         ofVideoPlayer v;
         v.load(_dir.getFiles()[i].getAbsolutePath());
         _videos.push_back(v);
         _videos[i].setVolume(0);
         _videos[i].setFrame(_videos[i].getTotalNumFrames()/20);
      }
   }
   return rc;
}


vector<ofFile> &VideoPlayer::getFiles()
{
   return _files;
}


vector<ofVideoPlayer> &VideoPlayer::getVideos()
{
   return _videos;
}


bool VideoPlayer::addVideo(std::string path)
{
   bool rc = false;
   ofFile *f = new ofFile(path);
   if (f->exists()) 
   {
      ofVideoPlayer v;
      if (v.load(path))
      {
         _videos.push_back(v);
         rc = true;
      }
   }
   return rc;
}


AirButton *VideoPlayer::getButton(std::string name)
{
   for (int i=0; i<_buttons.size(); i++)
   {
      AirButton *button = _buttons[i];
      if (button->getName() == name)
         return button;
   }
   return NULL;
}


void VideoPlayer::update()
{
   ofPoint m = ofPoint(ofGetAppPtr()->mouseX-ofGetWidth()/2,ofGetHeight()/2-ofGetAppPtr()->mouseY);
   if (isFocused(m) )
      select();
   else
      deselect();

   // Update the videos
   for (int i=0; i < _videos.size(); i++)
      _videos[i].update();
}


void VideoPlayer::draw(ofPoint orig)
{
   _orig = orig;
   ofSetColor(255,255,255);

   int gap = (_width - 3*BUTTON_WIDTH)/4;

   for (int i=0; i< VIDEO_ROWS; i++)
   {
      int idx = _currIdx + i;
      if (idx < _videos.size())
      {
         // Draw video
         _videos[idx].draw( _orig.x, _orig.y + gap + BUTTON_HEIGHT + i*_height/VIDEO_ROWS, 
                                   VIDEO_WIDTH-VIDEO_GAP/2, _height/VIDEO_ROWS );
         // Draw video file name
         _title.drawString(_files[idx].getFileName(), _orig.x + VIDEO_WIDTH + VIDEO_GAP/2, 
                              _orig.y + gap + BUTTON_HEIGHT + (i+0.5)*_height/VIDEO_ROWS);
      }
   }

   // Draw buttons
   getButton("Stop")->draw(ofPoint(_orig.x + gap, _orig.y));
   getButton("Play")->draw(ofPoint(_orig.x + 2*gap + BUTTON_WIDTH, _orig.y));
   getButton("Pause")->draw(ofPoint(_orig.x+ 3*gap + 2*BUTTON_WIDTH, _orig.y));
}


int VideoPlayer::size()
{
   return _videos.size();
}


//--------------------------------------------------------------
void VideoPlayer::keyPressed(int key)
{
  
}

//--------------------------------------------------------------
void VideoPlayer::keyReleased(int key)
{

}

//--------------------------------------------------------------
void VideoPlayer::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void VideoPlayer::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void VideoPlayer::mousePressed(int x, int y, int button)
{
   // Process buttons
   if (_playButton && _playButton->isFocused(ofPoint(x,y)))
   {
      _playButton->mousePressed(x, y, button);
   }
   else if (_stopButton && _stopButton->isFocused(ofPoint(x,y)))
   {
      _stopButton->mousePressed(x, y, button);
   }
   else if (_pauseButton && _pauseButton->isFocused(ofPoint(x,y)))
   {
      _pauseButton->mousePressed(x, y, button);
   }
}

//--------------------------------------------------------------
void VideoPlayer::mouseReleased(int x, int y, int button)
{
   // Process Buttons
   if (_playButton && _playButton->isSelected())
   {
      _playButton->mouseReleased(x, y, button);
      for (int i=0; i < _videos.size(); i++)
         _videos[i].play();
   }
   else if (_stopButton && _stopButton->isSelected())
   {
      _stopButton->mouseReleased(x, y, button);
      for (int i=0; i < _videos.size(); i++)
         _videos[i].stop();
   }
   else if (_pauseButton && _pauseButton->isSelected())
   {
      _pauseButton->mouseReleased(x, y, button);
      for (int i=0; i < _videos.size(); i++)
         _videos[i].setPaused(!_videos[i].isPaused());
   }
}


