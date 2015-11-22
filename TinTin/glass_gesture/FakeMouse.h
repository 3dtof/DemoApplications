/*
 * FakeMouse - simulated X windows mouse actions
 *
 * Copyright (c) 2015-2016 Texas Instruments Inc.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifndef __FAKEMOUSE_H__
#define __FAKEMOUSE_H__

class FakeMouse {
public:
   FakeMouse();
   FakeMouse(Display *disp);
   ~FakeMouse();
   void setDisplay(Display *disp);
   void buttonDown(int button); 
   void buttonUp(int button);
   void click(int button);
   void doubleClick(int button);
   void moveTo(int x, int y);
   void moveBy(int dx, int dy);
   void getPos(int &x, int &y);
   void getDim(int &xmax, int &ymax);

private:
   bool buttonEvent(int btn, int e);
   Display *display;
   Window root;
   Screen *screen;
   int width, height;
};
#endif
