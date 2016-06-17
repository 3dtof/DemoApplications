#include "CookTop.h"
#include "AirApp.h"
#include "AirButton.h"

#ifndef __VIDEOPLAYER_H__
#define __VIDEOPLAYER_H__

#define VIDEO_ROWS      7
#define VIDEO_GAP       6
#define VIDEO_WIDTH     150

class VideoPlayer : public AirApp
{
public:
   VideoPlayer(int w, int h);
   VideoPlayer(int w, int h, std::string path);
   ~VideoPlayer();
   void update();
   void draw(ofPoint orig);
   bool isFocused(ofPoint p);
   int size();

   bool loadDirectory(std::string path);
   vector<ofFile> &getFiles();
   vector<ofVideoPlayer> &getVideos();
   bool addVideo(std::string path);
   AirButton *getButton(std::string name);

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
   ofPoint _orig;
   int _width, _height;
   ofVideoPlayer _vid[VIDEO_ROWS];
   ofDirectory _dir;
   bool _bDirValid;
   vector<ofFile> _files;
   vector<ofVideoPlayer> _videos;
   int _currIdx;
   int _videoIdx;
   std::string _path;
   ofTrueTypeFont _title;
   vector<AirButton *> _buttons;
   AirButton *_stopButton;
   AirButton *_playButton;
   AirButton *_pauseButton;
};

#endif

