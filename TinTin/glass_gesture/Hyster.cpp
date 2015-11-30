/*
 * TI Hysteresis class implementation
 *
 * Copyright (c) 2015-2016 Texas Instruments Inc.
 */
#define __HYSTER_CPP__
#include <iostream>
#include <math.h>
#include "Hyster.h"

using namespace std;

Hyster::Hyster(float ub, float lb, int dt)
{
   init(ub, lb, dt);
}

Hyster::Hyster()
{
   init(0, 0, 0);
}

Hyster::~Hyster()
{
}

void Hyster::init(float ub, float lb, int dt)
{
   upperBound = ub;
   lowerBound = lb;
   deltaT = dt;
   state = reported_state = false;
   tick = 0;
}

void Hyster::reset(bool v)
{
   state = reported_state = v;
   tick = 0;
}

bool Hyster::update(float v)
{
   if (v <= lowerBound) {
      if (state == true) {
         state = false;
         tick = 0;
      } 
      else {
         if (tick >= deltaT)
            reported_state = state; 
         else
            tick++;
      }
   }
   else if (v >= upperBound) {
      if (state == false) {
         state = true;
         tick = 0;
      } 
      else {
         if (tick >= deltaT)
            reported_state = state; 
         else
            tick++;
      }
   }

   return reported_state;
}


#ifdef __UNIT_TEST__
int main(int argc,char * argv[]) 
{
   float pi = 3.1415926;
   Hyster h1(0.5, -0.5, 10);
   Hyster h2(0.5, -0.5, 10);

   cout << "x, y1, y2, v1, v2" << endl;
   for (float x = 0; x < 2*pi; x += 2*pi/360) {
       float y1 = sin(x);
       float y2 = cos(x);
       cout << x << "," << y1 << "," << y2 << "," << h1.update(y1) << "," << h2.update(y2) 
            << "," << h1.getUpperB() << "," << h1.getLowerB() 
            << "," << h2.getUpperB() << "," << h2.getLowerB() << endl;
   }
   return 0;
}
#undef __UNIT_TEST__
#endif // UNIT_TEST

#undef __HYSTER_CPP__

