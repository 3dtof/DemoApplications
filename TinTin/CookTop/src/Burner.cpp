/*!
 */
#include "CookTop.h"
#include "Burner.h"

#define MOUSE_BUTTON_RIGHT    2
#define MOUSE_BUTTON_MIDDLE   1
#define MOUSE_BUTTON_LEFT     0

/*!
 * @brief   Constructor
 */
Burner::Burner(ofPoint orig, int rings)
{
   _heat = 0.0;
   _onRate = 5.0;
   _offRate = 5.0;
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
   if (_on)
   {
      if ((int)_heat+_onRate <= 255)
         _heat += _onRate;
      else 
	 _heat = 255;
   }
   else 
   {
      if ((int)_heat-_offRate >= 0)
         _heat -= _offRate;
      else
         _heat = 0;
   }
   _red.r = (int)_heat;

}


/*!
 * @brief   Draw stove
 */
void Burner::draw()
{
   ofPoint m = ofPoint(ofGetAppPtr()->mouseX-ofGetWidth()/2,ofGetHeight()/2-ofGetAppPtr()->mouseY);

   // Draw heating elements
   ofSetCircleResolution(180);
   ofSetLineWidth(_stroke);
   ofNoFill();
   ofSetColor(_red);
   for (int i=0; i<_rings; i++)
      ofDrawCircle(_orig, (i+1)*(_stroke/2+_gap));

   // Draw stove border
   ofSetLineWidth(2);
   if (isFocused(m))
   {
      if (isSelected())
         ofSetColor(ofColor(0,255,0));
      else
         ofSetColor(ofColor(0,0,255));
   }
   else
      ofSetColor(ofColor(255,255,255));
   ofDrawCircle(_orig, _radius);
}


/*!
 * @brief   Test if a point is in stove's radius
 */
bool Burner::isFocused(ofPoint p)
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
   _bSelected = (button == MOUSE_BUTTON_LEFT);
}


void Burner::mouseReleased(int x, int y, int button)
{
   if (button == MOUSE_BUTTON_LEFT)
   {
      if (isOn())
         off();
      else
         on();
   }
   deselect();
}







