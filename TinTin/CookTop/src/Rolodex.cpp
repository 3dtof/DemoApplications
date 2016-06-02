#include "Rolodex.h"

#define CYLINDER_RADIUS    50
#define ANGULAR_SPACING    16
#define BOTTOM_CARD_ANGLE  120
#define FLIP_DELTA_ANGLE   5
#define ENDING_ANGLE       180

Rolodex::Rolodex(int w, int h) : AirApp("Rolodex")
{
   _width = w;
   _height = h;
   _orig = ofPoint(0,0);
   _currCardIndex=0;
   _angleTop = 0;
   _angleBottom = BOTTOM_CARD_ANGLE;
   _bAnimate = false;
}


Rolodex::~Rolodex()
{
}


bool Rolodex::isOver(ofPoint p)
{
   return (p.x > _orig.x && p.x < _orig.x + _width)
          && (p.y > _orig.y && p.y < _orig.y + _height);
}

void Rolodex::update()
{
   ofPoint m = ofPoint(ofGetAppPtr()->mouseX-ofGetWidth()/2,ofGetHeight()/2-ofGetAppPtr()->mouseY);
   if ( (m.x > _orig.x && m.x < _orig.x + _width)
               && (m.y > _orig.y && m.y < _orig.y + _height) )
      select();
   else
      deselect();
}


void Rolodex::draw(ofPoint orig)
{
   _orig = orig;
   int k = getCurrCardIndex();

   if (_cards.size() > 0)  // have at least one card
   {
      // Top card
      ofSetColor(256,256,256);
      ofPushMatrix();
      ofTranslate(orig.x+_width/2, orig.y+_height/2);
      _cards[k].setAnchorPoint(_cards[k].getWidth()/2, 0);
      ofRotateX(_angleTop);
      _cards[k].draw(0,0);
      ofPopMatrix();

      if (_cards.size() > 1) // have at least two cards to animate
      {
         // Card 2
         k = getNextCardIndex(k);
         ofPushMatrix();
         ofTranslate(orig.x+_width/2, orig.y+_height/2);
         _cards[k].setAnchorPoint(_cards[k].getWidth()/2, 0);
         ofRotateX(_angleBottom);
         _cards[k].draw(0,0);   
         ofPopMatrix();

         if (_cards.size() > 2) // have at least 3 cards to draw the rest
         {
            float theta = (180-BOTTOM_CARD_ANGLE)/(_cards.size()-2);
            float a = BOTTOM_CARD_ANGLE + theta;
            for (int i=0; i<_cards.size()-2; i++)
            {
               k = getNextCardIndex(k);
               ofPushMatrix();
               ofTranslate(orig.x+_width/2, orig.y+_height/2);
               _cards[k].setAnchorPoint(_cards[k].getWidth()/2, 0);
               ofRotateX(a);
               _cards[k].draw(0,0);   
               ofPopMatrix();    
               if (a >= ENDING_ANGLE)
                  break;
               else
                  a += theta;   
            }
         } // 3+ cards
      } // 2+ cards
   } // 1+ cards

   ofSetColor(255,255,255);
}


int Rolodex::size()
{
   return _cards.size();
}



void Rolodex::setCurrCardIndex(int k)
{
   _currCardIndex = k;
}


int Rolodex::getCurrCardIndex()
{
   if (_cards.size())
      return _currCardIndex;
   else
      return -1;
}


int Rolodex::getNextCardIndex(int k)
{
   return ( (k+1) > _cards.size()-1 ) ? 0 : k+1;
}


int Rolodex::getPrevCardIndex(int k)
{
   return (k-1 < 0) ? _cards.size()-1 : k-1;
}



void Rolodex::addCard(ofImage img)
{
   img.resize(_width, _height);
   _cards.push_back(img);
}


void Rolodex::addCard(std::string filename)
{
   ofImage img;
   img.load(filename);
   img.resize(_width, _height/2);
   _cards.push_back(img);
}


ofImage *Rolodex::getCard(int i)
{
   if (_cards.size() > i)
      return &_cards[i];
   else
      return NULL;
}


//--------------------------------------------------------------
void Rolodex::keyPressed(int key)
{
  
}

//--------------------------------------------------------------
void Rolodex::keyReleased(int key)
{

}

//--------------------------------------------------------------
void Rolodex::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void Rolodex::mouseDragged(int x, int y, int button)
{
//   if (_bAnimate)
   {
      int delta = y - _dragStart;

      if (_dragStart > _orig.y + _height/2)   // top card
      {
         _angleTop = -delta;
         if (_angleTop > BOTTOM_CARD_ANGLE) 
            _angleTop = BOTTOM_CARD_ANGLE-1;
         else if (_angleTop < 0)
            _angleTop = 0;
      }
      else                                   // bottom card
      {
         _angleBottom = BOTTOM_CARD_ANGLE - delta;
         if (_angleBottom <= 0) 
            _angleBottom = 1;
         else if (_angleBottom > BOTTOM_CARD_ANGLE)
            _angleBottom = BOTTOM_CARD_ANGLE;
      }
   }
}

//--------------------------------------------------------------
void Rolodex::mousePressed(int x, int y, int button)
{
   _dragStart = y;
}

//--------------------------------------------------------------
void Rolodex::mouseReleased(int x, int y, int button)
{
   if (_dragStart > _orig.y + _height/2)   // top card
   {
      if (_angleTop > BOTTOM_CARD_ANGLE/2) 
         _currCardIndex = getPrevCardIndex(_currCardIndex);
   }
   else                          // bottom card
   {
      if (_angleBottom < BOTTOM_CARD_ANGLE/2) 
         _currCardIndex = getNextCardIndex(_currCardIndex);
   }
   _angleTop = 0;
   _angleBottom = BOTTOM_CARD_ANGLE;
}


