#include "CookTop.h"
#include "AirButton.h"

#define IMG_GAP      4

AirButton::AirButton()
{
}


AirButton::AirButton(std::string name)
{
   _name = name;
   _bSelected = false;
}


AirButton::AirButton(std::string name, ofImage img)
{
   _name = name;
   _img = img;
   _width = _img.getWidth();
   _height = _img.getHeight();
   _bSelected = false;
}


AirButton::AirButton(std::string name, std::string path)
{
   _name = name;
   loadImage(path);
   _bSelected = false;
}

   
AirButton::~AirButton()
{
}


bool AirButton::loadImage(std::string path)
{
   bool rc = _img.load(path);
   _width = _img.getWidth();
   _height = _img.getHeight();

   return rc;
}


void AirButton::update()
{
}


void AirButton::draw(ofPoint orig)
{   
   ofPoint m = ofPoint(ofGetAppPtr()->mouseX-ofGetWidth()/2,ofGetHeight()/2-ofGetAppPtr()->mouseY);

   _orig = orig;

   ofSetColor(255,255,255);
   _img.draw(ofPoint(_orig.x+IMG_GAP, _orig.y+IMG_GAP));

   if (isFocused(m))
   {
      if (isSelected())
         ofSetColor(0,255,0);
      else
         ofSetColor(0,0,255);
      ofNoFill();
      ofDrawRectangle(_orig, _width, _height); 
   }
   else if (isSelected())
   {
      ofSetColor(0,255,0);
      ofNoFill();
      ofDrawRectangle(_orig, _width, _height); 
   }
}


void AirButton::select()
{
   _bSelected = true;
}


void AirButton::deselect()
{
   _bSelected = false;
}


bool AirButton::isSelected()
{
   return _bSelected;
}


std::string AirButton::getName()
{
   return _name;
}


void AirButton::setName(std::string name)
{
   _name = name;
}

   
int AirButton::getWidth()
{
   return _width;
}


int AirButton::getHeight()
{
   return _height;
}


void AirButton::resize(int w, int h)
{
   _img.resize(w-2*IMG_GAP, h-2*IMG_GAP);
   _width = w;
   _height = h;
}


bool AirButton::isFocused(ofPoint m)
{
   return (m.x > _orig.x && m.x < _orig.x + _width) 
       && (m.y > _orig.y && m.y < _orig.y + _height);
}


void AirButton::mousePressed(int x, int y, int button)
{
   if (button == MOUSE_BUTTON_LEFT)
      _bSelected = true;
}


void AirButton::mouseReleased(int x, int y, int button)
{
   _bSelected = false;
}




