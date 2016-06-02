/*!
 */
#include "Stove.h"

Stove::Stove()
{
}


Stove::Stove(ofPoint orig, int w, int h)
{
   init(orig, w, h);
}


void Stove::init(ofPoint orig, int w, int h)
{
   _orig = orig;
   _borderColor = ofColor(255,255,255);
   _width = w;
   _height = h;
   _bSelected = false;

   // Create the burners
   Burner b1 = Burner(_orig + ofPoint(200, 200), 3);
   Burner b2 = Burner(_orig + ofPoint(_width-200, 200), 4);   
   Burner b3 = Burner(_orig + ofPoint(200, _height-200), 4);   
   Burner b4 = Burner(_orig + ofPoint(_width-200, _height-200), 3);

   _burners.push_back(b1);
   _burners.push_back(b2);
   _burners.push_back(b3);
   _burners.push_back(b4);
}


/*!
 * @brief   Destructor
 */
Stove::~Stove()
{
}


/*!
 * @brief   Update
 */
void Stove::update()
{
   ofPoint m = ofPoint(ofGetAppPtr()->mouseX-ofGetWidth()/2,ofGetHeight()/2-ofGetAppPtr()->mouseY);
   _bSelected = isOver(m);

   // Update burners
   for (int i=0; i<_burners.size(); i++)
      _burners[i].update();
}


/*!
 * @brief   Draw stove
 */
void Stove::draw()
{
   // Draw stove border
   if (_bSelected)
      ofSetColor(0,0,255);
   else
      ofSetColor(255,255,255);
   ofNoFill();
   ofDrawRectRounded(_orig, _width, _height, 10);

   // Draw burners
   for (int i=0; i<_burners.size(); i++)
      _burners[i].draw();
}



bool Stove::isOver(ofPoint p)
{
   return (p.x > _orig.x && p.x < _orig.x + _width)
          && (p.y > _orig.y && p.y < _orig.y + _height);
}


/*!
 * @brief   Set Stove origin
 */
void Stove::setOrig(ofPoint orig)
{
   _orig = orig;
}


/*!
 * @brief   Get number of burners
 */
int Stove::size()
{
   return _burners.size();
}


/*!
 * @brief   Get burner pointer
 */
Burner *Stove::getBurner(int i)
{
   if (_burners.size() > i)
      return &(_burners[i]);
   else
      return NULL;
}


/*!
 * @brief   Turn on/off burner
 */
bool Stove::setBurner(int i, bool on)
{
   bool rc = false;

   if (_burners.size() < i)
   {
      if (on) 
         _burners[i].on();
      else
         _burners[i].off();
      rc = true;
   }

   return rc;
}


void Stove::select()
{
   _bSelected = true;
}


void Stove::deselect()
{
   _bSelected = false;
}


bool Stove::isSelected()
{
   return _bSelected;
}


void Stove::keyPressed(int key)
{
   for (int i=0; i<size(); i++)
   {
      Burner *b = getBurner(i);
      if (b) b->keyPressed(key);
   }
}



void Stove::keyReleased(int key)
{
   for (int i=0; i<size(); i++)
   {
      Burner *b = getBurner(i);
      if (b) b->keyReleased(key);
   }
}



void Stove::mouseMoved(int x, int y)
{
   for (int i=0; i<size(); i++)
   {
      Burner *b = getBurner(i);
      if (b) b->mouseMoved(x, y);
   }
}



void Stove::mouseDragged(int x, int y, int button)
{
   for (int i=0; i<size(); i++)
   {
      Burner *b = getBurner(i);
      if (b) b->mouseDragged(x, y, button);
   }
}


void Stove::mousePressed(int x, int y, int button)
{
   for (int i=0; i<size(); i++)
   {
      Burner *b = getBurner(i);
      if (b) b->mousePressed(x, y, button);
   }
}


void Stove::mouseReleased(int x, int y, int button)
{
   for (int i=0; i<size(); i++)
   {
      Burner *b = getBurner(i);
      if (b) b->mouseReleased(x, y, button);
   }
}



