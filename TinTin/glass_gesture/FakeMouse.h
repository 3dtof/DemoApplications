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
   FakeMouse(Display *disp);
   ~FakeMouse();
   void ButtonDown(int button); 
   void ButtonUp(int button);
   void Click(int button);
   void DoubleClick(int button);
   void MoveTo(int x, int y);
   void MoveBy(int dx, int dy);
   void GetPos(int &x, int &y);

private:
   bool ButtonEvent(int btn, int e);
   Display *display;
   Window root;
   Screen *screen;
   int width, height;
};
#endif
