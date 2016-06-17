#include "ofMain.h"
#include <string>

#include "AirApp.h"
#include "AirButton.h"
#include "Rolodex.h"
#include "HomeScreen.h"
#include "VideoPlayer.h"


#ifndef __AIRPANEL_H__
#define __AIRPANEL_H__

class AirPanel
{
public:
   AirPanel();
   AirPanel(std::string name, ofPoint orig, int w, int h);
   ~AirPanel();
   void init(std::string name, ofPoint orig, int w, int h);
   void update();
   void draw();
   void select();
   void deselect();
   bool isSelected();
   bool isFocused(ofPoint p);

   std::string getName();
   void setName(std::string name);

   int getWidth();
   int getHeight();
   void setDimension(int w, int h);

   int numApps();
   bool addApp(AirApp *app);
   AirApp *getApp(int i);

   int numButtons();
   bool addButton(AirButton *button);
   AirButton *getButton(int i);
   AirButton *getButton(std::string name);


	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
   bool _bSelected;
   int _focus;

   std::string _name;
   ofPoint _orig;
   int _width, _height;

   AirButton *_homeButton;
   AirButton *_backButton;
   AirButton *_selectButton;

   vector<AirApp *> _apps;
   vector<AirButton *> _buttons;
};

#endif

