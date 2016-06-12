#include "CookTop.h"
#include "AirPanel.h"

#define BORDER_WIDTH    10

#define BUTTON_WIDTH    150
#define BUTTON_HEIGHT   150

AirPanel::AirPanel()
{
}


AirPanel::AirPanel(std::string name, ofPoint orig, int w, int h)
{
   init(name, orig, w, h);
}


void AirPanel::init(std::string name, ofPoint orig, int w, int h)
{
   _orig = orig;
   _width =  w;
   _height = h;
   _name = name;
   _focus = 0;
   _bSelected = false;

   // Create and resize buttons
   _homeButton = new AirButton("Home", "images/HomeButton.png");
   _backButton = new AirButton("Back", "images/BackButton.png");
   _selectButton = new AirButton("Select", "images/SelectButton.png");
   _buttons.push_back(_homeButton);
   _buttons.push_back(_backButton);
   _buttons.push_back(_selectButton);
   for (int i=0; i<_buttons.size(); i++)
      _buttons[i]->resize(_width/_buttons.size(), BUTTON_HEIGHT);

   // Setup and add HomeScreen app (required, and always at _apps[0])
   HomeScreen *hs = new HomeScreen(_width-2*BORDER_WIDTH, _height-4*BORDER_WIDTH-BUTTON_HEIGHT);
   hs->loadIcon("images/home_icon.png");
   addApp((AirApp *)hs);  

   // Setup and add Rolodex app
   Rolodex *r = new Rolodex(_width-2*BORDER_WIDTH, _height-4*BORDER_WIDTH-BUTTON_HEIGHT);
   r->loadIcon("images/recipe_icon.png");
   r->addCard("images/recipe1.png");
   r->addCard("images/recipe2.png");
   r->addCard("images/recipe3.png");
   r->addCard("images/recipe4.png");
   r->addCard("images/recipe5.png");
   r->addCard("images/recipe6.png");
   r->addCard("images/recipe7.png");
   r->addCard("images/recipe8.png");
   r->addCard("images/recipe9.png");
   r->addCard("images/recipe10.png");
   r->addCard("images/recipe11.png");

   addApp((AirApp *)r);

   // Populate HomeScreen with apps
   hs->loadApps(_apps);
}


AirPanel::~AirPanel()
{
}


void AirPanel::update()
{
   // App updates
   for (int i=0; i<_apps.size(); i++)
   {
      AirApp *app = _apps[i];
      if (app->getName() == "HomeScreen")
      {
         HomeScreen *hs = (HomeScreen *)app;
         hs->update();
      }
      else if (app->getName() == "Rolodex")
      {
         Rolodex *r = (Rolodex *)app;
         r->update();
      }
   }

   // Button updates
   for (int i=0; i<_buttons.size(); i++)
      _buttons[i]->update();
}


void AirPanel::draw()
{
   ofPoint m = ofPoint(ofGetAppPtr()->mouseX-ofGetWidth()/2,ofGetHeight()/2-ofGetAppPtr()->mouseY);

   // Draw panel border
   ofSetColor(255, 255, 255);
   ofNoFill();
   ofDrawRectRounded(ofPoint(_orig.x, _orig.y+2*BORDER_WIDTH+BUTTON_HEIGHT), 
            _width, _height-2*BORDER_WIDTH-BUTTON_HEIGHT, BORDER_WIDTH);

   // Draw app in focus
   AirApp *app = _apps[_focus];
   if (app->getName() == "HomeScreen")
   {
      HomeScreen *hs = (HomeScreen *)app;
      hs->draw(ofPoint(_orig.x+BORDER_WIDTH, _orig.y+2*BORDER_WIDTH+BUTTON_HEIGHT+BORDER_WIDTH));
   }
   else if (app->getName() == "Rolodex")
   {
      Rolodex *r = (Rolodex *)app;
      r->draw(ofPoint(_orig.x+BORDER_WIDTH, _orig.y+2*BORDER_WIDTH+BUTTON_HEIGHT+BORDER_WIDTH));
   }  

   // Draw buttons
   getButton("Back")->draw(ofPoint(_orig.x, _orig.y+BORDER_WIDTH));
   getButton("Home")->draw(ofPoint(_orig.x+BUTTON_WIDTH, _orig.y+BORDER_WIDTH));
   getButton("Select")->draw(ofPoint(_orig.x+2*BUTTON_WIDTH, _orig.y+BORDER_WIDTH));
}


void AirPanel::deselect()
{
   _bSelected = false;
}


void AirPanel::select()
{
   _bSelected = true;
}


bool AirPanel::isSelected()
{
   return _bSelected;
}


bool AirPanel::isFocused(ofPoint p)
{
   return (p.x > _orig.x && p.x < _orig.x + _width)
          && (p.y > _orig.y && p.y < _orig.y + _height);
}


void AirPanel::setDimension(int w, int h)
{
   _width = w;
   _height = h;
}



bool AirPanel::addApp(AirApp *app)
{
   bool rc = false;

   if (app != NULL)
   {
      _apps.push_back(app);
      rc = true;
   }
   return rc;
}


int AirPanel::numApps()
{
   return _apps.size();
}


AirApp *AirPanel::getApp(int i)
{
   if (_apps.size() > i)
      return _apps[i];
   else
      return NULL;
}



int AirPanel::numButtons()
{
   return _buttons.size();
}


bool AirPanel::addButton(AirButton *button)
{
   bool rc = false;

   if (button)
   {
      _buttons.push_back(button);
      rc = true;
   }
   return rc;
}


AirButton *AirPanel::getButton(int i)
{
   if (_buttons.size() > i)
      return _buttons[i];
   else
      return NULL;
}


AirButton *AirPanel::getButton(std::string name)
{
   for (int i=0; i<_buttons.size(); i++)
   {
      AirButton *button = _buttons[i];
      if (button->getName() == name)
         return button;
   }
   return NULL;
}


std::string AirPanel::getName()
{
   return _name;
}


void AirPanel::setName(std::string name)
{
   _name = name;
}


int AirPanel::getWidth()
{
   return _width;
}


int AirPanel::getHeight()
{
   return _height;
}


void AirPanel::keyPressed(int key)
{
}


void AirPanel::keyReleased(int key)
{
}



void AirPanel::mouseMoved(int x, int y)
{
   AirApp *app = _apps[_focus];
   if (app->getName() == "HomeScreen")
   {
      HomeScreen *hs = (HomeScreen *)app;
      if (hs->isSelected())
         hs->mouseMoved(x, y);
   }
   else if (app->getName() == "Rolodex")
   {
      Rolodex *r = (Rolodex *)app;
      if (r->isSelected())
         r->mouseMoved(x, y);
   }
}


void AirPanel::mouseDragged(int x, int y, int button)
{
   AirApp *app = _apps[_focus];
   if (app->getName() == "HomeScreen")
   {
      HomeScreen *hs = (HomeScreen *)app;
      if (hs->isSelected())
         hs->mouseDragged(x, y, button);
   }
   else if (app->getName() == "Rolodex")
   {
      Rolodex *r = (Rolodex *)app;
      if (r->isSelected())
         r->mouseDragged(x, y, button);
   }

}


void AirPanel::mousePressed(int x, int y, int button)
{
   AirApp *app = _apps[_focus];
   if (app->getName() == "HomeScreen")
   {
      HomeScreen *hs = (HomeScreen *)app;
      if (hs->isFocused(ofPoint(x,y)))
         hs->mousePressed(x, y, button);
   }
   else if (app->getName() == "Rolodex")
   {
      Rolodex *r = (Rolodex *)app;
      if (r->isFocused(ofPoint(x,y)))
         r->mousePressed(x, y, button);
   }

   if (_homeButton && _homeButton->isFocused(ofPoint(x,y)))
   {
      _homeButton->mousePressed(x, y, button);
   }
   else if (_backButton && _backButton->isFocused(ofPoint(x,y)))
   {
      _backButton->mousePressed(x, y, button);
   }
   else if (_selectButton && _selectButton->isFocused(ofPoint(x,y)))
   {
      _selectButton->mousePressed(x, y, button);
   }
}


void AirPanel::mouseReleased(int x, int y, int button)
{
   // Process Apps
   AirApp *app = _apps[_focus];
   if (app->getName() == "HomeScreen")
   {
      HomeScreen *hs = (HomeScreen *)app;
      if (hs->isSelected())
      {
         hs->mouseReleased(x, y, button);
         _focus = hs->getAppSelected();
      }
   }
   else if (app->getName() == "Rolodex")
   {
      Rolodex *r = (Rolodex *)app;
      if (r->isSelected())
      {
         r->mouseReleased(x, y, button);
      }
   }

   // Process Buttons
   if (_homeButton && _homeButton->isSelected())
   {
      _homeButton->mouseReleased(x, y, button);
      _focus = 0;
   }
   else if (_backButton && _backButton->isSelected())
   {
      _backButton->mouseReleased(x, y, button);
   }
   else if (_selectButton && _selectButton->isSelected())
   {
      _selectButton->mouseReleased(x, y, button);
   }
}


