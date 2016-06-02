#include "ofMain.h"
#include <string>

#ifndef __AIRBUTTON_H__
#define __AIRBUTTON_H__

class AirButton
{
public:
   AirButton();
   AirButton(std::string name);
   AirButton(std::string name, ofImage img);
   AirButton(std::string name, std::string path);
   ~AirButton();

   bool loadImage(std::string path);
   bool loadImage(ofImage img);

   void update();
   void draw(ofPoint orig);
   void draw();

   void select();
   void deselect();
   bool isSelected();

   std::string getName();
   void setName(std::string name);

   int getWidth();
   int getHeight();
   void resize(int w, int h);

   bool isOver(ofPoint p);

	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
   bool _bSelected;
   int _selectCounter;

   std::string _name;
   ofPoint _orig;
   int _width, _height;

   std::string _path;
   ofImage _img;
};

#endif

