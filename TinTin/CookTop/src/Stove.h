/*!
 * 
 */
#include "ofMain.h"
#include <vector>

#include "Burner.h"

#ifndef __STOVE_H__
#define __STOVE_H__

class Stove
{
public:
   Stove();
   Stove(ofPoint orig, int w, int h);
   ~Stove();
   void init(ofPoint orig, int w, int h);
   void draw();
   void update();
   void select();
   void deselect();
   bool isSelected();
   bool isFocused(ofPoint p);

   void setOrig(ofPoint orig);
   int size();
   Burner *getBurner(int i);
   bool setBurner(int i, bool on);


	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
   bool _bSelected;
   int _selectCounter;

   ofPoint _orig;
   int _width, _height;
   ofColor _borderColor;
   vector<Burner> _burners;
};

#endif

