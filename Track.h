/*
  Track.h - Library for containing a track model.
  Created by Tim Coyne
*/

#ifndef Track_h
#define Track_h

#include "Energia.h"

class Track {
  public:
    Track();
    int update(int state);
    void setMicrosPerCount(int mpc);
    int getMicrosPerCount();
    int getTargetMicrosPerCount();
    int getCount();
  private:
    int _count;
    int _target;
    int _lastCount;
    unsigned long _prevTime;
};

#endif
