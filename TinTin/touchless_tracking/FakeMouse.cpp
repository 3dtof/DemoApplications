/*! 
 * ==========================================================================================
 *
 * @addtogroup		FakeMouse	
 * @{
 *
 * @file		      FakeMouse.cpp
 * @version		   1.0
 * @date		      1/7/2016
 *
 * @note		      FakeMouse class
 * 
 * Copyright(c) 20015-2016 Texas Instruments Corporation, All Rights Reserved.q
 * TI makes NO WARRANTY as to software products, which are supplied "AS-IS"
 *
 * ==========================================================================================
 */
#define __FAKEMOUSE_CPP__
#include "FakeMouse.h"


/*!
 *===========================================================================================
 * @brief   Constructors
 *===========================================================================================
 */
FakeMouse::FakeMouse()
{
   setDisplay(XOpenDisplay(0));
}

FakeMouse::FakeMouse(Display *disp) 
{
   setDisplay(disp);
}

/*!
 *===========================================================================================
 * @brief   Initialize X display info
 *===========================================================================================
 */
void FakeMouse::setDisplay(Display *disp)
{
   display = disp;
   root = DefaultRootWindow(display);
   screen = DefaultScreenOfDisplay(display);
   width = screen->width;
   height = screen->height;
}


/*!
 *===========================================================================================
 * @brief   Destructor
 *===========================================================================================
 */
FakeMouse::~FakeMouse()
{
}


/*!
 *===========================================================================================
 * @brief   Generate a button event
 *===========================================================================================
 */
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


/*!
 *===========================================================================================
 * @brief   Generate a button down event
 *===========================================================================================
 */
void FakeMouse::buttonDown(int button)
{
   buttonEvent(button, ButtonPress);
}


/*!
 *===========================================================================================
 * @brief   Generate a button up event
 *===========================================================================================
 */
void FakeMouse::buttonUp(int button)
{
   buttonEvent(button, ButtonRelease);
}


/*!
 *===========================================================================================
 * @brief   Generate a click event
 *===========================================================================================
 */
void FakeMouse::click(int button) 
{
   buttonDown(button);
   usleep(100000);
   buttonUp(button);
}


/*!
 *===========================================================================================
 * @brief   Generate a double click event
 *===========================================================================================
 */
void FakeMouse::doubleClick(int button)
{
   click(button);
   usleep(100000);
   click(button);
}


/*!
 *===========================================================================================
 * @brief   Move mouse to absolute position (x, y)
 *===========================================================================================
 */
void FakeMouse::moveTo(int x, int y)
{
   XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
   XFlush(display);
}


/*!
 *===========================================================================================
 * @brief   Move mouse position incrementally by (dx, dy)
 *===========================================================================================
 */
void FakeMouse::moveBy(int dx, int dy)
{
   int x, y;

   getPos(x, y);
   XWarpPointer(display, None, root, 0, 0, 0, 0, x+dx, y+dy);
   XFlush(display);
}


/*!
 *===========================================================================================
 * @brief   Get current mouse absolute position
 *===========================================================================================
 */
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


/*!
 *===========================================================================================
 * @brief   Get display dimension
 *===========================================================================================
 */
void FakeMouse::getDim(int &x, int &y)
{
   x = width;
   y = height;
}


/*!
 *===========================================================================================
 * @brief   Unit Test
 *===========================================================================================
 */
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

#undef __FAKEMOUSE_CPP__

