#include "HomeScreen.h"

#define APPS_PER_ROW       3

HomeScreen::HomeScreen(int w, int h) : AirApp("HomeScreen")
{
   _width = w;
   _height = h;
   _orig = ofPoint(0,0);
   _iconSize = _width/APPS_PER_ROW;
   _appSelected = 0;
}


HomeScreen::~HomeScreen()
{
}

bool HomeScreen::isOver(ofPoint p)
{
   return (p.x > _orig.x && p.x < _orig.x + _width)
          && (p.y > _orig.y && p.y < _orig.y + _height);
}


void HomeScreen::loadApps(vector<AirApp *> &app)
{
   _appButtons.clear();
   for (int i=0; i<app.size(); i++)
   {  
      ofImage icon = app[i]->getIcon();
      AirButton *b = new AirButton(app[i]->getName(), icon);
      b->resize(_iconSize, _iconSize);
      _appButtons.push_back(b);
   }
}


int HomeScreen::getAppSelected()
{
   return _appSelected;
}


void HomeScreen::update()
{
   ofPoint m = ofPoint(ofGetAppPtr()->mouseX-ofGetWidth()/2,ofGetHeight()/2-ofGetAppPtr()->mouseY);
   if (isOver(m))
      select();
   else
      deselect();
   
   // Update appButtons
   for (int i=0; i<_appButtons.size(); i++)
   {
      if (_appButtons[i])
         _appButtons[i]->update();
   }
}


void HomeScreen::draw(ofPoint orig)
{
   _orig = orig;
   for (int i=1; i<_appButtons.size(); i++)  // skip i=0
   {
      AirButton *b = _appButtons[i];
      if (b)
      {
         b->draw(ofPoint(_orig.x + ((i-1)%APPS_PER_ROW)*_iconSize, 
                         _orig.y + ((i-1)%APPS_PER_ROW)*_iconSize));
      }
   }
}


//--------------------------------------------------------------
void HomeScreen::keyPressed(int key)
{

}

//--------------------------------------------------------------
void HomeScreen::keyReleased(int key)
{

}

//--------------------------------------------------------------
void HomeScreen::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void HomeScreen::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void HomeScreen::mousePressed(int x, int y, int button)
{
   for (int i=0; i<_appButtons.size(); i++)
   {
      AirButton *b = _appButtons[i];
      if (b && b->isOver(ofPoint(x,y)))
      {
         b->select();
         break;
      }
   }
}

//--------------------------------------------------------------
void HomeScreen::mouseReleased(int x, int y, int button)
{
   for (int i=0; i<_appButtons.size(); i++)
   {
      AirButton *b = _appButtons[i];
      if (b && b->isSelected())
      {
         _appSelected = i;
         break;
      }
   }
}


