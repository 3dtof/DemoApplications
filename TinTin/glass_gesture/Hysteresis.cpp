/*
 * TI Hysteresis class implementation
 *
 * Copyright (c) 2015-2016 Texas Instruments Inc.
 */y
#define __HYSTER_CPP__
#include "hyster.h"

Hyster::Hyster()
{
   setDisplay(XOpenDisplay(0));
}

FakeMouse::FakeMouse(Display *disp) 
{
   setDisplay(disp);
}

void FakeMouse::setDisplay(Display *disp)
{
   display = disp;
   root = DefaultRootWindow(display);
   screen = DefaultScreenOfDisplay(display);
   width = screen->width;
   height = screen->height;
}


FakeMouse::~FakeMouse()
{
}

bool FakeMouse::buttonEvent(int button, int event_type)
{
   XEvent event;

   memset(&event, 0, sizeof(event));
   event.type = event_type;
   event.xbutton.button = button;
   event.xbutton.same_screen = True;
   XQueryPointer(display, RootWindow(display, DefaultScreen(display)), 
                 &event.xbutton.root, &event.xbutton.window, 
                 &event.xbutton.x_root, &event.xbutton.y_root, 
                 &event.xbutton.x, &event.xbutton.y, 
                 &event.xbutton.state);
   event.xbutton.subwindow = event.xbutton.window;
   while (event.xbutton.subwindow) {
      event.xbutton.window = event.xbutton.subwindow;
		
      XQueryPointer(display, event.xbutton.window, 
                 &event.xbutton.root, &event.xbutton.subwindow, 
                 &event.xbutton.x_root, &event.xbutton.y_root, 
                 &event.xbutton.x, &event.xbutton.y, 
                 &event.xbutton.state);
   }
   if (event_type == ButtonRelease)
      event.xbutton.state = 1 << (button+7);

   if (XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0) 
      return false;
   XFlush(display);
   return true;
}

void FakeMouse::buttonDown(int button)
{
   buttonEvent(button, ButtonPress);
}

void FakeMouse::buttonUp(int button)
{
   buttonEvent(button, ButtonRelease);
}

void FakeMouse::click(int button) 
{
   buttonDown(button);
   usleep(100000);
   buttonUp(button);
}


void FakeMouse::doubleClick(int button)
{
   click(button);
   usleep(100000);
   click(button);
}

void FakeMouse::moveTo(int x, int y)
{
   XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
   XFlush(display);
}

void FakeMouse::moveBy(int dx, int dy)
{
   int x, y;

   getPos(x, y);
   XWarpPointer(display, None, root, 0, 0, 0, 0, x+dx, y+dy);
   XFlush(display);
}

void FakeMouse::getPos(int &x, int &y)
{
   XEvent event;
   XQueryPointer(display, RootWindow(display, DefaultScreen(display)), 
                 &event.xbutton.root, &event.xbutton.window, 
                 &event.xbutton.x_root, &event.xbutton.y_root, 
                 &event.xbutton.x, &event.xbutton.y, 
                 &event.xbutton.state);
   x = event.xbutton.x;
   y = event.xbutton.y;
}

void FakeMouse::getDim(int &x, int &y)
{
   x = width;
   y = height;
}


#ifdef __UNIT_TEST__
int main(int argc,char * argv[]) 
{
   int i=0;
   int x , y;
   x=atoi(argv[1]);
   y=atoi(argv[2]);
   Display *display = XOpenDisplay(0);
   FakeMouse mouse(display);
   Screen *screen = DefaultScreenOfDisplay(display);
   mouse.moveTo(x, y);
   mouse.click(Button1);

   mouse.moveTo(0,0);
   for (int i=0; i < 200; i++) {
      mouse.moveBy(screen->width/200, screen->height/200);
      usleep(10000);
   }

   XCloseDisplay(display);
   return 0;
}
#undef __UNIT_TEST__
#endif // UNIT_TEST

#undef __HYSTERESIS_CPP__

