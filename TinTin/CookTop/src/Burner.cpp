/*!
 */
#include "ofMain.h"
#include "Burner.h"

#define MOUSE_BUTTON_RIGHT    2

/*!
 * @brief   Constructor
 */
Burner::Burner(ofPoint orig, int rings)
{
   _heat = 0.0;
   _onRate = 0.3;
   _offRate = 0.5;
   _red = ofColor(0,0,0);
   _orig = orig;
   _stroke = 10;
   _gap = 20;
   _rings = rings;
   _on = false;
   _bSelected = false;
   _radius = (_rings+1)*(_stroke/2+_gap);
}


/*!
 * @brief   Destructor
 */
Burner::~Burner()
{
}


/*!
 * @brief   Update
 */
void Burner::update()
{
   ofPoint m = ofPoint(ofGetAppPtr()->mouseX-ofGetWidth()/2,ofGetHeight()/2-ofGetAppPtr()->mouseY);
   _bSelected = isOver(m);

   if (_on)
   {
      if ((int)_heat < 255)
         _heat += _onRate;
   }
   else 
   {
      if ((int)_heat > 0)
         _heat -= _offRate;
   }
   _red.r = (int)_heat;

}


/*!
 * @brief   Draw stove
 */
void Burner::draw()
{
   // Draw heating elements
   ofSetCircleResolution(180);
   ofSetLineWidth(_stroke);
   ofNoFill();
   ofSetColor(_red);
   for (int i=0; i<_rings; i++)
      ofDrawCircle(_orig, (i+1)*(_stroke/2+_gap));

   // Draw stove border
   ofSetLineWidth(2);
   if (_bSelected)
      ofSetColor(ofColor(0,0,255));
   else
      ofSetColor(ofColor(255,255,255));
   ofDrawCircle(_orig, _radius);
}


/*!
 * @brief   Test if a point is in stove's radius
 */
bool Burner::isOver(ofPoint p)
{
   return _orig.distance(p) < _radius;
}


/*!
 * @brief   Test if stove is on
 */
bool Burner::isOn()
{
   return _on;
}


/*!
 * @brief   Turn stove ON
 */
void Burner::on()
{
   _on = true;
}


/*!
 * @brief   Turn stove OFF
 */
void Burner::off()
{
   _on = false;
}


/*!
 * @brief   Get radius
 */
float Burner::getRadius()
{
   return _radius;
}


void Burner::select()
{
   _bSelected = true;
}


void Burner::deselect()
{
   _bSelected = false;
}


bool Burner::isSelected()
{
   return _bSelected;
}


void Burner::keyPressed(int key)
{
}



void Burner::keyReleased(int key)
{

}



void Burner::mouseMoved(int x, int y)
{
}



void Burner::mouseDragged(int x, int y, int button)
{
}


void Burner::mousePressed(int x, int y, int button)
{
   _bSelected = (button == MOUSE_BUTTON_RIGHT);
}


void Burner::mouseReleased(int x, int y, int button)
{
   if (isSelected() && button == MOUSE_BUTTON_RIGHT)
   {
      if (isOn())
         off();
      else
         on();
   }
}







