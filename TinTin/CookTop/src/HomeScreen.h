#include "AirApp.h"
#include "AirButton.h"

#ifndef __HOMESCREEN_H__
#define __HOMESCREEN_H__

class HomeScreen : public AirApp
{
public:
   HomeScreen(int w, int h);
   ~HomeScreen();
   void update();
   void draw(ofPoint p);
   bool isOver(ofPoint p);

   std::string getName();
   void setName(std::string name);
   void loadApps(vector<AirApp *> &app);
   int getAppSelected();

   void keyPressed(int key);
   void keyReleased(int key);
   void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
   int _width, _height;
   ofPoint _orig;
   int _iconSize;
   int _appSelected;
   vector<AirButton *> _appButtons;
};

#endif

