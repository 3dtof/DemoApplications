#include "ofMain.h"
#include <string>

#ifndef __AIRAPP_H__
#define __AIRAPP_H__

class AirApp
{
public:
   AirApp();
   AirApp(std::string name);
   ~AirApp();
   virtual void update();
   virtual void draw();
   virtual bool isFocused(ofPoint p);
   void select();
   void deselect();
   bool isSelected();

   std::string getName();
   void setName(std::string name);
   bool loadIcon(std::string path);
   ofImage &getIcon();

	virtual void keyPressed(int key);
	virtual void keyReleased(int key);
	virtual void mouseMoved(int x, int y);
	virtual void mouseDragged(int x, int y, int button);
	virtual void mousePressed(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);

private:
   std::string _name;
   ofImage _icon;
   bool _bSelected;
};

#endif

