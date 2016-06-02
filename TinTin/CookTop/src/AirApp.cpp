#include "AirApp.h"


AirApp::AirApp()
{
   _bSelected = false;
}


AirApp::AirApp(std::string name)
{
   _name = name;
   _bSelected = false;
}


AirApp::~AirApp()
{
}

void AirApp::select()
{
   _bSelected = true;
}


void AirApp::deselect()
{
   _bSelected = false;
}


bool AirApp::isSelected()
{
   return _bSelected;
}


bool AirApp::isOver(ofPoint p)
{
}


void AirApp::update()
{
}


void AirApp::draw()
{
}


std::string AirApp::getName()
{
   return _name;
}


void AirApp::setName(std::string name)
{
   _name = name;
}


bool AirApp::loadIcon(std::string name)
{
   return _icon.load(name);
}


ofImage &AirApp::getIcon()
{
   return _icon;
}

void AirApp::keyPressed(int key)
{
}

void AirApp::keyReleased(int key)
{
}

void AirApp::mouseMoved(int x, int y)
{
}


void AirApp::mouseDragged(int x, int y, int button)
{
}


void AirApp::mousePressed(int x, int y, int button)
{
}


void AirApp::mouseReleased(int x, int y, int button)
{

}

