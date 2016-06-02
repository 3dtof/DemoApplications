#include "AirApp.h"

#ifndef __ROLODEX_H__
#define __ROLODEX_H__

class Rolodex : public AirApp
{
public:
   Rolodex(int w, int h);
   ~Rolodex();
   void update();
   void draw(ofPoint orig);
   bool isOver(ofPoint p);

   void addCard(ofImage img);
   void addCard(std::string filename);
   int size();
   ofImage *getCard(int i);
   int getCurrCardIndex(); 
   void setCurrCardIndex(int k);
   int getNextCardIndex(int k);
   int getPrevCardIndex(int k);

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
   ofPoint _orig;
   int _width, _height;

   vector<ofImage> _cards;
   int _currCardIndex;
   int _dragStart;
   bool _bAnimate;
   float _angleTop, _angleBottom;
};

#endif

