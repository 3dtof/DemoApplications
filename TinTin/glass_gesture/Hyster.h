/*
 * TI Hysteresis class implementation
 *
 * Copyright (c) 2015-2016 Texas Instruments Inc.
 */
#ifndef __HYSTER_H__
#define __HYSTER_H__

class Hyster
{
public:
   Hyster(float ub, float lb, int dt);
   Hyster();
   ~Hyster();
   void init(float ub, float lb, int dt);
   bool update(float v);
   void reset(bool v);
   float getUpperB() {return upperBound;}
   float getLowerB() {return lowerBound;}

private:
   bool state;
   bool reported_state;
   float upperBound;
   float lowerBound;
   int deltaT;
   int tick;
};

#endif // __HYSTER_H__
