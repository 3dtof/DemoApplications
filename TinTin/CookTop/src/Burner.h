/*!
 * 
 */
#include "ofMain.h"
#include <vector>

#ifndef __BURNER_H__
#define __BURNER_H__


class Burner
{
public:
   Burner(ofPoint center, int rings);
   ~Burner();
   void draw();
   void update();
   void select();
   void deselect();
   bool isSelected();

   bool isOn();
   void on();
   void off();
   float getRadius();
   bool isFocused(ofPoint p);


	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
   float _onRate;
   float _offRate;
   float _heat;
   float _radius;
   int _stroke;
   int _gap;
   int _rings;
   bool _on;
   ofPoint _orig;
   ofColor _red;

   bool _bSelected;
   int _selectCounter;
};

#endif

