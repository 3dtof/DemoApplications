#pragma once

#include "ofMain.h"
#include "Stove.h"
#include "AirPanel.h"

#ifndef __OFAPP_H__
#define __OFAPP_H__

class ofApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();
		
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
    
private:
   ofEasyCam _cam;
   ofLight _light;
   AirPanel _panel;
   Stove _stove;
   bool _bCamMouseEnabled;
};

#endif
