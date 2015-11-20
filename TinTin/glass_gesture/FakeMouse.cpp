/*
 * TI Smart Glass Demo
 *
 * Copyright (c) 2015-2016 Texas Instruments Inc.
 */
#define __FAKEMOUSE_CPP__
#include "FakeMouse.h"

FakeMouse::FakeMouse(Display *disp) 
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

bool FakeMouse::ButtonEvent(int button, int event_type)
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

void FakeMouse::ButtonDown(int button)
{
   ButtonEvent(button, ButtonPress);
}

void FakeMouse::ButtonUp(int button)
{
   ButtonEvent(button, ButtonRelease);
}

void FakeMouse::Click(int button) 
{
   ButtonDown(button);
   usleep(100000);
   ButtonUp(button);
}


void FakeMouse::DoubleClick(int button)
{
   Click(button);
   usleep(100000);
   Click(button);
}

void FakeMouse::MoveTo(int x, int y)
{
   XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
   XFlush(display);
}

void FakeMouse::MoveBy(int dx, int dy)
{
   int x, y;

   GetPos(x, y);
   XWarpPointer(display, None, root, 0, 0, 0, 0, x+dx, y+dy);
   XFlush(display);
}

void FakeMouse::GetPos(int &x, int &y)
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

#ifdef UNIT_TEST
int main(int argc,char * argv[]) 
{
   int i=0;
   int x , y;
   x=atoi(argv[1]);
   y=atoi(argv[2]);
   Display *display = XOpenDisplay(0);
   FakeMouse mouse(display);
   Screen *screen = DefaultScreenOfDisplay(display);
   mouse.MoveTo(x, y);
   mouse.Click(Button1);

   mouse.MoveTo(0,0);
   for (int i=0; i < 200; i++) {
      mouse.MoveBy(screen->width/200, screen->height/200);
      usleep(10000);
   }

   XCloseDisplay(display);
   return 0;
}
#endif // UNIT_TEST

#undef __FAKEMOUSE_CPP__

